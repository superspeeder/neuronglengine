#pragma once

#include <cstdlib>
#include <filesystem>
#include <glad/gl.h>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace neuron {
    class Buffer {
      public:
        enum class Target {
            Array            = GL_ARRAY_BUFFER,
            ElementArray     = GL_ELEMENT_ARRAY_BUFFER,
            CopyRead         = GL_COPY_READ_BUFFER,
            CopyWrite        = GL_COPY_WRITE_BUFFER,
            DispatchIndirect = GL_DISPATCH_INDIRECT_BUFFER,
            DrawIndirect     = GL_DRAW_INDIRECT_BUFFER,
            PixelPack        = GL_PIXEL_PACK_BUFFER,
            PixelUnpack      = GL_PIXEL_UNPACK_BUFFER,
            Query            = GL_QUERY_BUFFER,
            Texture          = GL_TEXTURE_BUFFER,

            AtomicCounter     = GL_ATOMIC_COUNTER_BUFFER,
            TransformFeedback = GL_TRANSFORM_FEEDBACK_BUFFER,
            Uniform           = GL_UNIFORM_BUFFER,
            ShaderStorage     = GL_SHADER_STORAGE_BUFFER,
        };

        enum class IndexedTarget {
            AtomicCounter     = GL_ATOMIC_COUNTER_BUFFER,
            TransformFeedback = GL_TRANSFORM_FEEDBACK_BUFFER,
            Uniform           = GL_UNIFORM_BUFFER,
            ShaderStorage     = GL_SHADER_STORAGE_BUFFER,
        };

        enum class Usage {
            StaticDraw  = GL_STATIC_DRAW,
            DynamicDraw = GL_DYNAMIC_DRAW,
            StreamDraw  = GL_STREAM_DRAW,
            StaticRead  = GL_STATIC_READ,
            DynamicRead = GL_DYNAMIC_READ,
            StreamRead  = GL_STREAM_READ,
            StaticCopy  = GL_STATIC_COPY,
            DynamicCopy = GL_DYNAMIC_COPY,
            StreamCopy  = GL_STREAM_COPY,
        };

        Buffer(std::size_t size, const void *data, Usage usage = Usage::StaticDraw);
        ~Buffer();

        template <typename T>
        static std::shared_ptr<Buffer> create(const std::vector<T> &data, Usage usage = Usage::StaticDraw) {
            return std::make_shared<Buffer>(data.size() * sizeof(T), data.data(), usage);
        }

        template <typename T, std::size_t N>
        static std::shared_ptr<Buffer> create(const std::array<T, N> &data, Usage usage = Usage::StaticDraw) {
            static constexpr std::size_t bufsize = N * sizeof(T);
            return std::make_shared<Buffer>(bufsize, data.data(), usage);
        }

        void bind(Target target) const;
        void bind_indexed(IndexedTarget target, unsigned int index) const;
        void bind_range(IndexedTarget target, unsigned int index, intptr_t offset, intptr_t size) const;

        void set(std::size_t size, const void *data);
        void set(std::size_t size, const void *data, Usage usage);

        template <typename T>
        void set(const std::vector<T> &data) {
            set(data.size() * sizeof(T), data.data());
        }

        template <typename T, std::size_t N>
        void set(const std::array<T, N> &data) {
            static constexpr std::size_t bufsize = N * sizeof(T);
            set(bufsize, data.data());
        }

        template <typename T>
        void set(const std::vector<T> &data, const Usage usage) {
            set(data.size() * sizeof(T), data.data(), usage);
        }

        template <typename T, std::size_t N>
        void set(const std::array<T, N> &data, const Usage usage) {
            static constexpr std::size_t bufsize = N * sizeof(T);
            set(bufsize, data.data(), usage);
        }

        inline unsigned int handle() const noexcept { return m_Buffer; };

      private:
        unsigned int m_Buffer;
        Usage        m_CurrentUsage;
        std::size_t  m_CurrentSize;
    };

    struct VertexBinding {
        unsigned int            binding;
        std::ptrdiff_t          stride;
        std::shared_ptr<Buffer> buffer;
        std::ptrdiff_t          offset = 0;
    };

    struct VertexAttribute {
        unsigned int   location;
        unsigned int   binding;
        std::ptrdiff_t offset;
        unsigned int   size;
    };

    struct VertexLayout {
        std::vector<VertexBinding>   bindings;
        std::vector<VertexAttribute> attributes;
    };

    class VertexArray {
      public:
        explicit VertexArray(const VertexLayout &vertexLayout, const std::shared_ptr<Buffer> &elementBuffer = nullptr);
        ~VertexArray();

        void bind() const;

        [[nodiscard]] inline unsigned int handle() const noexcept { return m_VertexArray; };


      private:
        unsigned int m_VertexArray;
        bool         m_HasElementBuffer;
    };

    class ShaderModule {
      public:
        enum class Type {
            Vertex                 = GL_VERTEX_SHADER,
            Fragment               = GL_FRAGMENT_SHADER,
            Geometry               = GL_GEOMETRY_SHADER,
            TessellationControl    = GL_TESS_CONTROL_SHADER,
            TessellationEvaluation = GL_TESS_EVALUATION_SHADER,
            Compute                = GL_COMPUTE_SHADER,
        };


        ShaderModule(std::string_view code, Type type);
        ~ShaderModule();

        [[nodiscard]] inline unsigned int handle() const noexcept { return m_Shader; };

        [[nodiscard]] static std::shared_ptr<ShaderModule> load(const std::filesystem::path &path, Type type);

      private:
        unsigned int m_Shader;
    };

    template <typename T, typename R>
    concept deref_into = std::same_as<std::remove_cv_t<decltype(*std::declval<T>())>, R &>;

    struct ProgramParameters {
        bool separable = false;
    };

    class Shader {
      public:
        template <deref_into<ShaderModule> Ts>
        inline explicit Shader(const std::vector<Ts> shaders) {
            if (shaders.empty()) {
                throw std::invalid_argument("Must pass at least one shader to Shader::Shader()");
            }

            m_Program = glCreateProgram();

            for (const auto &shader : shaders) {
                glAttachShader(m_Program, (*shader).handle());
            }

            glLinkProgram(m_Program);

            int status;
            glGetProgramiv(m_Program, GL_LINK_STATUS, &status);
            if (status != GL_TRUE) {
                glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &status);

                std::string info_log;
                info_log.resize(status);
                glGetProgramInfoLog(m_Program, status, &status, info_log.data());
                throw std::runtime_error("Failed to link shader program: " + info_log);
            }
        }

        template <deref_into<ShaderModule> Ts>
        inline Shader(const ProgramParameters &parameters, const std::vector<Ts> shaders) {
            if (shaders.empty()) {
                throw std::invalid_argument("Must pass at least one shader to Shader::Shader()");
            }

            m_Program = glCreateProgram();

            for (const auto &shader : shaders) {
                glAttachShader(m_Program, (*shader).handle());
            }

            glProgramParameteri(m_Program, GL_PROGRAM_SEPARABLE, parameters.separable);

            glLinkProgram(m_Program);

            int status;
            glGetProgramiv(m_Program, GL_LINK_STATUS, &status);
            if (status != GL_TRUE) {
                glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &status);

                std::string info_log;
                info_log.resize(status);
                glGetProgramInfoLog(m_Program, status, &status, info_log.data());
                throw std::runtime_error("Failed to link shader program: " + info_log);
            }
        }

        int getUniformLocation(const std::string_view &name) const;

        void uniform1f(int location, float x) const;
        void uniform2f(int location, float x, float y) const;
        void uniform3f(int location, float x, float y, float z) const;
        void uniform4f(int location, float x, float y, float z, float w) const;
        void uniform2f(int location, const ::glm::vec2 &v) const;
        void uniform3f(int location, const ::glm::vec3 &v) const;
        void uniform4f(int location, const ::glm::vec4 &v) const;

        inline void uniform1f(const std::string_view location, const float x) const { uniform1f(getUniformLocation(location), x); };

        inline void uniform2f(const std::string_view location, const float x, const float y) const { uniform2f(getUniformLocation(location), x, y); };

        inline void uniform3f(const std::string_view location, const float x, const float y, const float z) const { uniform3f(getUniformLocation(location), x, y, z); };

        inline void uniform4f(const std::string_view location, const float x, const float y, const float z, const float w) const {
            uniform4f(getUniformLocation(location), x, y, z, w);
        };

        inline void uniform2f(const std::string_view location, const ::glm::vec2 &v) const { uniform2f(getUniformLocation(location), v); };

        inline void uniform3f(const std::string_view location, const ::glm::vec3 &v) const { uniform3f(getUniformLocation(location), v); };

        inline void uniform4f(const std::string_view location, const ::glm::vec4 &v) const { uniform4f(getUniformLocation(location), v); };

        void uniformMatrix2f(int location, const ::glm::mat2 &v) const;
        void uniformMatrix3f(int location, const ::glm::mat3 &v) const;
        void uniformMatrix4f(int location, const ::glm::mat4 &v) const;

        inline void uniformMatrix2f(const std::string_view location, const ::glm::mat2 &v) const { uniformMatrix2f(getUniformLocation(location), v); };

        inline void uniformMatrix3f(const std::string_view location, const ::glm::mat3 &v) const { uniformMatrix3f(getUniformLocation(location), v); };

        inline void uniformMatrix4f(const std::string_view location, const ::glm::mat4 &v) const { uniformMatrix4f(getUniformLocation(location), v); };

        void uniformMatrix2x3f(int location, const ::glm::mat2x3 &v) const;
        void uniformMatrix2x4f(int location, const ::glm::mat2x4 &v) const;
        void uniformMatrix3x2f(int location, const ::glm::mat3x2 &v) const;
        void uniformMatrix3x4f(int location, const ::glm::mat3x4 &v) const;
        void uniformMatrix4x2f(int location, const ::glm::mat4x2 &v) const;
        void uniformMatrix4x3f(int location, const ::glm::mat4x3 &v) const;

        inline void uniformMatrix2x3f(const std::string_view location, const ::glm::mat2x3 &v) const { uniformMatrix2x3f(getUniformLocation(location), v); };

        inline void uniformMatrix2x4f(const std::string_view location, const ::glm::mat2x4 &v) const { uniformMatrix2x4f(getUniformLocation(location), v); };

        inline void uniformMatrix3x2f(const std::string_view location, const ::glm::mat3x2 &v) const { uniformMatrix3x2f(getUniformLocation(location), v); };

        inline void uniformMatrix3x4f(const std::string_view location, const ::glm::mat3x4 &v) const { uniformMatrix3x4f(getUniformLocation(location), v); };

        inline void uniformMatrix4x2f(const std::string_view location, const ::glm::mat4x2 &v) const { uniformMatrix4x2f(getUniformLocation(location), v); };

        inline void uniformMatrix4x3f(const std::string_view location, const ::glm::mat4x3 &v) const { uniformMatrix4x3f(getUniformLocation(location), v); };

        void uniform1d(int location, double x) const;
        void uniform2d(int location, double x, double y) const;
        void uniform3d(int location, double x, double y, double z) const;
        void uniform4d(int location, double x, double y, double z, double w) const;
        void uniform2d(int location, const ::glm::dvec2 &v) const;
        void uniform3d(int location, const ::glm::dvec3 &v) const;
        void uniform4d(int location, const ::glm::dvec4 &v) const;

        inline void uniform1d(const std::string_view location, const double x) const { uniform1d(getUniformLocation(location), x); };

        inline void uniform2d(const std::string_view location, const double x, const double y) const { uniform2d(getUniformLocation(location), x, y); };

        inline void uniform3d(const std::string_view location, const double x, const double y, const double z) const { uniform3d(getUniformLocation(location), x, y, z); };

        inline void uniform4d(const std::string_view location, const double x, const double y, const double z, const double w) const {
            uniform4d(getUniformLocation(location), x, y, z, w);
        };

        inline void uniform2d(const std::string_view location, const ::glm::dvec2 &v) const { uniform2d(getUniformLocation(location), v); };

        inline void uniform3d(const std::string_view location, const ::glm::dvec3 &v) const { uniform3d(getUniformLocation(location), v); };

        inline void uniform4d(const std::string_view location, const ::glm::dvec4 &v) const { uniform4d(getUniformLocation(location), v); };

        void uniformMatrix2d(int location, const ::glm::dmat2 &v) const;
        void uniformMatrix3d(int location, const ::glm::dmat3 &v) const;
        void uniformMatrix4d(int location, const ::glm::dmat4 &v) const;

        inline void uniformMatrix2d(const std::string_view location, const ::glm::dmat2 &v) const { uniformMatrix2d(getUniformLocation(location), v); };

        inline void uniformMatrix3d(const std::string_view location, const ::glm::dmat3 &v) const { uniformMatrix3d(getUniformLocation(location), v); };

        inline void uniformMatrix4d(const std::string_view location, const ::glm::dmat4 &v) const { uniformMatrix4d(getUniformLocation(location), v); };

        void uniformMatrix2x3d(int location, const ::glm::dmat2x3 &v) const;
        void uniformMatrix2x4d(int location, const ::glm::dmat2x4 &v) const;
        void uniformMatrix3x2d(int location, const ::glm::dmat3x2 &v) const;
        void uniformMatrix3x4d(int location, const ::glm::dmat3x4 &v) const;
        void uniformMatrix4x2d(int location, const ::glm::dmat4x2 &v) const;
        void uniformMatrix4x3d(int location, const ::glm::dmat4x3 &v) const;

        inline void uniformMatrix2x3d(const std::string_view location, const ::glm::dmat2x3 &v) const { uniformMatrix2x3d(getUniformLocation(location), v); };

        inline void uniformMatrix2x4d(const std::string_view location, const ::glm::dmat2x4 &v) const { uniformMatrix2x4d(getUniformLocation(location), v); };

        inline void uniformMatrix3x2d(const std::string_view location, const ::glm::dmat3x2 &v) const { uniformMatrix3x2d(getUniformLocation(location), v); };

        inline void uniformMatrix3x4d(const std::string_view location, const ::glm::dmat3x4 &v) const { uniformMatrix3x4d(getUniformLocation(location), v); };

        inline void uniformMatrix4x2d(const std::string_view location, const ::glm::dmat4x2 &v) const { uniformMatrix4x2d(getUniformLocation(location), v); };

        inline void uniformMatrix4x3d(const std::string_view location, const ::glm::dmat4x3 &v) const { uniformMatrix4x3d(getUniformLocation(location), v); };

        void uniform1ui(int location, const unsigned int x) const;
        void uniform2ui(int location, const unsigned int x, const unsigned int y) const;
        void uniform3ui(int location, const unsigned int x, const unsigned int y, const unsigned int z) const;
        void uniform4ui(int location, const unsigned int x, const unsigned int y, const unsigned int z, const unsigned int w) const;
        void uniform2ui(int location, const ::glm::uvec2 &v) const;
        void uniform3ui(int location, const ::glm::uvec3 &v) const;
        void uniform4ui(int location, const ::glm::uvec4 &v) const;

        inline void uniform1ui(const std::string_view location, const unsigned int x) const { uniform1ui(getUniformLocation(location), x); };

        inline void uniform2ui(const std::string_view location, const unsigned int x, const unsigned int y) const { uniform2ui(getUniformLocation(location), x, y); };

        inline void uniform3ui(const std::string_view location, const unsigned int x, const unsigned int y, const unsigned int z) const {
            uniform3ui(getUniformLocation(location), x, y, z);
        };

        inline void uniform4ui(const std::string_view location, const unsigned int x, const unsigned int y, const unsigned int z, const unsigned int w) const {
            uniform4ui(getUniformLocation(location), x, y, z, w);
        };

        inline void uniform2ui(const std::string_view location, const ::glm::uvec2 &v) const { uniform2ui(getUniformLocation(location), v); };

        inline void uniform3ui(const std::string_view location, const ::glm::uvec3 &v) const { uniform3ui(getUniformLocation(location), v); };

        inline void uniform4ui(const std::string_view location, const ::glm::uvec4 &v) const { uniform4ui(getUniformLocation(location), v); };

        void uniform1i(int location, int x) const;
        void uniform2i(int location, int x, int y) const;
        void uniform3i(int location, int x, int y, int z) const;
        void uniform4i(int location, int x, int y, int z, int w) const;
        void uniform2i(int location, const ::glm::ivec2 &v) const;
        void uniform3i(int location, const ::glm::ivec3 &v) const;
        void uniform4i(int location, const ::glm::ivec4 &v) const;

        inline void uniform1i(const std::string_view location, const int x) const { uniform1i(getUniformLocation(location), x); };

        inline void uniform2i(const std::string_view location, const int x, const int y) const { uniform2i(getUniformLocation(location), x, y); };

        inline void uniform3i(const std::string_view location, const int x, const int y, const int z) const { uniform3i(getUniformLocation(location), x, y, z); };

        inline void uniform4i(const std::string_view location, const int x, const int y, const int z, const int w) const { uniform4i(getUniformLocation(location), x, y, z, w); };

        inline void uniform2i(const std::string_view location, const ::glm::ivec2 &v) const { uniform2i(getUniformLocation(location), v); };

        inline void uniform3i(const std::string_view location, const ::glm::ivec3 &v) const { uniform3i(getUniformLocation(location), v); };

        inline void uniform4i(const std::string_view location, const ::glm::ivec4 &v) const { uniform4i(getUniformLocation(location), v); };

        void use() const;

      private:
        unsigned int m_Program = ~0U;
    };

    class Texture {
      public:
        enum class Type {
            Texture1D,
            Texture2D,
            Texture3D,
            Texture1DArray,
            Texture2DArray,
            TextureRectangle,
            TextureCubeMap,
            TextureCubeMapArray,
            TextureBuffer,
            Texture2DMS,
            Texture2DMSArray,
        };

        enum class Format {
            RGBA         = GL_RGBA,
            BGRA         = GL_BGRA,
            RGB          = GL_RGB,
            BGR          = GL_BGR,
            RG           = GL_RG,
            R            = GL_RED,
            DepthStencil = GL_DEPTH_STENCIL,
            Depth        = GL_DEPTH_COMPONENT,
        };

        enum class InternalFormat {
            RGBA           = GL_RGBA,
            RGB            = GL_RGB,
            RG             = GL_RG,
            R              = GL_RED,
            DepthStencil   = GL_DEPTH_STENCIL,
            Depth          = GL_DEPTH_COMPONENT,
            R8             = GL_R8,
            R8_SNORM       = GL_R8_SNORM,
            R16            = GL_R16,
            R16_SNORM      = GL_R16_SNORM,
            RG8            = GL_RG8,
            RG8_SNORM      = GL_RG8_SNORM,
            RG16           = GL_RG16,
            RG16_SNORM     = GL_RG16_SNORM,
            R3_G3_B2       = GL_R3_G3_B2,
            RGB4           = GL_RGB4,
            RGB5           = GL_RGB5,
            RGB8           = GL_RGB8,
            RGB8_SNORM     = GL_RGB8_SNORM,
            RGB10          = GL_RGB10,
            RGB12          = GL_RGB12,
            RGB16_SNORM    = GL_RGB16_SNORM,
            RGBA2          = GL_RGBA2,
            RGBA4          = GL_RGBA4,
            RGB5_A1        = GL_RGB5_A1,
            RGBA8          = GL_RGBA8,
            RGB10_A2       = GL_RGB10_A2,
            RGB10_A2UI     = GL_RGB10_A2UI,
            RGBA12         = GL_RGBA12,
            RGBA16         = GL_RGBA16,
            SRGB8          = GL_SRGB8,
            SRGB8_ALPHA8   = GL_SRGB8_ALPHA8,
            R16F           = GL_R16F,
            RG16F          = GL_RG16F,
            RGB16F         = GL_RGB16F,
            RGBA16F        = GL_RGBA16F,
            R32F           = GL_R32F,
            RG32F          = GL_RG32F,
            RGB32F         = GL_RGB32F,
            RGBA32F        = GL_RGBA32F,
            R11F_G11F_B10F = GL_R11F_G11F_B10F,
            RGB9_E5        = GL_RGB9_E5,
            R8I            = GL_R8I,
            R8UI           = GL_R8UI,
            R16I           = GL_R16I,
            R16UI          = GL_R16UI,
            R32I           = GL_R32I,
            R32UI          = GL_R32UI,
            RG8I           = GL_RG8I,
            RG8UI          = GL_RG8UI,
            RG16I          = GL_RG16I,
            RG16UI         = GL_RG16UI,
            RG32I          = GL_RG32I,
            RG32UI         = GL_RG32UI,
            RGB8I          = GL_RGB8I,
            RGB8UI         = GL_RGB8UI,
            RGB16I         = GL_RGB16I,
            RGB16UI        = GL_RGB16UI,
            RGB32I         = GL_RGB32I,
            RGB32UI        = GL_RGB32UI,
            RGBA8I         = GL_RGBA8I,
            RGBA8UI        = GL_RGBA8UI,
            RGBA16I        = GL_RGBA16I,
            RGBA16UI       = GL_RGBA16UI,
            RGBA32I        = GL_RGBA32I,
            RGBA32UI       = GL_RGBA32UI,

            COMPRESSED_RED                     = GL_COMPRESSED_RED,
            COMPRESSED_RG                      = GL_COMPRESSED_RG,
            COMPRESSED_RGB                     = GL_COMPRESSED_RGB,
            COMPRESSED_RGBA                    = GL_COMPRESSED_RGBA,
            COMPRESSED_SRGB                    = GL_COMPRESSED_SRGB,
            COMPRESSED_SRGB_ALPHA              = GL_COMPRESSED_SRGB_ALPHA,
            COMPRESSED_SIGNED_RED_RGTC1        = GL_COMPRESSED_SIGNED_RED_RGTC1,
            COMPRESSED_RG_RGTC2                = GL_COMPRESSED_SIGNED_RG_RGTC2,
            COMPRESSED_SIGNED_RG_RGTC2         = GL_COMPRESSED_SIGNED_RG_RGTC2,
            COMPRESSED_RGBA_BPTC_UNORM         = GL_COMPRESSED_RGBA_BPTC_UNORM,
            COMPRESSED_SRGB_ALPHA_BPTC_UNORM   = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM,
            COMPRESSED_RGB_BPTC_SIGNED_FLOAT   = GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT,
            COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,
        };

        enum class DataType {
            Float = GL_FLOAT,
            Int = GL_INT,
            UnsignedInt = GL_UNSIGNED_INT,
            Float16 = GL_HALF_FLOAT,
            Short = GL_SHORT,
            UnsignedShort = GL_UNSIGNED_SHORT,
            Byte = GL_BYTE,
            UnsignedByte = GL_UNSIGNED_BYTE,
        };

        explicit Texture(Type type);
        ~Texture();

        static std::shared_ptr<Texture> create2d(int width, int height, Format format = Format::RGBA, InternalFormat internal_format = InternalFormat::RGBA8, DataType dataType = DataType::UnsignedByte, const void *data = nullptr);

        void image2d(int width, int height, Format format = Format::RGBA, InternalFormat internalFormat = InternalFormat::RGBA8, DataType dataType = DataType::UnsignedByte, const void *data = nullptr) const;
        void image2d(int width, int height, int level, Format format = Format::RGBA, InternalFormat internal_format = InternalFormat::RGBA8, DataType dataType = DataType::UnsignedByte, const void *data = nullptr) const;

        void bind() const;

        [[nodiscard]] inline unsigned int handle() const { return m_Texture; };

      private:
        unsigned int m_Texture;
        Type         m_Type;
    };

    class Renderbuffer {
    public:

        Renderbuffer(Texture::InternalFormat internalFormat, int width, int height, int samples = 0);
        ~Renderbuffer();

        void bind() const;
        static void unbind();

        [[nodiscart]] inline unsigned int handle() const { return m_Renderbuffer; };

    private:
        unsigned int m_Renderbuffer;
    };

    class Framebuffer {
    public:
        enum class FramebufferTarget {
            Framebuffer = GL_FRAMEBUFFER,
            DrawFramebuffer = GL_DRAW_FRAMEBUFFER,
            ReadFramebuffer = GL_READ_FRAMEBUFFER,
        };

        Framebuffer();
        ~Framebuffer();

        [[nodiscard]] bool is_complete() const;

        void bind(FramebufferTarget target = FramebufferTarget::Framebuffer) const;
        static void unbind(FramebufferTarget target = FramebufferTarget::Framebuffer);

        void attach_color_texture(const std::shared_ptr<Texture>& texture, uint8_t index, int level = 0) const;
        void attach_depth_texture(const std::shared_ptr<Texture>& texture, int level = 0) const;
        void attach_stencil_texture(const std::shared_ptr<Texture>& texture, int level = 0) const;
        void attach_depth_stencil_texture(const std::shared_ptr<Texture>& texture, int level = 0) const;

        [[nodiscart]] inline unsigned int handle() const { return m_Framebuffer; };
    private:
        unsigned int m_Framebuffer;
    };

    struct DrawElementsIndirectCommand {
        GLuint count;
        GLuint instanceCount;
        GLuint firstIndex;
        GLint baseVertex;
        GLuint baseInstance;
    };
} // namespace neuron
