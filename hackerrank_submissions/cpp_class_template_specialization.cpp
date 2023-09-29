// https://www.hackerrank.com/challenges/cpp-class-template-specialization

#include <iostream>

enum class Fruit { apple, orange, pear };
enum class Color { red, green, orange };

template <typename T> struct Traits;


template <>
struct Traits<Fruit> {
    static auto name (int fruit) {
        switch(static_cast<Fruit>(fruit)) {
            case Fruit::apple: return "apple";
            case Fruit::orange: return "orange";
            case Fruit::pear: return "pear";
            default: return "unknown";
        }
    }
};

template <>
struct Traits<Color> {
    static auto name (int color) {
        switch(static_cast<Color>(color)) {
            case Color::red: return "red";
            case Color::green: return "green";
            case Color::orange: return "orange";
            default: return "unknown";
        }
    }
};



int main()
{
    int t = 0; std::cin >> t;

    for (int i=0; i!=t; ++i) {
        int index1; std::cin >> index1;
        int index2; std::cin >> index2;
        std::cout << Traits<Color>::name(index1) << ' ';
        std::cout << Traits<Fruit>::name(index2) << '\n';
    }
}
