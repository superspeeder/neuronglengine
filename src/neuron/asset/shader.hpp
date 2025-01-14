#pragma once
#include <utility>

#include "neuron/asset/asset.hpp"
#include "neuron/glwrap.hpp"

namespace neuron::asset {
    class Shader : public Asset {
    public:
        inline explicit Shader(std::shared_ptr<neuron::Shader> shader) : m_Shader(std::move(shader)) {
        }

        [[nodiscard]] std::shared_ptr<neuron::Shader> object() const {
            return m_Shader;
        }

        static std::unique_ptr<Shader> create(const std::vector<std::shared_ptr<neuron::ShaderModule>>& modules);

    private:
        std::shared_ptr<neuron::Shader> m_Shader;
    };

}
