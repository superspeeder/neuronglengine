#pragma once

#include <flecs.h>

namespace neuron::scene {

    class Scene {
      public:
        Scene();
        virtual ~Scene() = default;

      private:
        flecs::world m_World;

        flecs::entity m_SceneRoot;
    };

} // namespace neuron::scene
