#include "glwrap.hpp"

#include <fstream>

namespace neuron {
    Buffer::Buffer(const std::size_t size, const void *data, const Usage usage) : m_Buffer(~0U), m_CurrentUsage(usage), m_CurrentSize(size) {
        glCreateBuffers(1, &m_Buffer);
        glNamedBufferData(m_Buffer, static_cast<intptr_t>(size), data, static_cast<GLenum>(usage));
    }

    Buffer::~Buffer() {
        glDeleteBuffers(1, &m_Buffer);
    }

    void Buffer::bind(const Target target) const {
        glBindBuffer(static_cast<GLenum>(target), m_Buffer);
    }

    void Buffer::bind_indexed(const IndexedTarget target, const unsigned int index) const {
        glBindBufferBase(static_cast<GLenum>(target), index, m_Buffer);
    }

    void Buffer::bind_range(const IndexedTarget target, const unsigned int index, const intptr_t offset, const intptr_t size) const {
        glBindBufferRange(static_cast<GLenum>(target), index, m_Buffer, offset, size);
    }

    void Buffer::set(const std::size_t size, const void *data) {
        if (size != m_CurrentSize) {
            glNamedBufferData(m_Buffer, static_cast<intptr_t>(size), data, static_cast<GLenum>(m_CurrentUsage));
            m_CurrentSize = size;
        } else {
            glNamedBufferSubData(m_Buffer, 0, static_cast<intptr_t>(size), data);
        }
    }

    void Buffer::set(const std::size_t size, const void *data, const Usage usage) {
        if (size != m_CurrentSize || usage != m_CurrentUsage) {
            glNamedBufferData(m_Buffer, static_cast<intptr_t>(size), data, static_cast<GLenum>(usage));
            m_CurrentSize  = size;
            m_CurrentUsage = usage;
        } else {
            glNamedBufferSubData(m_Buffer, 0, static_cast<intptr_t>(size), data);
        }
    }

    VertexArray::VertexArray(const VertexLayout &vertexLayout, const std::shared_ptr<Buffer> &elementBuffer) : m_VertexArray(~0U), m_HasElementBuffer(elementBuffer != nullptr) {
        glCreateVertexArrays(1, &m_VertexArray);

        if (m_HasElementBuffer) {
            glVertexArrayElementBuffer(m_VertexArray, elementBuffer->handle());
        }

        for (const auto &[binding, stride, buffer, offset] : vertexLayout.bindings) {
            glVertexArrayVertexBuffer(m_VertexArray, binding, buffer->handle(), offset, static_cast<GLsizei>(stride));
        }

        for (const auto &[location, binding, offset, size] : vertexLayout.attributes) {
            glVertexArrayAttribBinding(m_VertexArray, location, binding);
            glVertexArrayAttribFormat(m_VertexArray, location, static_cast<GLint>(size), GL_FLOAT, GL_FALSE, static_cast<GLuint>(offset));
            glEnableVertexArrayAttrib(m_VertexArray, location);
        }
    }

    VertexArray::~VertexArray() {
        glDeleteVertexArrays(1, &m_VertexArray);
    }

    void VertexArray::bind() const {
        glBindVertexArray(m_VertexArray);
    }

    ShaderModule::ShaderModule(const std::string_view code, Type type) : m_Shader(glCreateShader(static_cast<GLenum>(type))) {
        const auto pcode = code.data();
        glShaderSource(m_Shader, 1, &pcode, nullptr);
        glCompileShader(m_Shader);

        int status;
        glGetShaderiv(m_Shader, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE) {
            int length;
            glGetShaderiv(m_Shader, GL_INFO_LOG_LENGTH, &length);
            std::string ilog;
            ilog.resize(length);
            glGetShaderInfoLog(m_Shader, length, &length, ilog.data());
            throw std::runtime_error("Failed to compile shader: " + std::string(ilog));
        }
    }

    ShaderModule::~ShaderModule() {
        glDeleteShader(m_Shader);
    }

    std::shared_ptr<ShaderModule> ShaderModule::load(const std::filesystem::path &path, Type type) {
        std::ifstream file(path, std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open " + path.string());
        }

        const std::streampos end = file.tellg();
        std::string          source;
        source.resize(end);
        file.seekg(0, std::ios::beg).read(source.data(), end);
        file.close();

        return std::make_shared<ShaderModule>(source, type);
    }

    int Shader::getUniformLocation(const std::string_view &name) const {
        return glGetUniformLocation(m_Program, name.data());
    }

    void Shader::uniform1f(const int location, const float x) const {
        glProgramUniform1f(m_Program, location, x);
    }

    void Shader::uniform2f(const int location, const float x, const float y) const {
        glProgramUniform2f(m_Program, location, x, y);
    }

    void Shader::uniform3f(const int location, const float x, const float y, const float z) const {
        glProgramUniform3f(m_Program, location, x, y, z);
    }

    void Shader::uniform4f(const int location, const float x, const float y, const float z, const float w) const {
        glProgramUniform4f(m_Program, location, x, y, z, w);
    }

    void Shader::uniform2f(const int location, const ::glm::vec2 &v) const {
        glProgramUniform2fv(m_Program, location, 1, ::glm::value_ptr(v));
    }

    void Shader::uniform3f(const int location, const ::glm::vec3 &v) const {
        glProgramUniform3fv(m_Program, location, 1, ::glm::value_ptr(v));
    }

    void Shader::uniform4f(const int location, const ::glm::vec4 &v) const {
        glProgramUniform4fv(m_Program, location, 1, ::glm::value_ptr(v));
    }

    void Shader::uniformMatrix2f(const int location, const ::glm::mat2 &v) const {
        glProgramUniformMatrix2fv(m_Program, location, 1, GL_FALSE, ::glm::value_ptr(v));
    }

    void Shader::uniformMatrix3f(const int location, const ::glm::mat3 &v) const {
        glProgramUniformMatrix3fv(m_Program, location, 1, GL_FALSE, ::glm::value_ptr(v));
    }

    void Shader::uniformMatrix4f(const int location, const ::glm::mat4 &v) const {
        glProgramUniformMatrix4fv(m_Program, location, 1, GL_FALSE, ::glm::value_ptr(v));
    }

    void Shader::uniformMatrix2x3f(const int location, const ::glm::mat2x3 &v) const {
        glProgramUniformMatrix2x3fv(m_Program, location, 1, GL_FALSE, ::glm::value_ptr(v));
    }

    void Shader::uniformMatrix2x4f(const int location, const ::glm::mat2x4 &v) const {
        glProgramUniformMatrix2x4fv(m_Program, location, 1, GL_FALSE, ::glm::value_ptr(v));
    }

    void Shader::uniformMatrix3x2f(const int location, const ::glm::mat3x2 &v) const {
        glProgramUniformMatrix3x2fv(m_Program, location, 1, GL_FALSE, ::glm::value_ptr(v));
    }

    void Shader::uniformMatrix3x4f(const int location, const ::glm::mat3x4 &v) const {
        glProgramUniformMatrix3x4fv(m_Program, location, 1, GL_FALSE, ::glm::value_ptr(v));
    }

    void Shader::uniformMatrix4x2f(const int location, const ::glm::mat4x2 &v) const {
        glProgramUniformMatrix4x2fv(m_Program, location, 1, GL_FALSE, ::glm::value_ptr(v));
    }

    void Shader::uniformMatrix4x3f(const int location, const ::glm::mat4x3 &v) const {
        glProgramUniformMatrix4x3fv(m_Program, location, 1, GL_FALSE, ::glm::value_ptr(v));
    }

    void Shader::uniform1d(const int location, const double x) const {
        glProgramUniform1d(m_Program, location, x);
    }

    void Shader::uniform2d(const int location, const double x, const double y) const {
        glProgramUniform2d(m_Program, location, x, y);
    }

    void Shader::uniform3d(const int location, const double x, const double y, const double z) const {
        glProgramUniform3d(m_Program, location, x, y, z);
    }

    void Shader::uniform4d(const int location, const double x, const double y, const double z, const double w) const {
        glProgramUniform4d(m_Program, location, x, y, z, w);
    }

    void Shader::uniform2d(const int location, const ::glm::dvec2 &v) const {
        glProgramUniform2dv(m_Program, location, 1, ::glm::value_ptr(v));
    }

    void Shader::uniform3d(const int location, const ::glm::dvec3 &v) const {
        glProgramUniform3dv(m_Program, location, 1, ::glm::value_ptr(v));
    }

    void Shader::uniform4d(const int location, const ::glm::dvec4 &v) const {
        glProgramUniform4dv(m_Program, location, 1, ::glm::value_ptr(v));
    }

    void Shader::uniformMatrix2d(const int location, const ::glm::dmat2 &v) const {
        glProgramUniformMatrix2dv(m_Program, location, 1, GL_FALSE, ::glm::value_ptr(v));
    }

    void Shader::uniformMatrix3d(const int location, const ::glm::dmat3 &v) const {
        glProgramUniformMatrix3dv(m_Program, location, 1, GL_FALSE, ::glm::value_ptr(v));
    }

    void Shader::uniformMatrix4d(const int location, const ::glm::dmat4 &v) const {
        glProgramUniformMatrix4dv(m_Program, location, 1, GL_FALSE, ::glm::value_ptr(v));
    }

    void Shader::uniformMatrix2x3d(const int location, const ::glm::dmat2x3 &v) const {
        glProgramUniformMatrix2x3dv(m_Program, location, 1, GL_FALSE, ::glm::value_ptr(v));
    }

    void Shader::uniformMatrix2x4d(const int location, const ::glm::dmat2x4 &v) const {
        glProgramUniformMatrix2x4dv(m_Program, location, 1, GL_FALSE, ::glm::value_ptr(v));
    }

    void Shader::uniformMatrix3x2d(const int location, const ::glm::dmat3x2 &v) const {
        glProgramUniformMatrix3x2dv(m_Program, location, 1, GL_FALSE, ::glm::value_ptr(v));
    }

    void Shader::uniformMatrix3x4d(const int location, const ::glm::dmat3x4 &v) const {
        glProgramUniformMatrix3x4dv(m_Program, location, 1, GL_FALSE, ::glm::value_ptr(v));
    }

    void Shader::uniformMatrix4x2d(const int location, const ::glm::dmat4x2 &v) const {
        glProgramUniformMatrix4x2dv(m_Program, location, 1, GL_FALSE, ::glm::value_ptr(v));
    }

    void Shader::uniformMatrix4x3d(const int location, const ::glm::dmat4x3 &v) const {
        glProgramUniformMatrix4x3dv(m_Program, location, 1, GL_FALSE, ::glm::value_ptr(v));
    }

    void Shader::uniform1ui(const int location, const unsigned int x) const {
        glProgramUniform1ui(m_Program, location, x);
    }

    void Shader::uniform2ui(const int location, const unsigned int x, const unsigned int y) const {
        glProgramUniform2ui(m_Program, location, x, y);
    }

    void Shader::uniform3ui(const int location, const unsigned int x, const unsigned int y, const unsigned int z) const {
        glProgramUniform3ui(m_Program, location, x, y, z);
    }

    void Shader::uniform4ui(const int location, const unsigned int x, const unsigned int y, const unsigned int z, const unsigned int w) const {
        glProgramUniform4ui(m_Program, location, x, y, z, w);
    }

    void Shader::uniform2ui(const int location, const ::glm::uvec2 &v) const {
        glProgramUniform2uiv(m_Program, location, 1, ::glm::value_ptr(v));
    }

    void Shader::uniform3ui(const int location, const ::glm::uvec3 &v) const {
        glProgramUniform3uiv(m_Program, location, 1, ::glm::value_ptr(v));
    }

    void Shader::uniform4ui(const int location, const ::glm::uvec4 &v) const {
        glProgramUniform4uiv(m_Program, location, 1, ::glm::value_ptr(v));
    }

    void Shader::uniform1i(const int location, const int x) const {
        glProgramUniform1i(m_Program, location, x);
    }

    void Shader::uniform2i(const int location, const int x, const int y) const {
        glProgramUniform2i(m_Program, location, x, y);
    }

    void Shader::uniform3i(const int location, const int x, const int y, const int z) const {
        glProgramUniform3i(m_Program, location, x, y, z);
    }

    void Shader::uniform4i(const int location, const int x, const int y, const int z, const int w) const {
        glProgramUniform4i(m_Program, location, x, y, z, w);
    }

    void Shader::uniform2i(const int location, const ::glm::ivec2 &v) const {
        glProgramUniform2iv(m_Program, location, 1, ::glm::value_ptr(v));
    }

    void Shader::uniform3i(const int location, const ::glm::ivec3 &v) const {
        glProgramUniform3iv(m_Program, location, 1, ::glm::value_ptr(v));
    }

    void Shader::uniform4i(const int location, const ::glm::ivec4 &v) const {
        glProgramUniform4iv(m_Program, location, 1, ::glm::value_ptr(v));
    }

    void Shader::use() const {
        glUseProgram(m_Program);
    }

    Texture::Texture(const Type type) : m_Texture(~0U), m_Type(type) {
        glCreateTextures(static_cast<GLenum>(type), 1, &m_Texture);
    }

    Texture::~Texture() {
        glDeleteTextures(1, &m_Texture);
    }

    void Texture::bind() const {
        glBindTexture(static_cast<GLenum>(m_Type), m_Texture);
    }

    std::shared_ptr<Texture> Texture::create2d(const int width, const int height, const Format format, const InternalFormat internal_format, const DataType dataType,
                                               const void *data) {
        auto texture = std::make_shared<Texture>(Type::Texture2D);
        texture->image2d(width, height);
        return texture;
    }

    void Texture::image2d(const int width, const int height, const Format format, const InternalFormat internalFormat, const DataType dataType, const void *data) const {
        image2d(width, height, 0, format, internalFormat, dataType, data);
    }

    void Texture::image2d(const int width, const int height, const int level, const Format format, const InternalFormat internal_format, const DataType dataType,
                          const void *data) const {
        bind();
        glTexImage2D(static_cast<GLenum>(m_Type), level, static_cast<GLint>(internal_format), width, height, 0, static_cast<GLenum>(format), static_cast<GLenum>(dataType), data);
        glBindTexture(static_cast<GLenum>(m_Type), 0);
    }

    Renderbuffer::Renderbuffer(Texture::InternalFormat internalFormat, const int width, const int height, const int samples) : m_Renderbuffer(~0U) {
        glCreateRenderbuffers(1, &m_Renderbuffer);
        if (samples <= 0) {
            glNamedRenderbufferStorage(m_Renderbuffer, static_cast<GLenum>(internalFormat), width, height);
        } else {
            glNamedRenderbufferStorageMultisample(m_Renderbuffer, samples, static_cast<GLenum>(internalFormat), width, height);
        }
    }

    Renderbuffer::~Renderbuffer() {
        glDeleteRenderbuffers(1, &m_Renderbuffer);
    }

    void Renderbuffer::bind() const {
        glBindRenderbuffer(GL_RENDERBUFFER, m_Renderbuffer);
    }

    void Renderbuffer::unbind() {}

    Framebuffer::Framebuffer() : m_Framebuffer(~0U) {
        glCreateFramebuffers(1, &m_Framebuffer);
    }

    Framebuffer::~Framebuffer() {
        glDeleteFramebuffers(1, &m_Framebuffer);
    }

    bool Framebuffer::is_complete() const {
        return glCheckNamedFramebufferStatus(m_Framebuffer, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    }

    void Framebuffer::bind(FramebufferTarget target) const {
        glBindFramebuffer(static_cast<GLenum>(target), m_Framebuffer);
    }

    void Framebuffer::unbind(FramebufferTarget target) {
        glBindFramebuffer(static_cast<GLenum>(target), 0);
    }

    void Framebuffer::attach_color_texture(const std::shared_ptr<Texture> &texture, const uint8_t index, const int level) const {
        glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0 + index, texture->handle(), level);
    }

    void Framebuffer::attach_depth_texture(const std::shared_ptr<Texture> &texture, const int level) const {
        glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_ATTACHMENT, texture->handle(), level);
    }

    void Framebuffer::attach_stencil_texture(const std::shared_ptr<Texture> &texture, const int level) const {
        glNamedFramebufferTexture(m_Framebuffer, GL_STENCIL_ATTACHMENT, texture->handle(), level);
    }

    void Framebuffer::attach_depth_stencil_texture(const std::shared_ptr<Texture> &texture, const int level) const {
        glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, texture->handle(), level);
    }
} // namespace neuron
