#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <atomic>

class ThreadPool {
public:
    // 构造函数，启动 num_threads 个工作线程
    explicit ThreadPool(size_t num_threads)
        : stop(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                        });

                        if (this->stop && this->tasks.empty())
                            return;

                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    task(); // 执行任务
                }
            });
        }
    }

    // 向线程池提交一个任务，返回一个 future 以便获取返回值（如果有的话）
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>> {
        using return_type = typename std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            // 不允许在停止后添加新任务
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");

            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    // 析构函数，停止所有线程并等待它们完成
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers)
            worker.join();
    }

private:
    std::vector<std::thread> workers;          // 工作线程
    std::queue<std::function<void()>> tasks;   // 任务队列

    std::mutex queue_mutex;             // 互斥锁，保护任务队列
    std::condition_variable condition;  // 条件变量，用于线程同步
    bool stop;                          // 是否停止线程池
};

int threadPoll_test() {
    ThreadPool pool(10);
        for (int i = 0; i < 10; ++i)
        {
            pool.enqueue([i](){
                static int x = 0;
                std::cout << "i: " << i << std::endl;
                static std::mutex m;
                {
                    std::lock_guard<std::mutex> lock(m);
                    ++x;
                    
                    std::cout << "x: " << x << std::endl;
                    std::cout << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
            });
        }
        return 0;
}
