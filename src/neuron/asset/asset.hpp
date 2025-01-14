#pragma once

#include "neuron/locking_ptr_ref.hpp"


#include <atomic>
#include <concepts>
#include <memory>
#include <shared_mutex>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>

namespace neuron::asset {

    // Assets are basically useless without extra data
    class Asset {
    public:
        Asset()          = default;
        virtual ~Asset() = default;

        Asset(const Asset &other)                = delete;
        Asset(Asset &&other) noexcept            = default;
        Asset &operator=(const Asset &other)     = delete;
        Asset &operator=(Asset &&other) noexcept = default;
    };

    template <std::derived_from<Asset> T>
    using AssetRef = locking_ptr_ref<T>;

    template <std::derived_from<Asset> T>
    class AssetTable;

    template <std::derived_from<Asset> T>
    class AssetHandle {
    public:
        using handle_t = uint64_t;
        using asset_t  = T;

        inline AssetHandle() : handle(UINT64_MAX) {
        }


        inline AssetRef<T> getFromGlobal();

    private:
        handle_t handle;

        inline explicit AssetHandle(const handle_t handle) : handle(handle) {
        }

        friend class AssetTable<T>;
    };

    struct AssetTableBase {
        virtual ~AssetTableBase() = default;
    };

    inline static std::unordered_set<std::shared_ptr<AssetTableBase>> tables_to_delete;


    template <std::derived_from<Asset> T>
    class AssetTable : public AssetTableBase {
    public:
        using handle_t    = AssetHandle<T>;
        using asset_ref_t = AssetRef<T>;

        ~AssetTable() override = default;

        /// Requires that the unique_ptr is moved into the function
        [[nodiscard]] inline handle_t initAsset(std::unique_ptr<T> &&asset) {
            auto handle = m_Counter++;
            m_TableMutex.lock();
            m_Table[handle] = std::move(asset);
            m_TableMutex.unlock();
            return handle_t(handle);
        }

        inline void replaceAsset(handle_t handle, std::unique_ptr<T> &&asset) {
            m_TableMutex.lock();
            m_Table[handle.handle] = std::move(asset);
            m_TableMutex.unlock();
        }

        [[nodiscard]] inline asset_ref_t getAsset(handle_t handle) const {
            return locking_ptr_ref<T>::from_indexing(m_TableMutex, m_Table, handle.handle);
        }

        inline void releaseAsset(handle_t handle) {
            m_TableMutex.lock();
            m_Table.erase(handle.handle);
            m_TableMutex.unlock();
        }

        inline static std::shared_ptr<AssetTable<T>> globalTable() {
            static std::weak_ptr<AssetTable<T>> globalTable = AssetTable::new_global();
            return globalTable.lock();
        }

        AssetTable() = default;

    private:
        inline static std::weak_ptr<AssetTable> new_global() {
            std::shared_ptr<AssetTable> atbl = std::make_shared<AssetTable>();
            tables_to_delete.insert(atbl);
            return atbl;
        }

        std::atomic<typename handle_t::handle_t> m_Counter;

        mutable std::shared_mutex                                           m_TableMutex;
        std::unordered_map<typename handle_t::handle_t, std::unique_ptr<T>> m_Table;
    };

    template <std::derived_from<Asset> T>
    std::shared_ptr<AssetTable<T>> assetTable() {
        return AssetTable<T>::globalTable();
    }

    template <std::derived_from<Asset> T>
    AssetRef<T> AssetHandle<T>::getFromGlobal() {
        return assetTable<T>()->getAsset(*this);
    }

    inline void cleanupAssetTables() {
        tables_to_delete.clear();
    }
} // namespace neuron::asset
