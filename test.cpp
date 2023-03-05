#include "ThreadPool.h"

int product_sell = 0;
void ProductCounter(std::mutex* task_protect)
{
    //std::this_thread::sleep_for(std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::microseconds(100));

    std::lock_guard<std::mutex> lock(*task_protect);
    std::cout <<"How many products sell: " << product_sell++ << std::endl;
}

int main()
{
    std::mutex protect_task;
    ThreadPool pool(0, -1);
    for(int i = 0; i < 100; i++)
    {
        pool.AddTask(std::bind(ProductCounter, &protect_task));
    }
    pool.Start();
    // Do more stuff...
    for(int i = 0; i < 50; i++)
    {
        pool.AddTask(std::bind(ProductCounter, &protect_task));
    }
    pool.WaitForStop();
    return 0;
}
