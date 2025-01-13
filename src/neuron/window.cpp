#include "window.hpp"

namespace neuron {
    Window::Window(const std::string_view title, const glm::uvec2 &size) {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef NDEBUG
        glfwWindowHint(GLFW_CONTEXT_DEBUG, GLFW_TRUE);
#endif

        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

        glfwWindowHint(GLFW_SAMPLES, 8);

        m_Window = glfwCreateWindow(static_cast<int>(size.x), static_cast<int>(size.y), title.data(), nullptr, nullptr);
        glfwMakeContextCurrent(m_Window);
        gladLoadGL(glfwGetProcAddress);
    }

    Window::~Window() {
        glfwDestroyWindow(m_Window);
    }

    bool Window::isOpen() const {
        return !glfwWindowShouldClose(m_Window);
    }

    void Window::close() const {
        glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
    }

    void Window::pollEvents() {
        glfwPollEvents();
    }

    void Window::swap() const {
        glfwSwapBuffers(m_Window);
    }
} // neuron