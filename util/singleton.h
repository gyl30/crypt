#ifndef LEAF_CRYPT_UTIL_SINGLETON_H
#define LEAF_CRYPT_UTIL_SINGLETON_H

#include <mutex>

namespace leaf
{
template <typename T>
class singleton
{
   public:
    singleton() = delete;
    ~singleton() = delete;

    static T& instance()
    {
        std::call_once(flag_, &singleton::init);
        return *value_;
    }

   private:
    static void init()
    {
        value_ = new T();
        ::atexit(destroy);
    }

    static void destroy()
    {
        delete value_;
        value_ = nullptr;
    }

   private:
    static std::once_flag flag_;
    static T* value_;
};

template <typename T>
std::once_flag singleton<T>::flag_;

template <typename T>
T* singleton<T>::value_ = nullptr;

}    // namespace leaf
#endif
