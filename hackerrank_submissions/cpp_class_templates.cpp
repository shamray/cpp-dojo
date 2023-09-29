// https://www.hackerrank.com/challenges/c-class-templates/

#include <iostream>
#include <algorithm>

template <typename T>
class AddElementsBase {
public:
    explicit AddElementsBase(const T& element)
        : element_{element}
    {}

    auto add(const T& operand) const {
        return element_ + operand;
    }

private:
    T element_;
};

template <typename T>
class AddElements: public AddElementsBase<T> {
public:
    AddElements(const T& element): AddElementsBase<T>(element) {}
};

template <>
class AddElements<std::string>: public AddElementsBase<std::string> {
public:
    AddElements(const std::string& element): AddElementsBase<std::string>(element) {}

    auto concatenate(const std::string& operand) const {
        return add(operand);
    }
};

int main () {
    int n,i;
    std::cin >> n;
    for(i=0;i<n;i++) {
        std::string type;
        std::cin >> type;
        if(type=="float") {
            double element1,element2;
            std::cin >> element1 >> element2;
            AddElements<double> myfloat (element1);
            std::cout << myfloat.add(element2) << std::endl;
        }
        else if(type == "int") {
            int element1, element2;
            std::cin >> element1 >> element2;
            AddElements<int> myint (element1);
            std::cout << myint.add(element2) << std::endl;
        }
        else if(type == "string") {
            std::string element1, element2;
            std::cin >> element1 >> element2;
            AddElements<std::string> mystring (element1);
            std::cout << mystring.concatenate(element2) << std::endl;
        }
    }
    return 0;
}