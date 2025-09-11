#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>

int sum;
std::mutex mtx;
using namespace std::chrono;

void worker(const int& loopCount);

int main()
{
	for (int num_threads = 1; num_threads <= 16; num_threads *= 2) {
		auto start = high_resolution_clock::now();
		std::vector<std::thread> threads;
		for (int i = 0; i < num_threads; ++i)
			threads.emplace_back(worker, 10000000 / num_threads);
		for (auto& t : threads)
			t.join();
		auto end = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(end - start);
		std::cout << num_threads << "multi: " << duration.count() << "ms" << std::endl;
	}
	std::cout << "sum = " << sum << std::endl;

	sum = 0;
	auto start = high_resolution_clock::now();
	for (int i = 0; i < 50000000; ++i)
		sum = sum + 2;
	auto end = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(end - start);
	std::cout << "single:" << duration.count() << "ms" << std::endl;

	std::cout << "sum = " << sum << std::endl;

}

void worker(const int& loopCount)
{
	mtx.lock();
	for (int i = 0; i < loopCount; ++i)
		sum = sum + 2;
	mtx.unlock();
}