#include <iostream>
#include <thread>

volatile int* ptr = nullptr;
volatile bool done = false;

void update_ptr() {
	for (int i = 0; i < 25000000; ++i)
		*ptr = -(1 + *ptr);
	done = true;
}

void watch_ptr() {
	int error_count = 0;
	while (!done) {
		int v = *ptr;
		if ((v != 0) && (v != -1)){
			printf("%X ", v);
			error_count++;
		}
	}
	std::cout << "Error count = " << error_count << std::endl;
}

int main()
{
	int value[32];
	long long addr = reinterpret_cast<long long>(&value[31]);
	addr = addr - (addr % 64);
	addr = addr - 2;
	ptr = reinterpret_cast<int*>(addr);
	*ptr = 0;
	std::thread t1(watch_ptr);
	std::thread t2(update_ptr);
	t1.join();
	t2.join();
}