cmake_minimum_required(VERSION 3.30)
project(glengine)

set(CMAKE_CXX_STANDARD 23)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(flecs CONFIG REQUIRED)

add_subdirectory(glad)

add_executable(glengine src/main.cpp
        src/neuron/window.cpp
        src/neuron/window.hpp
        src/neuron/glwrap.cpp
        src/neuron/glwrap.hpp
        src/neuron/mesh.cpp
        src/neuron/mesh.hpp
        src/neuron/scene/scene.cpp
        src/neuron/scene/scene.hpp
        src/neuron/ecs/components.hpp
)
target_include_directories(glengine PUBLIC src/)
target_link_libraries(glengine PUBLIC glfw glm::glm glad::glad assimp::assimp imgui::imgui $<IF:$<TARGET_EXISTS:flecs::flecs>,flecs::flecs,flecs::flecs_static>)
target_compile_definitions(glengine PUBLIC -DGLM_ENABLE_EXPERIMENTAL)