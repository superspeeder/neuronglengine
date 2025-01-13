#pragma once

#include <glad/gl.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <string_view>
#include <vector>

#include "neuron/glwrap.hpp"

namespace neuron {

    struct StandardVertex {
        glm::vec4 position;
        glm::vec4 color;
        glm::vec4 normal;
        glm::vec2 texCoord;
    };

    class Mesh final {
      public:
        enum class Mode { Array, ElementArray, ElementArrayMultiDraw };

        enum class PType {
            Points                 = GL_POINTS,
            Lines                  = GL_LINES,
            Triangles              = GL_TRIANGLES,
            LineStrip              = GL_LINE_STRIP,
            TriangleStrip          = GL_TRIANGLE_STRIP,
            TriangleFan            = GL_TRIANGLE_FAN,
            LineLoop               = GL_LINE_LOOP,
            TriangleStripAdjacency = GL_TRIANGLE_STRIP_ADJACENCY,
            LineStripAdjacency     = GL_LINE_STRIP_ADJACENCY,
        };

        struct Data {
            Mode                                               mode;
            PType                                              ptype;
            bool                                               primrestart;
            std::vector<StandardVertex>                        vertices;
            std::vector<unsigned int>                          indices;
            std::vector<std::pair<unsigned int, unsigned int>> draws;

            static Data loadFromNMeshFile(const std::filesystem::path &path);
        };

        explicit Mesh(const Data &data);
        ~Mesh() = default;

        static std::shared_ptr<Mesh> loadFromNMeshFile(const std::filesystem::path &path);

        static std::vector<std::shared_ptr<Mesh>> loadWithAssimp(const std::filesystem::path &path);

        void draw();


      private:
        Mode m_Mode;

        std::shared_ptr<Buffer>      m_VertexBuffer;
        std::shared_ptr<Buffer>      m_ElementBuffer;
        std::shared_ptr<VertexArray> m_VertexArray;

        std::shared_ptr<Buffer> m_DrawBuffer;

        std::size_t m_VertexCount;
        std::size_t m_IndexCount;
        std::size_t m_DrawCount;

        PType m_PType;
        bool  m_SetPrimrestart;
    };

} // namespace neuron
