#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>

const int MAX_THREADS = 2;
volatile int sum = 0;
std::mutex mtx;
using namespace std::chrono;

std::atomic<int> victim = 0;
std::atomic<bool> flags[2] = { false, false };

void p_lock(const int thread_id);
void p_unlock(const int thread_id);
void worker(const int thread_id, const int loop_count);
void worker3(const int thread_id, const int loop_count);

int main()
{
	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		sum = 0;
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
	const int other = 1 - thread_id;
	flags[thread_id] = true;
	victim = thread_id;
	// std::atomic_thread_fence(std::memory_order_seq_cst);
	while ((true == flags[other]) && (victim == thread_id));
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

std::atomic<bool> lockFlag(false);

bool CAS(std::atomic_bool* lock_flag, bool old_value, bool new_value)
{
	return std::atomic_compare_exchange_strong(lock_flag, &old_value, new_value);
}

void CAS_Lock()
{
	while (CAS(&lockFlag, 0, 1));
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