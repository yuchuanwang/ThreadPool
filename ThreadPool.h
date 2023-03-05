// ThreadPool with C++ std::thread
// Author: Yuchuan Wang
// yuchuan.wang@gmail.com
// 

#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <queue>

// Function will be running inside thread pool
using ThreadTask = std::function<void()>;

class ThreadPool
{
public:
    // If threads_num is 0, it will use the same number of CPU cores
    // If tasks_num is -1, the number of tasks will be unlimited
    ThreadPool(int threads_num = 0, int tasks_num = -1)
    {
        if(threads_num == 0)
        {
            max_threads = std::thread::hardware_concurrency();
        }
        else
        {
            max_threads = threads_num;
        }
        max_tasks = tasks_num;
        is_running = false;
    }

    ~ThreadPool()
    {
        WaitForStop();
    }

    bool AddTask(ThreadTask task)
    {
        // Scope for lock
        {
            std::unique_lock<std::mutex> lock(tasks_guard);
            if(max_tasks == -1)
            {
                // Unlimited
                tasks.push(task);
            }
            else
            {
                if(tasks.size() >= max_tasks)
                {
                    return false;
                }
                else
                {
                    tasks.push(task);
                }
            }
        }

        // Notify thread
        tasks_event.notify_one();

        return true; 
    }

    bool Start()
    {
        if(is_running)
        {
            // Running already
            return false;
        }

        is_running = true;
        if(threads.empty())
        {
            CreateThreads();
        }

        return true;
    }

    void WaitForStop()
    {
        if(!is_running)
        {
            // I am not running
            return;
        }

        is_running = false; 
        tasks_event.notify_all();
        for(auto &t : threads)
        {
            // Wait for all threads to exit
            t.join();
        }
        threads.clear();
    }

private:
    void CreateThreads()
    {
        for(int i = 0; i < max_threads; i++)
        {
            threads.push_back(std::thread(&ThreadPool::ThreadRoutine, this));
        }
    }

    // Thread worker function
    // Take task from queue, and run it
    static void ThreadRoutine(ThreadPool* ptr)
    {
        if(ptr == nullptr)
        {
            return; 
        }

        while(ptr->is_running || !ptr->tasks.empty())
        {
            ThreadTask task; 
            // Scope for lock
            {
                // Get task to run
                std::unique_lock<std::mutex> lock(ptr->tasks_guard);
                while(ptr->tasks.empty())
                {
                    // Wait until task is ready
                    ptr->tasks_event.wait(lock);
                }
                
                // OK, now there is a task ready to run
                task = ptr->tasks.front();
                ptr->tasks.pop();
            }
            // Run it
            task();
        }
    }

private:
    // Max threads allowed
    int max_threads;
    // Max tasks inside queue
    int max_tasks; 
    // Vector of threads
    std::vector<std::thread> threads; 
    // Queue of tasks
    std::queue<ThreadTask> tasks; 
    // Flag of runnin status
    bool is_running;
    // Mutex to protect the tasks queue
    std::mutex tasks_guard; 
    // Condition of tasks event
    std::condition_variable tasks_event; 
};
