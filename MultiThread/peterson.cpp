#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <algorithm>
#include <atomic>


const int MAX_THREADS = 8;
volatile int sum = 0;
std::mutex mtx;
using namespace std::chrono;

volatile int maxLabel = 0;
volatile int label[MAX_THREADS] = { 0 };
volatile bool flags[MAX_THREADS] = { false, false };

void p_lock(const int thread_id);
void p_unlock(const int thread_id);
void worker(const int thread_id, const int loop_count);

int main()
{
	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		sum = 0;
		std::vector<std::thread> threads;
		auto start = high_resolution_clock::now();
		for (int i = 0; i < num_threads; ++i)
			threads.emplace_back(worker, i, 50000000 / num_threads);
		for (int i = 0; i < num_threads; ++i)
			threads[i].join();
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

void p_lock(const int thread_id)
{
	flags[thread_id] = true;
	for (int k = 0; k < MAX_THREADS; k++)
		maxLabel = std::max(maxLabel, label[k]);
	label[thread_id] = maxLabel + 1;
	for (int k = 0; k < MAX_THREADS; k++)
		while ((flags[k] == true) && (label[k] < label[thread_id] || (label[k] == label[thread_id] && k < thread_id)));
}

void p_unlock(const int thread_id)
{
	flags[thread_id] = false;
}

void worker(const int thread_id, const int loop_count)
{
	for (int i = 0; i < loop_count; ++i) {
		p_lock(thread_id);
		sum = sum + 2;
		p_unlock(thread_id);
	}
}