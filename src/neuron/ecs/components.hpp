#pragma once

#include <flecs/addons/cpp/flecs.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace neuron::ecs {
    struct Position {
        glm::vec3 position;
    };

    struct Rotation {
        glm::quat rotation;
    };

    struct Scale {
        glm::vec3 scale;
    };

    struct CalculatedTransformMatrix {
        glm::mat4 matrix;
    };

    // This will be placed on anything which has a parent with some kind of transform component (Position, Rotation, or Scale)
    struct GlobalTransformMatrix {
        glm::mat4 matrix;
    };

    struct Visibility {
        bool visible;
        bool only_self = false;
    };

    // the system will update this on everything which has any parent node along its path to the root of its part of the tree which contains a `Visibility` component
    struct CalculatedVisibility {
        bool visible;
    };

    // the system will calculate the position *after* the transform stack and place it in here
    struct GlobalPosition {
        glm::vec3 position;
    };

    struct RenderOnCameraLayer {
        flecs::entity camera_layer; // camera layers are represented as entities which are related to an entity with a camera component
    };

    /**
     * 
     */
    struct CameraLayer {

    };

    namespace tags {

        // these two are used to keep objects from calling built-in systems for entities. Not all systems can be disabled (the GlobalPosition and GlobalTransformMatrix will always be calculated, but after these would've been called).
        // you MUST provide your own implementation of systems for these or things most likely won't work right.
        struct HasCustomVisibility {
        };

        struct HasCustomTransformMatrix {
        };
    }}
