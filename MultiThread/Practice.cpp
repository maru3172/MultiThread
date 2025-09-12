#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>

struct NUM {
	alignas(64) volatile int value;
};

const int MAX_THREADS = 16;
const int CACHE_LINE_SIZE_INT = 16;
volatile int sum = 0;
NUM array_sum[MAX_THREADS] = { 0 };
std::mutex mtx;
using namespace std::chrono;

void worker(const int& thread_id, const int& loopCount);

int main()
{
	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		sum = 0;
		std::vector<std::thread> threads;
		auto start = high_resolution_clock::now();
		for (int i = 0; i < num_threads; ++i)
			threads.emplace_back(worker, i, 50000000 / num_threads);
		for (int i = 0; i < num_threads; ++i) {
			threads[i].join();
			sum = sum + array_sum[i].value;
			array_sum[i].value = 0;
		}
		auto end = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(end - start);

		std::cout << num_threads << "multi: " << duration.count() << "ms" << std::endl;
		std::cout << "sum = " << sum << std::endl;
	}

	sum = 0;
	auto start = high_resolution_clock::now();
	for (int i = 0; i < 50000000; ++i)
		sum = sum + 2;
	auto end = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(end - start);
	std::cout << "single: " << duration.count() << "ms" << std::endl;

	std::cout << "sum = " << sum << std::endl;

}

void worker(const int& thread_id, const int& loopCount)
{
	for (int i = 0; i < loopCount; ++i)
		array_sum[thread_id].value = array_sum[thread_id].value + 2;
}