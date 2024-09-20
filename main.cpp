#include "unitauto/method_util.hpp"
// #include "unitauto/server.hpp"
#include <iostream>

#include "unitauto/test/test_util.hpp"

using json = unitauto::json;

// 普通函数 Demo
int add(int a, int b) {
    return a + b;
}

double divide(double a, double b) {
    return a / b;
}

void print(const std::string &message) {
    std::cout << message << std::endl;
}

// 类和方法(成员函数) Demo

struct User {
    int id;
    int sex;
    std::string name;
    std::time_t date;

    long getId() {
        return id;
    }

    void setId(long id_) {
        id = id_;
    }

    std::string getName() {
        return name;
    }

    void setName(std::string name_) {
        name = name_;
    }

    std::time_t getDate() {
        return date;
    }

    void setDate(std::time_t date_) {
        date = date_;
    }

    // TODO FIXME 对 char 等部分类型无效
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(User, id, sex, name, date)

    bool is_male()
    {
        return sex == 0;
    }
};

// void to_json(json& j, const User& p) {
//     j = json{{"id", p.id}, {"sex", p.sex}, {"name", p.name}, {"date", p.date}};
// }
//
// void from_json(const json& j, User& p) {
//     j.at("id").get_to(p.id);
//     j.at("sex").get_to(p.sex);
//     j.at("name").get_to(p.name);
//     j.at("date").get_to(p.date);
// }

class Moment {
public:
    long id;
    long userId;
    std::string content;

    Moment() {
    }

    Moment(long id) {
      setId(id);
    }

    void setId(long id) {
        std::cout << "setId: " << id << std::endl;
        this->id = id;
    }

    long getId() {
        return this->id;
    }

    void setUserId(long userId) {
        this->userId = userId;
    }

    long getUserId() {
        return this->userId;
    }

    void setContent(std::string content) {
        std::cout << "setContent: " << content << std::endl;
        this->content = content;
    }

    std::string getContent() {
        return this->content;
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Moment, id, userId, content)
};

Moment newMoment(long id) {
    return Moment(id);
}

User newUser(long id, std::string name) {
    User u = User();
    u.id = id;
    u.setName(name);
    return u;
}



// 自定义类型示例
struct Person {
    std::string name;
    int age;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Person, name, age)
};


 int test() {
    // 示例值
    std::any ret = Person{"Alice", 30};
    auto type = typeid(ret).name();

    unitauto::add_struct<Person>("Person");

    // 执行函数
    try {
        //显式转换字符串字面量为 std::string
        unitauto::invoke("print", {std::string("Hello, UnitAuto C++ !")});

        auto ret = unitauto::invoke("add", {1, 2});
        std::cout << "invoke(\"add\", {1, 2}) = " << std::any_cast<int>(ret) << std::endl;

        auto divideRet = unitauto::invoke("divide", {5.6, static_cast<double>(3)});
        std::cout << "invoke(\"divide\", {5.6, 3}) = " << std::any_cast<double>(divideRet) << std::endl;

        auto moment2Ret = unitauto::invoke("main.newMoment", {12L});
        Moment moment2 = std::any_cast<Moment>(moment2Ret);
        std::cout << "invoke(\"main.newMoment\", {12}).id = " << moment2.id << std::endl;
        json j;
        j["type"] = "main.Moment";
        unitauto::invoke_method(j, "main.Moment.setId", {static_cast<long>(123)});

        auto getIdRet = unitauto::invoke_method(j, "main.Moment.getId", {});
        std::cout << "invoke(\"main.Moment.getId\", {}) = " << std::any_cast<long>(getIdRet) << std::endl;

        User user = User();
        auto userRet = unitauto::invoke("main.User.setId",{static_cast<long>(225)});
        std::cout << "invoke(\"main.Moment.getId\", {}) = " << std::any_cast<long>(getIdRet) << std::endl;

        std::vector<std::any> v;
        v.emplace_back(3.08);
        v.emplace_back(0.5);

        auto dr = unitauto::invoke("unitauto.test.divide", v);
        std::cout << "invoke(\"unitauto.test.divide\", {3.08, 0.5}) = " << std::any_cast<double>(dr) << std::endl;
    } catch (const std::exception e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    std::string str = R"({"id":1, "sex":1, "name":"John Doe", "date":1705293785163})";
    json j = json::parse(str);

    User u = j.get<User>();
    void* obj;
    try {
        obj = unitauto::json_2_obj(j, "User");
    } catch (const std::exception& e) {
        std::cerr << "json 2 obj failed:" << e.what() << std::endl;
    }

    User* userPtr = static_cast<User*>(obj);
    if (userPtr) {
        std::cout << "\nUser: {" << std::endl;
        std::cout << "  id: " << userPtr->id << std::endl;
        std::cout << "  sex: " << userPtr->sex << std::endl;
        std::cout << "  name: " << userPtr->name << std::endl;
        std::cout << "  date: " << userPtr->date << std::endl;
        std::cout << "}" << userPtr->id << std::endl;

        // malloc: *** error for object 0x16cf96110: pointer being freed was not allocated unitauto::del_obj<User>(obj);
    } else {
        std::cerr << "Type match error, have u added type with add_type?" << std::endl;
    }


    // json result;
    // result["code"] = 200;
    // result["msg"] = "success";
    // result["type"] = type;
    // result["return"] = unitauto::any_to_json(ret);
    // result["methodArgs"] = json::array({{{"type", "long"}, {"value", 1}}, {{"type", "long"}, {"value", 2}}});

    // std::cout << "\n\ntest return " << result.dump(4) << std::endl;

    return 0;
}

static int compare(User u, User u2) {
     if (u.id < u2.id) {
         return -1;
     }

     if (u.id > u2.id) {
         return 1;
     }

     return 0;
 }


int main() {
    unitauto::DEFAULT_MODULE_PATH = "unitauto::"; // TODO 改为你项目的默认包名

    UNITAUTO_ADD_FUNC(Moment, &Moment::getId, &Moment::setContent, &Moment::getUserId);

    // unitauto::add_struct<Moment>("Moment");
    // Moment ins = Moment();
    // auto tup = std::make_tuple(&Moment::getId, &Moment::setContent);
    // // std::vector<void*> arr = {&Moment::getId, &Moment::setContent};
    // const size_t count = std::tuple_size_v<decltype(tup)>;
    //  if (count > 0) {
    //      auto tup2 = std::make_tuple(&Moment::getId, &Moment::setContent);
    //      unitauto::add_func("Moment.getId", ins, std::get<0>(tup2));
    //  }
    //  if (count > 1) {
    //      auto tup2 = std::make_tuple(&Moment::getId, &Moment::setContent);
    //      unitauto::add_func("Moment.setContent", ins, std::get<1>(tup2));
    //  }
    //  if (count > 2) {
    //      auto tup2 = std::make_tuple(&Moment::getId, &Moment::setContent, &Moment::getUserId);
    //      unitauto::add_func("Moment.setContent", ins, std::get<2>(tup2));
    //  }

    // for (int i = 0; i < count; ++i) {
    //     // const size_t ind = i;
    //     if (i == 0) {
    //         unitauto::add_func("Moment.getId", ins, std::get<0>(tup));
    //     }
    //     else if (i == 1) {
    //         unitauto::add_func("Moment.setContent", ins, std::get<1>(tup));
    //     }
    //     else if (i == 2) {
    //         unitauto::add_func("Moment.getId", ins, std::get<2>(tup));
    //     }
    // }

    // // 必须先注册类型
    // // unitauto::add_type<Moment>("main.Moment");
    // unitauto::add_struct<Moment>("main.Moment", [](json &j) -> Moment {
    //     std::cout << "\ncallback Moment: {" << std::endl;
    //     Moment ins = unitauto::INSTANCE_GETTER<Moment>(j);
    //     unitauto::add_func("main.Moment.getId", ins, &Moment::getId);
    //     unitauto::add_func("main.Moment.setId", ins, &Moment::setId);
    //     unitauto::add_func("main.Moment.setContent", ins, &Moment::setContent);
    //     unitauto::add_func("main.Moment.getContent", ins, &Moment::getContent);
    //     std::cout << "\ncallback return ins;" << std::endl;
    //     return ins;
    // // }, [](std::any val) -> json {
    // //     auto j = std::any_cast<Moment>(val);
    // //     return j;
    // });

    unitauto::add_struct<User>("User");
    // unitauto::add_struct<unitauto::test::TestUtil>("unitauto.test.TestUtil");
    UNITAUTO_ADD_FUNC(unitauto::test::TestUtil, &unitauto::test::TestUtil::divide);

    // 注册函数
    unitauto::add_func("print", std::function<void(const std::string &)>(print));
    unitauto::add_func("add", std::function<int(int, int)>(add));
    unitauto::add_func("divide", std::function<double(double, double)>(divide));
    unitauto::add_func("main.newMoment", std::function<Moment(long)>(newMoment));
    unitauto::add_func("unitauto.test.divide", std::function<double(double,double)>(unitauto::test::divide));
    unitauto::add_func("unitauto.test.contains", std::function<bool(long[],long)>(unitauto::test::contains));
    unitauto::add_func("unitauto.test.index", std::function<int(std::string[],std::string)>(unitauto::test::index));
    unitauto::add_func("unitauto.test.is_contain", std::function<bool(std::vector<int>,int)>(unitauto::test::is_contain));
    unitauto::add_func("unitauto.test.index_of", std::function<int(std::vector<std::string>,std::string)>(unitauto::test::index_of));
    unitauto::add_func("main.newUser", std::function<User(long, std::string)>(newUser));

    // 注册方法(成员函数)
    User user = User();
    user.setId(1);
    user.name = "Test User";
    user.date = time(nullptr);

    unitauto::add_func("unitauto.test.User.is_male", user, &User::is_male);
    unitauto::add_func("main.User.setId", user, &User::setId);
    unitauto::add_func("main.User.getId", user, &User::getId);
    unitauto::add_func("main.User.setName", &user, &User::setName);
    unitauto::add_func("main.User.getName", &user, &User::getName);
    unitauto::add_func("main.User.setDate", (User *) nullptr, &User::setDate);
    unitauto::add_func("main.User.getDate", User(), &User::getDate);

    // unitauto::add_func("unitauto.test.TestUtil.divide", (unitauto::test::TestUtil *) nullptr, &unitauto::test::TestUtil::divide);

    test();
    unitauto::add_func("main.compare", std::function<int(User,User)>(compare));

    unitauto::start(8085);

    return 0;
}

