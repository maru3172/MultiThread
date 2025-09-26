#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <numeric>

const int MAX_THREADS = 32;
using namespace std::chrono;

class NODE {
public:
	NODE(int x) : next(nullptr), value(x) {}
	int value;
	NODE* next;
};

class C_SET {
public:
	C_SET() {
		head = new NODE(std::numeric_limits<int>::min());
		tail = new NODE(std::numeric_limits<int>::max());
		head->next = tail;
	}

	~C_SET()
	{
		clear();
		delete head;
		delete tail;
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

	bool add(int x)
	{
		mtx.lock();
		auto prev = head;
		auto curr = prev->next;

		while (curr->value < x) {
			prev = curr;
			curr = curr->next;
		}

		if (curr->value == x) {
			mtx.unlock();
			return false;
		}

		auto newNode = new NODE(x);
		newNode->next = curr;
		prev->next = newNode;
		mtx.unlock();
		return true;
	}

	bool remove(int x)
	{
		mtx.lock();
		auto prev = head;
		auto curr = prev->next;

		while (curr->value < x) {
			prev = curr;
			curr = curr->next;
		}

		if (curr->value == x) {
			prev->next = curr->next;
			delete curr;
			mtx.unlock();
			return true;
		}

		mtx.unlock();
		return false;
	}

	bool contains(int x)
	{
		mtx.lock();
		auto prev = head;
		auto curr = prev->next;

		while (curr->value < x) {
			prev = curr;
			curr = curr->next;
		}

		if (curr->value == x) {
			mtx.unlock();
			return true;
		}
		mtx.unlock();
		return false;
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
	NODE* head, * tail;
	std::mutex mtx;
};

C_SET set;

void benchmark(const int num_threads)
{
	const int LOOP = 4000000 / num_threads;
	const int RANGE = 1000;

	for (int i = 0; i < LOOP; ++i) {
		int value = rand() % RANGE;
		int op = rand() % 3;
		if (op == 0) set.add(value);
		else if (op == 1) set.remove(value);
		else set.contains(value);
	}
}

int main()
{
	for (int num_thread = 1; num_thread <= MAX_THREADS; num_thread *= 2) {
		set.clear();
		std::vector<std::thread> threads;
		auto start = high_resolution_clock::now();
		for (int i = 0; i < num_thread; ++i)
			threads.emplace_back(benchmark, num_thread);
		for (auto& th : threads)
			th.join();
		auto end = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(end - start);

		std::cout << "Threads: " << num_thread << ", Duration: " << duration.count() << "ms.\n";
		std::cout << "Set : "; set.print20();
	}
}