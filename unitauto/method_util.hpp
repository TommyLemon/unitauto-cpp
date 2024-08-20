/*Copyright ©2024 TommyLemon(https://github.com/TommyLemon)

Licensed under the Apache License, Version 2.0 (the "License")
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#include <string>
#include <unordered_map>
#include "nlohmann/json.hpp"
#include <functional>
#include <any>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <typeinfo>

/**@author Lemon
 */
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
        // typeid(T).name() 会得到 4User 这种带了其它字符的名称
        TYPE_MAP[type] = [](const std::string& str) -> void* {
            if (str.empty()) {
                T* obj = new T();
                return static_cast<void*>(obj);
            }

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
        throw std::runtime_error("Unkown func: " + name + ", call add_func firstly!");
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
        if (instance == nullptr) {
            // instance = json_2_obj("", typeid(T).name());

            if (instance == nullptr) {
                instance = new T();
            }
        }

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

    static const std::string TYPE_ANY = "std::any"; // typeid(bool).name();
    static const std::string TYPE_BOOL = "bool"; // typeid(bool).name();
    static const std::string TYPE_CHAR = "char"; // typeid(char).name();
    static const std::string TYPE_BYTE = "std::byte"; // typeid(std::byte).name();
    static const std::string TYPE_SHORT = "short"; // typeid(bool).name();
    static const std::string TYPE_INT = "int"; // typeid(int).name();
    static const std::string TYPE_LONG = "long"; // typeid(long).name();
    static const std::string TYPE_LONG_LONG = "long long"; // typeid(long).name();
    static const std::string TYPE_FLOAT = "float"; // typeid(float).name();
    static const std::string TYPE_DOUBLE = "double"; // typeid(double).name();
    static const std::string TYPE_STRING = "std::string"; // typeid(std::string).name();
    // static const auto TYPE_ARR = typeid(std::array).name();
    static const std::string TYPE_ANY_ARR = "std::any[]"; // typeid(bool).name();
    static const std::string TYPE_BOOL_ARR = "bool[]"; // typeid(bool).name();
    static const std::string TYPE_CHAR_ARR = "char[]"; // typeid(char).name();
    static const std::string TYPE_BYTE_ARR = "std::byte[]"; // typeid(std::byte).name();
    static const std::string TYPE_SHORT_ARR = "short[]"; // typeid(bool).name();
    static const std::string TYPE_INT_ARR = "int[]"; // typeid(int).name();
    static const std::string TYPE_LONG_ARR = "long[]"; // typeid(long).name();
    static const std::string TYPE_LONG_LONG_ARR = "long long[]"; // typeid(long).name();
    static const std::string TYPE_FLOAT_ARR = "float[]"; // typeid(float).name();
    static const std::string TYPE_DOUBLE_ARR = "double[]"; // typeid(double).name();
    static const std::string TYPE_STRING_ARR = "std::string[]"; // typeid(std::string).name();


    // 类型转换函数映射
    static std::map<std::string, std::function<json(const std::any&)>> CAST_MAP;

    // 注册类型转换函数
    template<typename T>
    void register_cast() {
        CAST_MAP[typeid(T).name()] = [](std::any value) -> json {
            return std::any_cast<T>(value);
        };
    }


    // any_to_json 函数
    json _any_to_json(const std::any& value) {
        auto it = CAST_MAP.find(value.type().name());
        if (it != CAST_MAP.end()) {
            return it->second(value);
        }

        return (json &) value;
    }

    // any_to_json 函数模板
    json any_to_json(const std::any& value) {
        try {
            if (value.type() == typeid(bool)) {
                return std::any_cast<bool>(value);
            }
            if (value.type() == typeid(std::byte)) {
                return std::any_cast<std::byte>(value);
            }
            if (value.type() == typeid(char)) {
                return std::any_cast<char>(value);
            }
            if (value.type() == typeid(short)) {
                return std::any_cast<short>(value);
            }
            if (value.type() == typeid(int)) {
                return std::any_cast<int>(value);
            }
            if (value.type() == typeid(long)) {
                return std::any_cast<long>(value);
            }
            if (value.type() == typeid(long long)) {
                return std::any_cast<long long>(value);
            }
            if (value.type() == typeid(float)) {
                return std::any_cast<float>(value);
            }
            if (value.type() == typeid(double)) {
                return std::any_cast<double>(value);
            }
            if (value.type() == typeid(std::string)) {
                return std::any_cast<std::string>(value);
            }

            // if (value.type() == typeid(bool&)) {
            //     return std::any_cast<bool&>(value);
            // }
            // if (value.type() == typeid(std::byte&)) {
            //     return std::any_cast<std::byte&>(value);
            // }
            // if (value.type() == typeid(char&)) {
            //     return std::any_cast<char&>(value);
            // }
            // if (value.type() == typeid(short&)) {
            //     return std::any_cast<short&>(value);
            // }
            // if (value.type() == typeid(int&)) {
            //     return std::any_cast<int&>(value);
            // }
            // if (value.type() == typeid(long&)) {
            //     return std::any_cast<long&>(value);
            // }
            // if (value.type() == typeid(long long&)) {
            //     return std::any_cast<long long&>(value);
            // }
            // if (value.type() == typeid(float&)) {
            //     return std::any_cast<float&>(value);
            // }
            // if (value.type() == typeid(double&)) {
            //     return std::any_cast<double&>(value);
            // }
            // if (value.type() == typeid(std::string&)) {
            //     return std::any_cast<std::string&>(value);
            // }

            if (value.type() == typeid(std::vector<bool>)) {
                return std::any_cast<std::vector<bool>>(value);
            }
            if (value.type() == typeid(std::vector<char>)) {
                return std::any_cast<std::vector<char>>(value);
            }
            if (value.type() == typeid(std::vector<std::byte>)) {
                return std::any_cast<std::vector<std::byte>>(value);
            }
            if (value.type() == typeid(std::vector<short>)) {
                return std::any_cast<short>(value);
            }
            if (value.type() == typeid(std::vector<int>)) {
                return std::any_cast<std::vector<int>>(value);
            }
            if (value.type() == typeid(std::vector<long>)) {
                return std::any_cast<std::vector<long>>(value);
            }
            if (value.type() == typeid(std::vector<long long>)) {
                return std::any_cast<std::vector<long long>>(value);
            }
            if (value.type() == typeid(std::vector<float>)) {
                return std::any_cast<std::vector<float>>(value);
            }
            if (value.type() == typeid(std::vector<double>)) {
                return std::any_cast<std::vector<double>>(value);
            }
            if (value.type() == typeid(std::vector<std::string>)) {
                return std::any_cast<std::vector<std::string>>(value);
            }
            // if (value.type() == typeid(std::vector<std::any>)) {
            //     return (std::vector<std::any>) value; //  std::any_cast<std::vector<std::any>>(value);
            // }

            if (value.type() == typeid(std::map<std::string, bool>)) {
                return std::any_cast<std::map<std::string, bool>>(value);
            }
            if (value.type() == typeid(std::map<std::string, std::byte>)) {
                return std::any_cast<std::map<std::string, std::byte>>(value);
            }
            if (value.type() == typeid(std::map<std::string, char>)) {
                return std::any_cast<std::map<std::string, char>>(value);
            }
            if (value.type() == typeid(std::map<std::string, short>)) {
                return std::any_cast<std::map<std::string, short>>(value);
            }
            if (value.type() == typeid(std::map<std::string, int>)) {
                return std::any_cast<std::map<std::string, int>>(value);
            }
            if (value.type() == typeid(std::map<std::string, long>)) {
                return std::any_cast<std::map<std::string, long>>(value);
            }
            if (value.type() == typeid(std::map<std::string, long long>)) {
                return std::any_cast<std::map<std::string, long long>>(value);
            }
            if (value.type() == typeid(std::map<std::string, std::string>)) {
                return std::any_cast<std::map<std::string, std::string>>(value);
            }
            // if (value.type() == typeid(std::map<std::string, std::any>)) {
            //     return std::any_cast<std::map<std::string, std::any>>(value);
            // }
        } catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
        }

        return _any_to_json(value);
    }

    static std::any json_to_any(json j) {
        if (j.is_null()) {
            return nullptr;
        }

        if (j.is_number_integer()) {
            return j.get<long>();
        }

        if (j.is_number_float()) {
            return j.get<float>();
        }

        if (j.is_number()) {
            return j.get<double>();
        }

        if (j.is_string()) {
            // return j.get<std::string>();
            std::string val = j.get<std::string>();
            size_t ind = val.find(':');
            // const char *s = val.c_str();
            // auto pc = strchr(s, ':');
            // int ind = pc - s;
            if (ind == std::string::npos || ind < 0) {
                return val;
            }

            std::string type = val.substr(0, ind);
            std::string vs = val.substr(ind + 1);

            if (type == TYPE_ANY) {
                if (vs == "nullptr") {
                    return nullptr;
                }
                if (vs == "NULL") {
                    return NULL;
                }
                return vs;
            }
            if (type == TYPE_BOOL) {
                if (vs == "true") {
                    return true;
                }
                if (vs == "false") { //  || vs == "") {
                    return false;
                }
                throw vs + " cannot be cast to bool! only true, false illegal!";
            }
            if (type == TYPE_CHAR) {
                if (vs.size() != 1) {
                    throw vs + " size != 1 ! cannot be cast to char!";
                }
                return vs.at(0);
            }
            if (type == TYPE_BYTE || type == TYPE_SHORT || type == TYPE_INT) {
                return std::stoi(vs);
            }
            if (type == TYPE_LONG) {
                return std::stol(vs);
            }
            if (type == TYPE_LONG_LONG) {
                return std::stoll(vs);
            }
            if (type == TYPE_FLOAT) {
                return std::stof(vs);
            }
            if (type == TYPE_DOUBLE) {
                return std::stod(vs);
            }
            if (type == TYPE_STRING) {
                return vs;
            }

            return json_2_obj(vs, type);
        }

        if (j.is_object()) {
            json type = j["type"];
            if (type.is_null() || type.empty()) {
                return j;
            }

            auto value = j["value"];
            if (type == TYPE_BOOL) {
                return value.get<bool>();
            }
            if (type == TYPE_CHAR) {
                return value.get<char>();
            }
            if (type == TYPE_BYTE) {
                return value.get<std::byte>();
            }
            if (type == TYPE_SHORT) {
                return value.get<short>();
            }
            if (type == TYPE_INT) {
                return value.get<int>();
            }
            if (type == TYPE_LONG) {
                return value.get<long>();
            }
            if (type == TYPE_LONG_LONG) {
                return value.get<long long>();
            }
            if (type == TYPE_FLOAT) {
                return value.get<float>();
            }
            if (type == TYPE_DOUBLE) {
                return value.get<double>();
            }
            if (type == TYPE_STRING) {
                return value.get<std::string>();
            }
            // if (type == "any" || type == "std::any") {
            //     return j;
            // }

            std::string type_s = type;
            int l = type_s.size();
            if (l > 2 && type_s.substr( l - 2, l) == "[]") {
                if (type == TYPE_STRING_ARR) {
                    auto vec = value.get<std::vector<std::string>>();
                    std::string arr[vec.size()];
                    for (int i = 0; i < vec.size(); ++i) {
                        arr[i] = vec.at(i);
                    }
                    return *arr;
                }
            }

            return json_2_obj(value.dump(), type);
        }

        if (j.is_array()) {
            std::vector<std::any> vec;
            for (int i = 0; i < j.size(); ++i) {
                auto arg = j[i];
                vec.push_back(json_to_any(arg));
            }
            return vec;
        }

        return static_cast<std::any>(j);
    }

    static std::map<std::string, void*> INSTANCE_MAP; // = {
    //     {TYPE_BOOL, false},
    //     {TYPE_CHAR, ''},
    //     {TYPE_SHORT, static_cast<short>(1)},
    //     {TYPE_INT, 1},
    //     {TYPE_LONG, 0L},
    //     {TYPE_LONG, 0L},
    //     {TYPE_LONG, 0L},
    //     {TYPE_LONG, 0L}
    // };
    // INSTANCE_MAP[TYPE_BOOL] = false;

    static void init() {
        // TYPE_MAP[TYPE_INT] = typeid(0);

    }


    template <typename Func>
    struct function_traits;

    template <typename Ret, typename... Args>
    struct function_traits<std::function<Ret(Args...)>> {
        using return_type = Ret;
        using argument_types = std::tuple<Args...>;
    };

    static nlohmann::json list_json(nlohmann::json j) {
        nlohmann::json result;

        try {
            json method = j["method"];
            std::string mtd = method.empty() ? "" : method.get<std::string>();

            json package = j["package"];
            std::string pkg = package.empty() ? "" : package.get<std::string>();

            json clazz = j["class"];
            std::string cls = clazz.empty() ? "" : clazz.get<std::string>();

            int packageTotal = 0;
            int classTotal = 0;
            int methodTotal = 0;

            json packageList;

            for (const auto& kv : FUNC_MAP) {
                auto key = kv.first;
                auto value = kv.second;

                auto ind = key.find_last_of('.');
                std::string pkg2 = "";
                std::string cls2 = "";
                std::string mtd2 = "";
                while (ind != std::string::npos && ind >= 0) {
                    if (mtd2.empty()) {
                        mtd2 = key.substr(ind + 1);
                    } else {
                        auto last = key.substr(ind + 1);
                        auto first = cls2.empty() ? last.at(0) : '0';
                        if (first >= 'A' && first <= 'Z') {
                            cls2 = last;
                        }
                        else {
                            pkg2 += (pkg2.empty() ? "" : ".") + last;
                        }
                    }

                    key = key.substr(0, ind);
                    ind = key.find_last_of('.');
                }

                if (mtd2.empty()) {
                    mtd2 = key;
                }

                if ((pkg2 != pkg && ! pkg.empty()) || (cls2 != cls && ! cls.empty()) || (mtd2 != mtd && ! mtd.empty())) {
                    continue;
                }

                std::string path2 = "";
                if (! cls.empty()) {
                    path2 = cls + "." + path2;
                }
                if (! pkg.empty()) {
                    path2 = pkg + "." + path2;
                }

                auto it = FUNC_MAP.find(path2);
                if (it == FUNC_MAP.end()) {
                    it = FUNC_MAP.find(mtd2);
                }

                if (it == FUNC_MAP.end()) {
                    continue;
                }

                auto func = it->second;
                if (func == nullptr) {
                    continue;
                }

                using traits = function_traits<decltype(func)>;
                // std::cout << "Argument types: ";
                // std::apply( { ((std::cout << typeid(args).name() << " "), ...); }, traits::argument_types{});
                // std::cout << std::endl;

                json mtdObj;
                mtdObj["name"] = mtd2;
                // mtdObj["parameterTypeList"] = parameterTypeList;
                // mtdObj["genericParameterTypeList"] = genericParameterTypeList;
                mtdObj["returnType"] = typeid(traits::return_type).name();
                mtdObj["genericReturnType"] = typeid(traits::return_type).name();
                // mtdObj["static"] = is_static;
                // mtdObj["exceptionTypeList"] = exceptionTypeList;
                // mtdObj["genericExceptionTypeList"] = genericExceptionTypeList;
                // mtdObj["parameterDefaultValueList"] = parameterDefaultValueList;


                json pkgObj;
                int pkgInd = -1;
                for (int i = 0; i < packageList.size(); ++i) {
                    json po = packageList[i];
                    if (po["package"] == pkg2) {
                        pkgObj = po;
                        pkgInd = i;
                        break;
                    }
                }

                if (pkgObj.empty()) {
                    pkgObj["package"] = pkg2;
                    pkgObj["classTotal"] = 1;

                    packageList.push_back(pkgObj);
                    packageTotal ++;
                } else {
                    auto t = pkgObj["classTotal"] ;
                    pkgObj["classTotal"] = t.get<int>() + 1;
                }

                if (packageList.empty()) {
                    packageList.push_back(pkgObj);
                }

                json classList = pkgObj["classList"];

                json clsObj;
                int clsInd = -1;
                for (int i = 0; i < classList.size(); ++i) {
                    json co = classList[i];
                    if (co["class"] == cls2) {
                        clsObj = co;
                        clsInd = i;
                        break;
                    }
                }

                if (clsObj.empty()) {
                    // auto clsConf = json_2_obj("", pkg2.empty() ? cls2 : pkg2 + "." + cls2);
                    // if (clsConf == nullptr) {
                    //     clsConf = json_2_obj("", cls2);
                    // }

                    clsObj["class"] = cls2;
                    // 父类 clsObj["type"] = typeid(clsConf).name();
                    clsObj["methodTotal"] = 1;

                    classList.push_back(clsObj);
                    classTotal ++;
                } else {
                    auto t = clsObj["methodTotal"] ;
                    pkgObj["methodTotal"] = t.get<int>() + 1;
                }

                if (classList.empty()) {
                    classList.push_back(clsObj);
                }

                json methodList = clsObj["methodList"];
                methodList.push_back(mtdObj);
                methodTotal ++;

                clsObj["methodList"] = methodList;

                if (clsInd < 0) {
                    clsInd = static_cast<int>(classList.size()) - 1;
                }
                classList[clsInd] = clsObj;
                pkgObj["classList"] = classList;

                if (pkgInd < 0) {
                    pkgInd = static_cast<int>(packageList.size()) - 1;
                }
                packageList[pkgInd] = pkgObj;
            }

            result["code"] = 200;
            result["msg"] = "success";
            result["packageTotal"] = packageTotal;
            result["classTotal"] = classTotal;
            result["methodTotal"] = methodTotal;
            result["packageList"] = packageList;
        } catch (const std::exception& e) {
            result["code"] = 500;
            result["msg"] = e.what();
        }

        return result;
    }

    static nlohmann::json invoke_json(nlohmann::json j) {
        nlohmann::json result;

        try {
            json method = j["method"];
            std::string mtd = method.empty() ? "" : method.get<std::string>();
            if (mtd.empty()) {
                throw "method cannot be empty! should be a string!";
            }

            json is_static = j["static"];
            bool is_sttc = is_static.empty() ? false : is_static.get<bool>();

            json package = j["package"];
            std::string pkg = package.empty() ? "" : package.get<std::string>();

            json clazz = j["class"];
            std::string cls = clazz.empty() ? "" : clazz.get<std::string>();

            std::string path = mtd;
            if (! cls.empty()) {
                path = cls + "." + path;
            }
            if (! pkg.empty()) {
                path = pkg + "." + path;
            }

            nlohmann::json args_ = j["args"];
            if (args_.empty()) {
                args_ = j["methodArgs"];
            }

            std::vector<std::any> args;
            json methodArgs;

            for (int i = 0; i < args_.size(); ++i) {
                auto arg = args_.at(i);
                std::any a = json_to_any(arg);
                // std::any a = static_cast<std::any>(arg);
                args.push_back(a);

                json ma;
                try {
                    auto type_cs = typeid(a).name();  // TYPE.name();
                    std::string type(type_cs);
                    if (type_cs == 0 || type.empty()) {
                        ma["type"] = arg.type_name();
                    } else {
                        ma["type"] = type;
                    }
                } catch (const std::exception& e) {
                    std::cout << "invoke_json  try { \n auto type_cs = typeid(a).name();... \n } catch (const std::exception& e) = " << e.what() << " >> ma[\"type\"] = arg.type_name();" << std::endl;
                    ma["type"] = arg.type_name();
                }

                try {
                    ma["value"] = any_to_json(a);
                } catch (const std::exception& e) {
                    std::cout << "invoke_json  try { \n ma[\"value\"] = any_to_json(a); \n } catch (const std::exception& e) = " << e.what() << " >> ma[\"value\"] = arg;" << std::endl;
                    ma["value"] = arg;
                }

                methodArgs.push_back(ma);
            }

            std::any ret = invoke(path, args);

            // auto TYPE = typeid(ret);
            auto type_cs = typeid(ret).name();  // TYPE.name();
            std::string type(type_cs);

            result["code"] = 200;
            result["msg"] = "success";
            if (! type.empty()) {
                result["type"] = type;  // type_cs;
            }

            result["return"] = any_to_json(ret);

            // if (ret.has_value()) {
            //     if (type_cs == TYPE_BOOL) {
            //         result["return"] = std::__convert_to_bool<>(ret); //  static_cast<int>(ret);
            //     }
            //     else if (type_cs == TYPE_INT) {
            //         result["return"] = std::to_integer(ret); //  static_cast<int>(ret);
            //     }
            //     else if (type_cs == TYPE_STR) {
            //         result["return"] = std::to_string(ret); //  static_cast<int>(ret);
            //     }
            // }


            result["methodArgs"] = methodArgs; // any_to_json(args);
        } catch (const std::exception& e) {
            result["code"] = 500;
            result["msg"] = e.what();
        }

        return result;
    }

        // 处理请求并生成响应
    inline void handle_request(int client_socket) {
        char buffer[1024];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            std::string request(buffer, bytes_received);

            // 简单解析 HTTP 请求
            std::istringstream request_stream(request);
            std::string method, path, http_version;
            request_stream >> method >> path >> http_version;

            // 解析 JSON 数据（假设数据在请求体中）
            std::string json_data;
            // std::getline(request_stream, json_data); // 跳过空行
            // std::getline(request_stream, json_data);

            bool first = true;
            bool start = false;
            std::string host = "";
            int len = static_cast<int>(strlen("Origin:"));

            while (! request_stream.eof()) {
                std::string line;
                std::getline(request_stream, line); // 跳过空行
                std::string pre = line.length() < len ? "" : line.substr(0, len);
                if (host.empty() && (pre == "Origin:" || pre == "origin:")) {
                    host = line.substr(len + 1);
                }

                if (start) {
                    json_data += "\n" + line;
                } else if (! first) {
                    line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
                    if (line.empty()) {
                        start = true;
                    }
                    else if (line.substr(0, 1) == "{") {
                        start = true;
                        json_data += line;
                    }
                }

                first = false;
            }

            // 处理数据并生成响应 JSON
            std::string response_json = R"({
                "code": 200,
                "msg": "success"
            })";

            bool isOpt = method == "options" || method == "OPTIONS";
            bool isPost = method == "post" || method == "POST";

            if (isPost) {
                if (path == "/method/invoke") {
                    json j = json::parse(json_data);
                    json result = invoke_json(j);
                    response_json = result.dump();
                }
                else if (path == "/method/list") {
                    json j = json::parse(json_data);
                    json result = list_json(j);
                    response_json = result.dump();
                }
                else {
                    response_json = R"({
                    "code": 404,
                    "msg": "Only support POST /method/invoke and POST /method/list ！"
                })";
                }
            }
            else if (! isOpt) {
                response_json = R"({
                    "code": 400,
                    "msg": "Only support HTTP POST Method！"
                })";
            }

            // 构建 HTTP 响应
            std::ostringstream response;
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: application/json\r\n";
            response << "Access-Control-Allow-Origin:" + host + "\n";
            response << "Access-Control-Allow-Credentials: true\r\n";
            response << "Access-Control-Allow-Headers: content-type\r\n";
            response << "Access-Control-Request-Method: POST\r\n";
            response << "Content-Length: " << response_json.size() << "\r\n";
            response << "\r\n";
            response << response_json;

            // 发送响应
            send(client_socket, response.str().c_str(), response.str().size(), 0);
        }
        close(client_socket);
    }

    static int start(int port) { // C++ 不支持重载方法
        port = port <= 0 ? 8084 : port;

        int server_socket = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
        listen(server_socket, SOMAXCONN);

        std::cout << "Server is running on port " << port << "..." << std::endl;

        while (true) {
            int client_socket = accept(server_socket, nullptr, nullptr);
            handle_request(client_socket);
        }

        close(server_socket);
        return 0;
    }


}
