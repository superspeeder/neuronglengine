#include "neuron/glwrap.hpp"
#include "neuron/mesh.hpp"
#include "neuron/window.hpp"

#include <iostream>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "neuron/asset/asset.hpp"
#include "neuron/asset/framebuffer.hpp"
#include "neuron/asset/mesh.hpp"
#include "neuron/asset/post_processing_pipeline.hpp"
#include "neuron/asset/shader.hpp"

using neuron::asset::assetTable;

template <typename T>
using vfn = T (*)();

template <typename T1, typename T2, vfn<T1> En, vfn<T2> Ex>
struct entry_exit_wrapper_ {
    inline entry_exit_wrapper_() { En(); }

    inline ~entry_exit_wrapper_() { Ex(); }
};

#define entry_exit_wrapper(en, ex) entry_exit_wrapper_<decltype(en()), decltype(ex()), en, ex>

using glfw_lib_obj = entry_exit_wrapper(glfwInit, glfwTerminate);

struct Vertex {
    glm::vec4 position;
    glm::vec4 color;
    glm::vec4 normal;
    glm::vec2 uv;
};

int main() {
    char modelPath[260] = "res/test.glb";

    glfw_lib_obj glfw;

    const auto window = std::make_shared<neuron::Window>("Wheeeeee!", glm::uvec2{800, 600});

    neuron::asset::AssetHandle<neuron::asset::Shader> shader;

    {
        const auto vsh = neuron::ShaderModule::load("res/vert.glsl", neuron::ShaderModule::Type::Vertex);
        const auto fsh = neuron::ShaderModule::load("res/frag.glsl", neuron::ShaderModule::Type::Fragment);

        shader = assetTable<neuron::asset::Shader>()->initAsset(neuron::asset::Shader::create(std::vector{vsh, fsh}));
    }

    auto mesh_handle = assetTable<neuron::asset::Mesh>()->initAsset(neuron::asset::Mesh::load("res/test.glb"));

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window->handle(), true);
    ImGui_ImplOpenGL3_Init("#version 460 core");

    auto &io = ImGui::GetIO();


    glm::mat4 projection = glm::perspective(90.0f, 4.0f / 3.0f, 0.1f, 100.0f);

    glm::vec3 sunDirection = glm::normalize(glm::vec3(0.0f, -0.1f, -1.0f));

    glm::vec3 sunColor     = glm::vec3(1.0f);
    glm::vec3 ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);

    glm::vec3 modelPosition = glm::vec3(0.0f);
    glm::vec3 modelScale    = glm::vec3(0.5f);

    float xtheta = 0.0f;
    float ytheta = 0.0f;

    glm::vec3 eyePosition = glm::vec3(glm::cos(xtheta), ytheta, glm::sin(xtheta));

    float specularStrength = 1.5f;

    double thisFrame = glfwGetTime();
    double deltaTime = 1.0f / 60.0f;
    double lastFrame = thisFrame - deltaTime;

    float zoom = 1.0f;

    while (window->isOpen()) {
        neuron::Window::pollEvents();
        int w, h;
        glfwGetFramebufferSize(window->handle(), &w, &h);
        glViewport(0, 0, w, h);

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        if (!io.WantCaptureKeyboard) {
            if (glfwGetKey(window->handle(), GLFW_KEY_A)) {
                xtheta += deltaTime;
            }

            if (glfwGetKey(window->handle(), GLFW_KEY_D)) {
                xtheta -= deltaTime;
            }

            if (glfwGetKey(window->handle(), GLFW_KEY_W)) {
                ytheta += deltaTime;
            }

            if (glfwGetKey(window->handle(), GLFW_KEY_S)) {
                ytheta -= deltaTime;
            }

            if (glfwGetKey(window->handle(), GLFW_KEY_MINUS)) {
                zoom += deltaTime;
            }

            if (glfwGetKey(window->handle(), GLFW_KEY_EQUAL)) {
                zoom -= deltaTime;
            }
        }

        ytheta = glm::clamp(ytheta, -1.0f, 1.0f);

        zoom = glm::clamp(zoom, 0.1f, 5.0f);

        eyePosition = glm::vec3(glm::cos(xtheta) * glm::cos(ytheta) * zoom, glm::sin(ytheta) * zoom, glm::sin(xtheta) * glm::cos(ytheta) * zoom);

        glm::mat4 view = glm::lookAt(eyePosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 model           = glm::scale(glm::translate(glm::identity<glm::mat4>(), modelPosition), modelScale);
        glm::mat3 modelNormMatrix = glm::mat3(glm::transpose(glm::inverse(model)));

        {
            auto sh = shader.getFromGlobal();
            auto mesh = mesh_handle.getFromGlobal();

            sh->object()->use();
            sh->object()->uniformMatrix4f("uViewProjection", projection * view);
            sh->object()->uniformMatrix4f("uModel", model);
            sh->object()->uniformMatrix3f("uModelNormal", modelNormMatrix);

            sh->object()->uniform3f("uSunDirection", sunDirection);
            sh->object()->uniform3f("uSunLight", sunColor);
            sh->object()->uniform3f("uAmbientLight", ambientColor);
            sh->object()->uniform3f("uEyePosition", eyePosition);
            sh->object()->uniform1f("uSpecularStrength", specularStrength);

            mesh->object()->draw();
        }


        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        if (ImGui::Begin("Debug")) {
            ImGui::Text("Camera");
            ImGui::BeginDisabled();
            ImGui::InputFloat3("Eye Position", glm::value_ptr(eyePosition), "%.2f");

            ImGui::InputFloat("Theta X", &xtheta);
            ImGui::InputFloat("Theta Y", &ytheta);
            ImGui::InputFloat("Zoom", &zoom);
            ImGui::EndDisabled();

            ImGui::Text("Basic Lighting & Material");
            ImGui::InputFloat("Specular Strength", &specularStrength);
            ImGui::ColorEdit3("Ambient Light Color", glm::value_ptr(ambientColor));

            ImGui::Text("Sun Settings");
            ImGui::InputFloat3("Sun Direction", glm::value_ptr(sunDirection), "%.2f");
            ImGui::ColorEdit3("Sun Light Color", glm::value_ptr(sunColor));

            ImGui::Spacing();

            ImGui::Text("FPS: %d", static_cast<int>(round(1.0 / deltaTime)));

            if (ImGui::Button("Reload Shaders")) {
                const auto vsh = neuron::ShaderModule::load("res/vert.glsl", neuron::ShaderModule::Type::Vertex);
                const auto fsh = neuron::ShaderModule::load("res/frag.glsl", neuron::ShaderModule::Type::Fragment);
                assetTable<neuron::asset::Shader>()->replaceAsset(shader, neuron::asset::Shader::create(std::vector{vsh, fsh}));
            }

            ImGui::Spacing();
            ImGui::InputText("Model Filename", modelPath, 260);

            if (ImGui::Button("Reload Model")) {
                if (std::filesystem::exists(modelPath)) {
                    assetTable<neuron::asset::Mesh>()->replaceAsset(mesh_handle, neuron::asset::Mesh::load(modelPath));
                }
            }
        }
        ImGui::End();

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window->swap();

        lastFrame = thisFrame;
        thisFrame = glfwGetTime();
        deltaTime = thisFrame - lastFrame;
    }

    neuron::asset::cleanupAssetTables();

    std::cout << "Test" << std::endl;
    return 0;
}
