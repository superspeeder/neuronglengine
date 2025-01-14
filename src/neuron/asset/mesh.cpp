//
// Created by andy on 1/14/25.
//

#include "mesh.hpp"

namespace neuron::asset {
    std::unique_ptr<Mesh> Mesh::load(const std::filesystem::path &path) {
        if (path.extension() == ".nmesh") {
            return std::make_unique<Mesh>(neuron::Mesh::loadFromNMeshFile(path));
        }

        // TODO: replace this with a multiloader from assimp
        return std::make_unique<Mesh>(neuron::Mesh::loadWithAssimp(path)[0]);
    }
}
