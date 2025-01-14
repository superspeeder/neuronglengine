#pragma once
#include "asset.hpp"
#include "neuron/glwrap.hpp"

namespace neuron::asset {

    class Framebuffer final : public Asset {
      public:
        Framebuffer();
        ~Framebuffer() override = default;

        [[nodiscard]] inline bool isDefaultFBO() const { return m_FramebufferObject == nullptr; }

      private:
        std::unique_ptr<neuron::Framebuffer> m_FramebufferObject;


    };

} // namespace neuron::asset
