//
// Created by andy on 1/14/25.
//

#include "shader.hpp"


namespace neuron::asset {
    std::unique_ptr<Shader> Shader::create(const std::vector<std::shared_ptr<neuron::ShaderModule>> &modules) {
        return std::make_unique<Shader>(std::make_shared<neuron::Shader>(modules));
    }
} // asset
// neuron
