#pragma once
#include "asset.hpp"
#include "neuron/mesh.hpp"

#include <memory>

namespace neuron::asset {

    class Mesh final : public Asset {
    public:

        explicit Mesh(std::shared_ptr<neuron::Mesh> mesh) : m_Mesh(std::move(mesh)) {}
        ~Mesh() override = default;

        static std::unique_ptr<Mesh> load(const std::filesystem::path &path);

        [[nodiscard]] inline std::shared_ptr<neuron::Mesh> object() const { return m_Mesh; }

    private:
        std::shared_ptr<neuron::Mesh> m_Mesh;
    };

}
