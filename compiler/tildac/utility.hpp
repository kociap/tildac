#ifndef TILDAC_UTILITY_HPP_INCLUDE
#define TILDAC_UTILITY_HPP_INCLUDE

#undef max
#undef min

namespace tildac {
    template<typename T>
    T min(T a, T b) {
        return a < b ? a : b;
    }

    template<typename T>
    T max(T a, T b) {
        return a > b ? a : b;
    }

    template<typename T>
    T clamp(T v, T low, T high) {
        return min(max(v, low), high);
    }

    template<typename T>
    struct Owning_Ptr {
    public:
        Owning_Ptr(): _pointer(nullptr) {}
        Owning_Ptr(T* ptr): _pointer(ptr) {}
        Owning_Ptr(Owning_Ptr const&) = delete;
        Owning_Ptr& operator=(Owning_Ptr const&) = delete;

        Owning_Ptr(Owning_Ptr&& other): _pointer(other._pointer) {
            other._pointer = nullptr;
        }

        Owning_Ptr& operator=(Owning_Ptr&& other) {
            T* tmp = _pointer;
            _pointer = other._pointer;       
            other._pointer = tmp;
            return *this;
        }

        ~Owning_Ptr() {
            delete _pointer;
        }

        [[nodiscard]] operator bool() {
            return _pointer;
        }

        [[nodiscard]] T& operator*() const {
            return *_pointer;
        }

        [[nodiscard]] T* operator->() const {
            return _pointer;
        }

        [[nodiscard]] T* release() {
            T* pointer = _pointer;
            _pointer = nullptr;
            return pointer;
        }

    private:
        T* _pointer;
    };

    template<typename T>
    Owning_Ptr(T*) -> Owning_Ptr<T>;
}

#endif // !TILDAC_UTILITY_HPP_INCLUDE
