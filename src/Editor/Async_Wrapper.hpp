#pragma once
#include <future>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <functional>
#include <string>

//Barrowed from: https://stackoverflow.com/a/39563127/21271522
template<class Function, class... Args>
void async_wrapper(Function&& f, Args&&... args, std::future<void>& future,
                   std::future<void>&& is_valid, std::promise<void>&& is_moved) {
    is_valid.wait(); // Wait until the return value of std::async is written to "future"
    auto our_future = std::move(future); // Move "future" to a local variable
    is_moved.set_value(); // Only now we can leave void_async in the main thread

    // This is also used by std::async so that member function pointers work transparently
    auto functor = std::bind(f, std::forward<Args>(args)...);
    functor();
}

template<class Function, class... Args> // This is what you call instead of std::async
void void_async(Function&& f, Args&&... args) {
    std::future<void> future; // This is for std::async return value
    // This is for our synchronization of moving "future" between threads
    std::promise<void> valid;
    std::promise<void> is_moved;
    auto valid_future = valid.get_future();
    auto moved_future = is_moved.get_future();

    // Here we pass "future" as a reference, so that async_wrapper
    // can later work with std::async's return value
    future = std::async(
        async_wrapper<Function, Args...>,
        std::forward<Function>(f), std::forward<Args>(args)...,
        std::ref(future), std::move(valid_future), std::move(is_moved)
    );
    valid.set_value(); // Unblock async_wrapper waiting for "future" to become valid
    moved_future.wait(); // Wait for "future" to actually be moved
}

template<class Function, class... Args>
std::string string_async(Function&& f, Args&&... args) {
    std::future<std::string> future_result; // This is for storing the result of the function
    std::promise<void> valid;
    std::promise<void> is_moved;
    auto valid_future = valid.get_future();
    auto moved_future = is_moved.get_future();

    // Here we pass "future_result" as a reference, so that async_wrapper
    // can later work with the function's return value
    future_result = std::async(
        async_wrapper<Function, Args...>,
        std::forward<Function>(f), std::forward<Args>(args)...,
        std::ref(future_result), std::move(valid_future), std::move(is_moved)
    );
    valid.set_value(); // Unblock async_wrapper waiting for "future_result" to become valid
    moved_future.wait(); // Wait for "future_result" to actually be moved

    // Retrieve the result from the future
    std::string result = future_result.get();
    return result;
}



struct ThreadPool {
    struct Task {
        virtual void Run() const = 0;
        virtual ~Task() {};
    };   

    template < typename task_, typename... args_ >
    struct RealTask : public Task {
        RealTask( task_&& task, args_&&... args ) : fun_( std::bind( std::forward<task_>(task), std::forward<args_>(args)... ) ) {}
        void Run() const override {
            fun_();
        }
    private:
        decltype( std::bind(std::declval<task_>(), std::declval<args_>()... ) ) fun_;
    };

    template < typename task_, typename... args_ >
    void AddTask( task_&& task, args_&&... args ) {
        auto lock = std::unique_lock<std::mutex>{mtx_};
        using FinalTask = RealTask<task_, args_... >;
        q_.push( std::unique_ptr<Task>( new FinalTask( std::forward<task_>(task), std::forward<args_>(args)... ) ) );
    }

    ThreadPool() {
        for( auto & t : pool_ )
            t = std::thread( [=] {
                while ( true ) {
                    std::unique_ptr<Task> task;
                    {
                        auto lock = std::unique_lock<std::mutex>{mtx_};
                        if ( q_.empty() && stop_ ) 
                            break;
                        if ( q_.empty() )
                            continue;
                        task = std::move(q_.front());
                        q_.pop();
                    }
                    if (task)
                        task->Run();
                }
            } );
    }
    ~ThreadPool() {
        {
            auto lock = std::unique_lock<std::mutex>{mtx_};
            stop_ = true;
        }
        for( auto & t : pool_ )
            t.join();
    }
private:
    std::queue<std::unique_ptr<Task>> q_;
    std::thread pool_[8]; 
    std::mutex mtx_;
    volatile bool stop_ {};
};
