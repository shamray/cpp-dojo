#include <iostream>
#include <future>
#include <cassert>

#include "lock_based_queue.hpp"
#include "lock_based_stack.hpp"

void test_stack() {
    std::cout << "LOCK-BASED STACK\n";
    dojo::lock_based::stack<int> my_stack;

    auto fut = std::async([&my_stack]{ my_stack.push(2014); });
    auto fut1 = std::async([&my_stack]{ my_stack.push(2020); });
    auto fut2 = std::async([&my_stack]{ my_stack.push(2023); });

    auto fut3 = std::async([&my_stack]{ return my_stack.pop(); });
    auto fut4 = std::async([&my_stack]{ return my_stack.pop(); });
    auto fut5 = std::async([&my_stack]{ return my_stack.pop(); });

    fut.get(), fut1.get(), fut2.get();

    std::cout << fut3.get().value() << '\n';
    std::cout << fut4.get().value() << '\n';
    std::cout << fut5.get().value() << '\n';
}

void test_queue_coarse_grained() {
    std::cout << "LOCK-BASED QUEUE\n";

    using sp = dojo::lock_based::synchronization_policy;
    dojo::lock_based::queue<int, sp::coarse_grained> q;
    q.push(1998);
    q.push(2003);

    std::cout << *q.pop() << '\n';
    std::cout << *q.pop() << '\n';
    q.push(2011);
    q.push(2014);
    std::cout << *q.pop() << '\n';
    q.push(2017);
    q.push(2020);
    std::cout << *q.pop() << '\n';
    std::cout << *q.pop() << '\n';
    std::cout << *q.pop() << '\n';

    std::jthread push_thread{[&q](){
        using namespace std::chrono_literals;

        std::this_thread::sleep_for(3s);
        q.push(777);
    }};

    std::cout << "waiting for value...\n";
    std::cout << *q.pop() << '\n';
}

void test_queue_fine_grained() {
    std::cout << "LOCK-BASED QUEUE\n";

    using sp = dojo::lock_based::synchronization_policy;
    dojo::lock_based::queue<int, sp::fine_grained> q;
    q.push(1998);
    q.push(2003);

    std::cout << *q.pop() << '\n';
    std::cout << *q.pop() << '\n';
}

int main() {
    test_queue_fine_grained();
    test_stack();
    test_queue_coarse_grained();
    std::cout << "OK\n";
    return 0;
}