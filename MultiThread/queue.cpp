#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <numeric>
#include <array>

class NODE {
public:
	NODE(int x) : next(nullptr), value(x) {}

	int value;
	NODE* next;
};

class CE_QUEUE {
public:
	CE_QUEUE()
	{
		head = new NODE(std::numeric_limits<int>::min());
		tail = new NODE(std::numeric_limits<int>::max());
		head->next = tail;
	}

	~CE_QUEUE()
	{
		clear();
		delete head;
		delete tail;
	}

	void Enqueue(int x)
	{
		enqLock.lock();
		NODE* newNode = new NODE(x);
		tail->next = newNode;
		tail = newNode;
		enqLock.unlock();
	}
	
	int Dequeue()
	{
		int result;
		deqLock.lock();
		if (head->next == nullptr) {
			deqLock.unlock();
			return -1;
		}
		result = head->next->value;
		head = head->next;
		deqLock.unlock();
		return result;
	}

	void clear()
	{
		NODE* curr = head->next;
		while (curr != tail) {
			NODE* temp = curr;
			curr = curr->next;
			delete temp;
		}
		head->next = tail;
	}

	void print20()
	{
		auto curr = head->next;
		for (int i = 0; i < 20 && curr != tail; ++i) {
			std::cout << curr->value << ", ";
			curr = curr->next;
		}
		std::cout << std::endl;
	}
private:
	std::mutex enqLock;
	std::mutex deqLock;
	NODE* head;
	NODE* tail;
};

CE_QUEUE my_queue;

const int NUM_TEST = 400'0000;
const int MAX_THREADS = 16;
int num_threads = 0;

class HISTORY {
public:
	int op;
	int i_value;
	bool o_value;
	HISTORY(int o, int i, bool re) : op(o), i_value(i), o_value(re) {}
};

std::array<std::vector<HISTORY>, MAX_THREADS> history;

void benchmark(const int num_thread)
{
	int key = 0;
	for (int i = 0; i < NUM_TEST / num_thread; i++) {
		if ((i < 32) || (rand() % 2 == 0))
			my_queue.Enqueue(key++);
		else
			my_queue.Dequeue();
	}
}

int main()
{
	using namespace std::chrono;

	std::cout << "Benchmarking\n";
	int key = 0;
	for (num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		my_queue.clear();
		std::vector<std::thread> threads;
		auto start = high_resolution_clock::now();
		for (int i = 0; i < num_threads; ++i)
			threads.emplace_back(benchmark, num_threads);
		for (auto& th : threads)
			th.join();
		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(stop - start);
		std::cout << "Threads: " << num_threads << ", Duration: " << duration.count() << " ms.\n";
		std::cout << "Queue: "; my_queue.print20();
	}
}