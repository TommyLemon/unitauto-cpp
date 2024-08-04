#include <string>
#include <unordered_map>
#include "nlohmann/json.hpp"
#include <functional>
#include <any>
#include <map>
#include <vector>

namespace unitauto {
    using json = nlohmann::json;

    static std::unordered_map<std::string, std::function<void*(const std::string&)>> TYPE_MAP;

    // 对象转 JSON 字符串
    // static std::string obj_2_json(const std::any& obj) {
    //     auto j = nlohmann::to_json(obj);
    // }

    // JSON 字符串转对应类型的对象
    static void* json_2_obj(const std::string& str, const std::string& type) {
        auto it = TYPE_MAP.find(type);
        if (it != TYPE_MAP.end()) {
            return it->second(str);
        }

        throw std::runtime_error("Unknown type: "+ type + ", call add_type firstly!");
    }

    // 对象转对象
    // static void* obj_2_obj(const std::any& obj, const std::string& type) {
    //     auto str = obj_2_json(obj);
    //     return json_2_obj(str, type);
    // }

    // 删除对象
    template<typename T>
    static void del_obj(void* obj) {
        delete static_cast<T*>(obj);
    }

    // 注册类型
    template<typename T>
    static void add_type(const std::string& type) {
        TYPE_MAP[type] = [](const std::string& str) -> void* {
            json j = json::parse(str);
            T* obj = new T(j.get<T>());
            return static_cast<void*>(obj);
        };
    }

    // 取消注册类型
    static void remove_type(const std::string& type) {
        TYPE_MAP.erase(type);
    }


    // 函数与方法(成员函数) <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    using FT = std::function<std::any(std::vector<std::any>)>;
    static std::map<std::string, FT> FUNC_MAP;

    // 执行已注册的函数/方法(成员函数)
    static std::any invoke(const std::string &name, std::vector<std::any> args) {
        auto it = FUNC_MAP.find(name);
        if (it != FUNC_MAP.end()) {
            return it->second(args);
        }
        throw std::runtime_error("Unkown func: " + name + ", call add_func/add_func firstly!");
    }

    // 执行非 void 函数
    template<typename Ret, typename... Args, std::size_t... I>
    static std::any invoke(std::function<Ret(Args...)> func, std::vector<std::any> &args, std::index_sequence<I...>) {
        return func(std::any_cast<Args>(args[I])...);
    }

    // 执行 void 函数
    template<typename... Args, std::size_t... I>
    static void invoke_void(std::function<void(Args...)> func, std::vector<std::any> &args, std::index_sequence<I...>) {
        func(std::any_cast<Args>(args[I])...);
    }

    // 执行非 void 方法(成员函数)
    template<typename Ret, typename T, typename... Args, std::size_t... I>
    static std::any invoke(T *instance, Ret (T::*func)(Args...), std::vector<std::any> &args, std::index_sequence<I...>) {
        return (instance->*func)(std::any_cast<Args>(args[I])...);
    }

    // 执行 void 方法(成员函数)
    template<typename T, typename... Args, std::size_t... I>
    static void invoke_void(T *instance, void (T::*func)(Args...), std::vector<std::any> &args, std::index_sequence<I...>) {
        (instance->*func)(std::any_cast<Args>(args[I])...);
    }

    // 注册函数
    template<typename Ret, typename... Args>
    static void add_func(const std::string &name, std::function<Ret(Args...)> func) {
        FUNC_MAP[name] = [func](std::vector<std::any> args) -> std::any {
            if constexpr (std::is_void_v<Ret>) {
                invoke_void(func, args, std::index_sequence_for<Args...>{});
                return {};
            } else {
                return invoke(func, args, std::index_sequence_for<Args...>{});
            }
        };
    }

    // 注册方法(成员函数)
    template<typename Ret, typename T, typename... Args>
    static void add_func(const std::string &name, T *instance, Ret (T::*func)(Args...)) {
        // if (instance == nullptr) {
        //     FUNC_MAP[name] = [func](std::vector<std::any> args) -> std::any {
        //         if constexpr (std::is_void_v<Ret>) {
        //             invoke_void(func, args, std::index_sequence_for<Args...>{});
        //             return {};
        //         } else {
        //             return invoke(func, args, std::index_sequence_for<Args...>{});
        //         }
        //     };
        //     return;
        // }

        FUNC_MAP[name] = [instance, func](std::vector<std::any> args) -> std::any {
            if constexpr (std::is_void_v<Ret>) {
                invoke_void(instance, func, args, std::index_sequence_for<Args...>{});
                return {};
            } else {
                return invoke(instance, func, args, std::index_sequence_for<Args...>{});
            }
        };
    }

    // 函数与方法(成员函数) >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

}
