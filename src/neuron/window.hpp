#pragma once

#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <string_view>

namespace neuron {

    class Window final {
      public:
        Window(std::string_view title, const glm::uvec2 &size);
        ~Window();

        [[nodiscard]] bool isOpen() const;
        void               close() const;

        static void pollEvents();
        void        swap() const;

        [[nodiscard]] inline GLFWwindow* handle() const { return m_Window; };

      private:
        GLFWwindow *m_Window;
    };

} // namespace neuron
