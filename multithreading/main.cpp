#include <iostream>
#include <deque>
#include <stack>
#include <queue>
#include <optional>
#include <mutex>
#include <future>
#include <cassert>

namespace dojo::lock_based {
    template<typename T, template <typename, typename> class Cont = std::deque>
    class stack {
    public:
        stack() = default;

        stack(const stack&) = delete;
        stack& operator= (const stack&) = delete;

        void push(T val) {
            std::scoped_lock<std::mutex> lock(m_mutex);
            m_impl.push(std::move(val));
        }
        auto pop() -> std::optional<T> {
            std::scoped_lock<std::mutex> lock(m_mutex);

            if ( m_impl.empty() )
                return std::nullopt;

            auto val = m_impl.top();
            m_impl.pop();
            return val;
        }
    private:
        mutable std::mutex m_mutex;
        std::stack<T, Cont<T, std::allocator<T>>> m_impl;
    };

    enum class synchronization_policy {
        coarse_grained,
        fine_grained
    };

    template<typename T, synchronization_policy sp, template <typename, typename> class Cont = std::deque>
    class queue;


    template<typename T, template <typename, typename> class Cont>
    class queue<T, synchronization_policy::coarse_grained, Cont> {
    public:
        queue() = default;

        queue(const queue&) = delete;
        queue& operator= (const queue&) = delete;

        void push(T val) {
            std::scoped_lock<std::mutex> lock(m_mutex);
            m_impl.push(std::move(val));
            m_value_pushed.notify_one();
        }

        auto pop() -> std::optional<T> {
            std::unique_lock<std::mutex> lock(m_mutex);

            m_value_pushed.wait(lock, [this](){ return !m_impl.empty(); });

            auto val = m_impl.front();
            m_impl.pop();
            return val;
        }

    private:
        mutable std::mutex m_mutex;
        mutable std::condition_variable m_value_pushed;
        std::queue<T, Cont<T, std::allocator<T>>> m_impl;
    };

    template<typename T>
    class queue<T, synchronization_policy::fine_grained> {
    public:
        queue() = default;

        void push(T val) {
            auto dummy_node = std::make_unique<node>(T{});
            auto new_tail = dummy_node.get();

            std::scoped_lock tail_lock{m_tail_mutex};

            m_tail->next = std::move(dummy_node);
            m_tail->value = val;
            m_tail = new_tail;

            m_value_pushed.notify_one();
        }

        auto pop() -> std::optional<T> {
            std::scoped_lock head_lock{m_head_mutex};

            if (std::unique_lock tail_lock{m_tail_mutex}; m_head.get() == m_tail)
                m_value_pushed.wait(tail_lock, [this](){ return m_head.get() != m_tail; });

            auto res = m_head->value;
            auto old_head = std::move(m_head);
            m_head = std::move(old_head->next);

            assert(m_head != nullptr);
            return res;
        }

    private:
        struct node {
            std::unique_ptr<node> next;
            mutable std::mutex mutex;
            T value;

            explicit node(T value): value{std::move(value)} {}
        };

        std::unique_ptr<node> m_head{std::make_unique<node>(T{})};
        node* m_tail{m_head.get()};
        mutable std::mutex m_head_mutex;
        mutable std::mutex m_tail_mutex;
        mutable std::condition_variable m_value_pushed;
    };
}

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