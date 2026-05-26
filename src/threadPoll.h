#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

class ThreadPool {
    public:
        explicit ThreadPool(size_t threadCount)
            : m_stop(false)
        {
            for (size_t i = 0; i < threadCount; ++i) {
                m_workers.emplace_back([this]() {
                    workerLoop();
                });
            }
        }
    
        ~ThreadPool() {
            shutdown();
        }
    
        template <typename F, typename... Args>
        auto submit(F&& f, Args&&... args) {
            using ReturnType = decltype(f(args...));
    
            auto task = std::make_shared<std::packaged_task<ReturnType()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );
    
            std::future<ReturnType> future = task->get_future();
    
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_stop)
                    throw std::runtime_error("submit on stopped ThreadPool");
    
                m_tasks.emplace([task]() {
                    try {
                        (*task)();
                    } catch (const std::exception& e) {
                        std::cerr << "[ThreadPool Exception] "
                                  << e.what() << std::endl;
                    } catch (...) {
                        std::cerr << "[ThreadPool Unknown Exception]" << std::endl;
                    }
                });
            }
    
            m_cond.notify_one();
            return future;
        }
    
        void shutdown() {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_stop) return;
                m_stop = true;
            }
    
            m_cond.notify_all();
    
            for (auto& t : m_workers)
                if (t.joinable()) t.join();
        }
    
    private:
        void workerLoop() {
            while (true) {
                std::function<void()> task;
    
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_cond.wait(lock, [this] {
                        return m_stop || !m_tasks.empty();
                    });
    
                    if (m_stop && m_tasks.empty())
                        return;
    
                    task = std::move(m_tasks.front());
                    m_tasks.pop();
                }
    
                task();
            }
        }
    
    private:
        std::vector<std::thread> m_workers;
        std::queue<std::function<void()>> m_tasks;
    
        std::mutex m_mutex;
        std::condition_variable m_cond;
        bool m_stop{false};
    };