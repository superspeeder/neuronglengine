#pragma once

#include "neuron/locking_ptr_ref.hpp"


#include <concepts>
#include <memory>
#include <shared_mutex>
#include <unordered_map>

namespace neuron::asset {

    class Asset;

    template<std::derived_from<Asset> T>
    using AssetRef = locking_ptr_ref<T>;

    template<std::derived_from<Asset> T>
    class AssetTable;

    template<std::derived_from<Asset> T>
    class AssetHandle {
    public:
        using handle_t = uint64_t;
        using asset_t = T;

    private:
        handle_t handle;

        inline explicit AssetHandle(const handle_t handle) : handle(handle) {}

        friend class AssetTable<T>;
    };

    // Assets are basically useless without extra data
    class Asset {
      public:
        Asset();
        virtual ~Asset() = default;
    };

    template<std::derived_from<Asset> T>
    class AssetTable {
    public:
        using handle_t = AssetHandle<T>;
        using asset_ref_t = AssetRef<T>;

        /// Requires that the unique_ptr is moved into the function
        [[nodiscard]] inline handle_t initAsset(std::unique_ptr<T>&& asset) {
            auto handle = m_Counter++;
            m_TableMutex.lock();
            m_Table[handle] = std::move(asset);
            m_TableMutex.unlock();
            return handle_t(handle);
        };

        [[nodiscard]] inline asset_ref_t getAsset(handle_t handle) const {
            return locking_ptr_ref<T>::from_indexing(m_TableMutex, m_Table, handle.handle);
        }

        inline void releaseAsset(handle_t handle) {
            m_TableMutex.lock();
            m_Table.erase(handle.handle);
            m_TableMutex.unlock();
        }

    private:
        std::atomic<typename handle_t::handle_t> m_Counter;

        mutable std::shared_mutex m_TableMutex;
        std::unordered_map<typename handle_t::handle_t, std::unique_ptr<Asset>> m_Table;
    };



    class AssetManager {
    public:


    private:

    };
} // namespace neuron::asset
