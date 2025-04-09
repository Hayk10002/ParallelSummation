#include <iostream>
#include <random>
#include <chrono>
#include <atomic>
#include <thread>
#include <vector>
#include <format>
#include <span>
#include <mutex>
#include <numeric>

int generateRandomNumber(int min = 1, int max = 100) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(min, max);
    return dis(gen);
}

std::pair<std::chrono::nanoseconds, int> one_threaded(std::span<int> span)
{
    int sum = 0;
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < span.size(); ++i) sum += span[i];

    return {std::chrono::high_resolution_clock::now() - start, sum};
}

std::pair<std::chrono::nanoseconds, int> test_atomic(std::span<int> span, int chunk_size)
{
    std::atomic<int> curr_index = 0;
    std::atomic<int> sum = 0;

    int thread_count = std::jthread::hardware_concurrency();
    if (!thread_count) thread_count = 4;
    
    std::chrono::high_resolution_clock::time_point start;
    
    {
        std::vector<std::jthread> threads;
        threads.reserve(thread_count);

        start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&]() {
                for (int index = 0; (index = curr_index.fetch_add(chunk_size, std::memory_order::acq_rel)) < span.size();)                    
                    sum.fetch_add(std::accumulate(span.begin() + index, span.begin() + std::min(index + chunk_size, (int)span.size()), 0), std::memory_order::acq_rel);
            });
        }
    }

    return {std::chrono::high_resolution_clock::now() - start, sum.load(std::memory_order::relaxed)};
}

std::pair<std::chrono::nanoseconds, int> test_mutex(std::span<int> span, int chunk_size)
{
    static std::mutex ind_mutex;
    int curr_index = 0;

    static std::mutex sum_mutex;
    int sum = 0;

    int thread_count = std::jthread::hardware_concurrency();
    if (!thread_count) thread_count = 4;

    std::chrono::high_resolution_clock::time_point start;

    {
        std::vector<std::jthread> threads;
        threads.reserve(thread_count);

        start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&]() {
                for (int index = 0;;)
                {
                    std::unique_lock<std::mutex> lock(ind_mutex);
                    if ((index = (curr_index+=chunk_size) - chunk_size) >= span.size()) break;
                    lock.unlock();
                    int s = std::accumulate(span.begin() + index, span.begin() + std::min(index + chunk_size, (int)span.size()), 0);
                    std::lock_guard<std::mutex> sum_lock(sum_mutex);
                    sum += s;
                }                    
            });
        }
    }

    return {std::chrono::high_resolution_clock::now() - start, sum};
}

int main(int argc, char* argv[])
{
    int vec_size = 100000000;
    int chunk_size = 10000;
    if (argc > 1) vec_size = std::atoi(argv[1]);
    if (argc > 2) chunk_size = std::atoi(argv[2]);
    
    std::cout << std::format("Vector size: {}, Chunk size: {}\n", vec_size, chunk_size);

    std::vector<int> vec(vec_size);

    std::generate(vec.begin(), vec.end(), []() { return generateRandomNumber(-25, 25); });

    {
        auto [time, sum] = one_threaded(vec);
        std::cout << std::format("One_thread: Time: {:>7}, Sum: {:>7}\n", std::chrono::duration_cast<std::chrono::milliseconds>(time), sum);
    }

    {
        auto [time, sum] = test_atomic(vec, chunk_size);
        std::cout << std::format("Atomic    : Time: {:>7}, Sum: {:>7}\n", std::chrono::duration_cast<std::chrono::milliseconds>(time), sum);
    }

    {
        auto [time, sum] = test_mutex(vec, chunk_size);
        std::cout << std::format("Mutex     : Time: {:>7}, Sum: {:>7}\n", std::chrono::duration_cast<std::chrono::milliseconds>(time), sum);
    }

    std::cout << "--------------------------\n";

    return 0;
}
