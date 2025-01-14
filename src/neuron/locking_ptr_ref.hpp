#pragma once

#include <memory>
#include <shared_mutex>
#include <unordered_map>

namespace neuron {
    template <typename T, typename R>
    concept any_reference = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<R>>;

    template <typename T, typename R, typename I>
    concept indexable_for = requires(const T &t, I index) {
        { t.at(index) } -> any_reference<R>;
    };


    template <typename T>
    class locking_ptr_ref {
      private:
        std::shared_mutex        &mutex;
        const std::unique_ptr<T> &ptr;

      public:
        template <typename I, indexable_for<std::unique_ptr<T>, I> C>
        inline static locking_ptr_ref from_indexing(std::shared_mutex &mutex, const C &container, I index) {
            mutex.lock_shared();
            return locking_ptr_ref(mutex, container.at(index));
        }

        locking_ptr_ref(std::shared_mutex &mutex, const std::unique_ptr<T> &ptr) : mutex(mutex), ptr(ptr) {}

        ~locking_ptr_ref() { mutex.unlock_shared(); }

        // no copy, allow move
        locking_ptr_ref(const locking_ptr_ref &other)            = delete;
        locking_ptr_ref &operator=(const locking_ptr_ref &other) = delete;
        locking_ptr_ref(locking_ptr_ref &&other)                 = default;
        locking_ptr_ref &operator=(locking_ptr_ref &&other)      = default;

        inline T *operator->() const { return ptr.get(); }

        inline T &operator*() const { return *ptr.get(); }
    };
} // namespace neuron
