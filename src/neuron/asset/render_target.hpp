#pragma once
#include "asset.hpp"
#include "framebuffer.hpp"

namespace neuron::asset {

    class RenderTarget final : public Asset {
      public:
        RenderTarget();
        ~RenderTarget() override = default;

      private:
        AssetHandle<Framebuffer> m_Framebuffer;
    };

} // namespace neuron::assets
