#include <iostream>
#include <thread>
#include <mutex>

int sum;
std::mutex mtx;

void worker();

int main()
{
	std::thread t1{ worker };
	std::thread t2{ worker };
	t1.join();
	t2.join();

	std::cout << "sum = " << sum << std::endl;
}

void worker()
{
	mtx.lock();
	for (int i = 0; i < 25000000; ++i)
		sum = sum + 2;
	mtx.unlock();
}