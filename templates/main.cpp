#include <print>
#include <variant>

namespace dojo
{
template <typename... T>
struct Visitor: T... {
    using T::operator()...;
};

#if (__cplusplus <= 201703L)
template <typename... T>
Visitor(T...) -> Visitor<T...>;
#endif

}

int main() {
    std::variant<int, float, std::string> intFloatString{"Hello"};

    dojo::Visitor visitor{
        [](const int& i) { std::println("int: {}", i); },
        [](const float& f) { std::println("float: {}", f); },
        [](const std::string& s) { std::println("string: {}", s); }};
    std::visit(visitor, intFloatString);
}
