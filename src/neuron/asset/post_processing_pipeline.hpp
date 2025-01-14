#pragma once
#include "asset.hpp"

namespace neuron::asset {

    class PostProcessingPipeline final : public Asset {
      public:
        PostProcessingPipeline();
        ~PostProcessingPipeline() override = default;

      private:
    };

} // namespace neuron::asset
