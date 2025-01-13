#pragma once

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

    struct GlobalTransformMatrix {
        glm::mat4 matrix;
    };

    struct Visible {};
}