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

std::atomic<int> maxLabel = 0;
std::atomic<int> label[MAX_THREADS] = { 0 };
std::atomic<bool> flags[MAX_THREADS] = { false, false };

void p_lock(const int thread_id);
void p_unlock(const int thread_id);
void worker(const int thread_id, const int loop_count);
void worker2(const int thread_id, const int loop_count);
void worker3(const int thread_id, const int loop_count);
void worker4(const int thread_id, const int loop_count);

int main()
{
	std::cout << "CAS" << std::endl;
	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		sum = 0;
		maxLabel = 0;
		std::vector<std::thread> threads;
		auto start = high_resolution_clock::now();
		for (int i = 0; i < num_threads; ++i)
			threads.emplace_back(worker3, i, 50000000 / num_threads);
		for (int i = 0; i < num_threads; ++i)
			threads[i].join();
		auto end = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(end - start);

		std::cout << num_threads << "multi: " << duration.count() << "ms" << std::endl;
		std::cout << "sum = " << sum << std::endl;
	}

	std::cout << "=================================" << std::endl;
	std::cout << "mutex" << std::endl;
	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		sum = 0;
		maxLabel = 0;
		std::vector<std::thread> threads1;
		auto start = high_resolution_clock::now();
		for (int i = 0; i < num_threads; ++i)
			threads1.emplace_back(worker2, i, 50000000 / num_threads);
		for (int i = 0; i < num_threads; ++i)
			threads1[i].join();
		auto end = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(end - start);

		std::cout << num_threads << "multi: " << duration.count() << "ms" << std::endl;
		std::cout << "sum = " << sum << std::endl;
	}

	std::cout << "=================================" << std::endl;
	std::cout << "Bakery" << std::endl;
	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		sum = 0;
		maxLabel = 0;
		std::vector<std::thread> threads2;
		auto start = high_resolution_clock::now();
		for (int i = 0; i < num_threads; ++i)
			threads2.emplace_back(worker, i, 50000000 / num_threads);
		for (int i = 0; i < num_threads; ++i)
			threads2[i].join();
		auto end = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(end - start);

		std::cout << num_threads << "multi: " << duration.count() << "ms" << std::endl;
		std::cout << "sum = " << sum << std::endl;
	}

	std::cout << "=================================" << std::endl;
	std::cout << "no Lock" << std::endl;
	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		sum = 0;
		maxLabel = 0;
		std::vector<std::thread> threads3;
		auto start = high_resolution_clock::now();
		for (int i = 0; i < num_threads; ++i)
			threads3.emplace_back(worker4, i, 50000000 / num_threads);
		for (int i = 0; i < num_threads; ++i)
			threads3[i].join();
		auto end = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(end - start);

		std::cout << num_threads << "multi: " << duration.count() << "ms" << std::endl;
		std::cout << "sum = " << sum << std::endl;
	}

	std::cout << "=================================" << std::endl;
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
	for (int k = 0; k < MAX_THREADS; k++) {
		// maxLabel = std::max(maxLabel, label[k]);
		if (maxLabel < label[k])
			maxLabel.store(label[k]);
	}
	// label[thread_id] = maxLabel + 1;
	maxLabel += 1;
	label[thread_id].store(maxLabel);
	for (int k = 0; k < MAX_THREADS; k++)
		while ((flags[k] == true) && (label[k] < label[thread_id] || (label[k] == label[thread_id] && k < thread_id)));
}

void p_unlock(const int thread_id)
{
	flags[thread_id] = false;
}

std::atomic<bool> lockFlag(false);

bool CAS(std::atomic_bool* lock_flag, bool old_value, bool new_value)
{
	return std::atomic_compare_exchange_strong(lock_flag, &old_value, new_value);
}

void CAS_Lock()
{
	while(!CAS(&lockFlag, false, true));
}

void CAS_Unlock()
{
	lockFlag = false;
}

void worker3(const int thread_id, const int loop_count)
{
	for (int i = 0; i < loop_count; ++i) {
		CAS_Lock();
		sum = sum + 2;
		CAS_Unlock();
	}
}

void worker(const int thread_id, const int loop_count)
{
	for (int i = 0; i < loop_count; ++i) {
		p_lock(thread_id);
		sum = sum + 2;
		p_unlock(thread_id);
	}
}

void worker2(const int thread_id, const int loop_count)
{
	for (int i = 0; i < loop_count; ++i) {
		mtx.lock();
		sum = sum + 2;
		mtx.unlock();
	}
}

void worker4(const int thread_id, const int loop_count)
{
	for (int i = 0; i < loop_count; ++i) {
		sum = sum + 2;
	}
}