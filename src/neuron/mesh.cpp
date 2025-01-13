#include "neuron/mesh.hpp"

#include <fstream>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <span>

namespace neuron {

    enum class NMeshSVEl { Vertex = 'v', Color = 'c', Normal = 'n', TexCoord = 't' };

    StandardVertex readNMeshVertex(const std::string &line) {
        if (!line.ends_with(';'))
            throw std::invalid_argument("Malformed nmesh vertex line: must end with a semicolon");
        std::string rem = line;

        StandardVertex vert{};
        do {
            const std::size_t pos  = rem.find_first_of(';');
            std::string       sect = rem.substr(0, pos);
            rem                    = rem.substr(pos + 1);

            std::size_t off = 0;
            while (off < sect.length() && sect[off] == ' ')
                off++; // skip spaces

            if (off < sect.length()) {
                char cid = sect[off];
                sect     = sect.substr(off + 2);
                switch (const auto id = static_cast<NMeshSVEl>(cid)) {
                case NMeshSVEl::Vertex:
                    sscanf_s(sect.c_str(), "%f %f %f", &vert.position.x, &vert.position.y, &vert.position.z);
                    vert.position.w = 1.0;
                    break;
                case NMeshSVEl::Color:
                    sscanf_s(sect.c_str(), "%f %f %f %f", &vert.color.r, &vert.color.g, &vert.color.b, &vert.color.a);
                    break;
                case NMeshSVEl::Normal:
                    sscanf_s(sect.c_str(), "%f %f %f", &vert.normal.x, &vert.normal.y, &vert.normal.z);
                    vert.normal.w = 0.0;
                    break;
                case NMeshSVEl::TexCoord:
                    sscanf_s(sect.c_str(), "%f %f", &vert.texCoord.x, &vert.texCoord.y);
                    break;
                default:
                    throw std::invalid_argument("Malformed nmesh vertex line: '" + std::to_string(cid) + "' is not a valid vertex component id");
                }
            }
        } while (!rem.empty());

        return vert;
    }

    Mesh::Data Mesh::Data::loadFromNMeshFile(const std::filesystem::path &path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file " + path.string());
        }

        Data        data;
        std::string line;
        while (line.empty()) {
            std::getline(file, line);
        }

        if (!line.starts_with("MODE ")) {
            throw std::runtime_error("Malformed nmesh file: First non-blank line must be in the format 'MODE mode ptype'");
        }
        std::string mode  = line.substr(5);
        std::string ptype = mode.substr(mode.find_first_of(' ') + 1);
        mode              = mode.substr(0, mode.find_first_of(' '));

        if (mode == "array") {
            data.mode = Mode::Array;
        } else if (mode == "elements") {
            data.mode = Mode::ElementArray;
        } else if (mode == "elements_md") {
            data.mode = Mode::ElementArrayMultiDraw;
        } else {
            throw std::runtime_error("Malformed nmesh file: Unknown mode '" + mode + "'");
        }

        int iperprim = 0; // 0 is used for strips and fans

        data.primrestart = false;
        if (ptype == "triangles") {
            data.ptype = PType::Triangles;
            iperprim   = 3;
        } else if (ptype == "points") {
            data.ptype = PType::Points;
            iperprim   = 1;
        } else if (ptype == "lines") {
            data.ptype = PType::Lines;
            iperprim   = 2;
        } else if (ptype == "triangle_strip") {
            data.ptype       = PType::TriangleStrip;
            data.primrestart = true;
        } else if (ptype == "triangle_strip_adj") {
            data.ptype       = PType::TriangleStripAdjacency;
            data.primrestart = true;
        } else if (ptype == "triangle_fan") {
            data.ptype       = PType::TriangleFan;
            data.primrestart = true;
        } else if (ptype == "line_strip") {
            data.ptype       = PType::LineStrip;
            data.primrestart = true;
        } else if (ptype == "line_strip_adj") {
            data.ptype       = PType::LineStripAdjacency;
            data.primrestart = true;
        } else if (ptype == "line_loop") {
            data.ptype       = PType::LineLoop;
            data.primrestart = true;
        }

        unsigned int draw_start   = 0;
        unsigned int draw_indices = 0;

        while (!file.eof()) {
            std::getline(file, line);
            if (line.empty())
                continue;

            std::size_t off = 0;

            while (off < line.length() && line[off] == ' ')
                off++;
            if (off < line.length()) {
                line = line.substr(off);
                if (line[0] == '#')
                    continue;

                if (line[0] == 'i') {
                    // index mode
                    std::size_t offset = 2;
                    int         remi   = iperprim;
                    while (((iperprim > 0 && remi-- > 0) || iperprim == 0) && offset < line.length()) {
                        try {
                            std::size_t extraoff = 0;
                            long        indexl   = std::stol(line.substr(offset), &extraoff, 10);
                            offset += extraoff;

                            auto index = static_cast<unsigned int>(indexl);
                            if (indexl > std::numeric_limits<unsigned int>::max()) {
                                throw std::runtime_error("Malformed nmesh file: Index out of range");
                            }

                            if (data.primrestart && indexl == -1) {
                                index = ~0U;
                            }

                            draw_indices++;
                            data.indices.push_back(index);
                        } catch (std::invalid_argument &e) {
                            if (iperprim > 0)
                                throw std::runtime_error("Malformed nmesh file: Not enough indices in primitive");
                            break; // otherwise maybe we hit the end of the line
                        }
                    }

                    if (data.primrestart && !data.indices.empty()) {
                        data.indices.push_back(~0U); // primitive restart is ~0U
                    }


                    if (draw_indices > 0) { // draws only get reset on new lines (manual primitive restart doesn't form a new draw)
                        data.draws.push_back(std::make_pair(draw_start, draw_indices));
                    }
                    draw_start   = data.indices.size();
                    draw_indices = 0;

                } else {
                    data.vertices.push_back(readNMeshVertex(line));
                }
            }
        }

        return data;
    }

    Mesh::Mesh(const Data &data) {
        m_Mode = data.mode;

        m_VertexBuffer = Buffer::create(data.vertices);
        m_VertexCount  = data.vertices.size();

        if (m_Mode == Mode::ElementArray || m_Mode == Mode::ElementArrayMultiDraw) {
            m_ElementBuffer  = Buffer::create(data.indices);
            m_IndexCount     = data.indices.size();
            m_SetPrimrestart = data.primrestart;
        }

        if (m_Mode == Mode::ElementArrayMultiDraw) {
            std::vector<DrawElementsIndirectCommand> draws;
            draws.reserve(data.draws.size());
            for (const auto &[start, count] : data.draws) {
                draws.push_back({count, 1, start, 0, 0});
            }

            m_DrawBuffer = Buffer::create(draws);
            m_DrawCount  = draws.size();
        }

        m_PType = data.ptype;

        m_VertexArray = std::make_shared<VertexArray>(
            VertexLayout{
                .bindings = {{
                    .binding = 0,
                    .stride  = sizeof(StandardVertex),
                    .buffer  = m_VertexBuffer,
                    .offset  = 0,
                }},
                .attributes =
                    {
                        {
                            .location = 0,
                            .binding  = 0,
                            .offset   = offsetof(StandardVertex, position),
                            .size     = 4,
                        },
                        {
                            .location = 1,
                            .binding  = 0,
                            .offset   = offsetof(StandardVertex, color),
                            .size     = 4,
                        },
                        {
                            .location = 2,
                            .binding  = 0,
                            .offset   = offsetof(StandardVertex, normal),
                            .size     = 4,
                        },
                        {
                            .location = 3,
                            .binding  = 0,
                            .offset   = offsetof(StandardVertex, texCoord),
                            .size     = 2,
                        },
                    },
            },
            m_ElementBuffer);
    }

    std::shared_ptr<Mesh> Mesh::loadFromNMeshFile(const std::filesystem::path &path) {
        return std::make_shared<Mesh>(Data::loadFromNMeshFile(path));
    }

    std::vector<std::shared_ptr<Mesh>> Mesh::loadWithAssimp(const std::filesystem::path &path) {
        Assimp::Importer importer;
        const aiScene   *scene = importer.ReadFile(path.string(), aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

        if (scene == nullptr) {
            throw std::runtime_error("Failed to load model");
        }

        std::vector<std::shared_ptr<Mesh>> meshes;

        for (std::size_t i = 0; i < scene->mNumMeshes; i++) {
            aiMesh    *mesh = scene->mMeshes[i];
            Mesh::Data meshData{};

            for (std::size_t j = 0; j < mesh->mNumVertices; j++) {
                StandardVertex vert{};

                aiVector3D position = mesh->mVertices[j];
                vert.position       = {position.x, position.y, position.z, 1.0f};

                if (mesh->HasNormals()) {
                    aiVector3D normal = mesh->mNormals[j];
                    vert.normal       = {normal.x, normal.y, normal.z, 0.0f};
                }

                if (mesh->HasVertexColors(0)) {
                    aiColor4D color = mesh->mColors[0][j];
                    vert.color      = {color.r, color.g, color.b, color.a};
                } else {
                    vert.color = {1.0f, 1.0f, 1.0f, 1.0f};
                }

                if (mesh->HasTextureCoords(0)) {
                    aiVector3D texCoord = mesh->mTextureCoords[0][j];
                    vert.texCoord       = {texCoord.x, texCoord.y};
                }

                meshData.vertices.push_back(vert);
            }

            switch (mesh->mPrimitiveTypes) {
            case aiPrimitiveType_POINT:
                meshData.ptype = PType::Points;
                break;
            case aiPrimitiveType_LINE:
                meshData.ptype = PType::Lines;
                break;
            case aiPrimitiveType_TRIANGLE:
                meshData.ptype = PType::Triangles;
                break;
            case aiPrimitiveType_POLYGON:
                throw std::runtime_error("Polygons not supported yet");
            default:
                throw std::runtime_error("Unsupported primitive type");
            }

            for (std::size_t j = 0; j < mesh->mNumFaces; j++) {
                aiFace face = mesh->mFaces[j];

                std::span span(face.mIndices, face.mNumIndices);
                meshData.indices.append_range(span);
            }

            meshData.mode = Mode::ElementArray;
            meshData.primrestart = false;

            meshes.push_back(std::make_shared<Mesh>(meshData));
        }

        return meshes;
    }

    void Mesh::draw() {
        m_VertexArray->bind();

        if (m_Mode == Mode::ElementArray) {
            if (m_SetPrimrestart) {
                glEnable(GL_PRIMITIVE_RESTART);
                glPrimitiveRestartIndex(~0U);
            }

            glDrawElements(static_cast<GLenum>(m_PType), m_IndexCount, GL_UNSIGNED_INT, nullptr);
        } else if (m_Mode == Mode::ElementArrayMultiDraw) {
            if (m_SetPrimrestart) {
                glEnable(GL_PRIMITIVE_RESTART);
                glPrimitiveRestartIndex(~0U);
            }

            m_DrawBuffer->bind(Buffer::Target::DrawIndirect);

            glMultiDrawElementsIndirect(static_cast<GLenum>(m_PType), GL_UNSIGNED_INT, nullptr, m_DrawCount, 0);

        } else {
            glDrawArrays(static_cast<GLenum>(m_PType), 0, m_VertexCount);
        }
    }


} // namespace neuron
