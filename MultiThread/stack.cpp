#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include <set>
#include <unordered_set>
#include <immintrin.h>

const int MAX_THREADS = 16;
int num_threads = 0;
int count = 0;

class NODE {
public:
	NODE(int v) : value(v), next(nullptr) {}

	int value;
	NODE* volatile next;
};

class DUMMY_MUTEX {
public:
	void lock() {}
	void unlock() {}
};

class C_STACK {
public:
	C_STACK() {
		top = nullptr;
	}

	~C_STACK() {
		clear();
	}

	void clear() {
		while (nullptr != top) pop();
	}

	void push(int x)
	{
		NODE* new_node = new NODE(x);
		set_lock.lock();
		new_node->next = top;
		top = new_node;
		set_lock.unlock();
	}

	int pop()
	{
		set_lock.lock();
		if (nullptr == top) {
			set_lock.unlock();
			return -2;
		}
		int res = top->value;
		auto temp = top;
		top = top->next;
		set_lock.unlock();
		delete temp;
		return res;
	}

	void print20()
	{
		NODE* curr = top;
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next)
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}

public:
	NODE* top;
	std::mutex set_lock;
};

class LF_STACK {
public:
	LF_STACK() {
		top = nullptr;
	}

	~LF_STACK() {
		clear();
	}

	void clear() {
		while (nullptr != top) pop();
	}

	bool CAS(NODE* volatile* addr, NODE* expected, NODE* desired)
	{
		return std::atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic<NODE*>*>(addr), &expected, desired);
	}

	void push(int x)
	{
		NODE* new_node = new NODE(x);
		while (true) {
			NODE* curr_top = top;
			new_node->next = curr_top;
			if (CAS(&top, curr_top, new_node)) return;
		}
	}

	int pop()
	{
		while (true) {
			NODE* curr_top = top;
			if (nullptr == curr_top) return -2;
			NODE* curr_next = curr_top->next;
			int res = curr_top->value;
			if (curr_top != top) continue;
			if (CAS(&top, curr_top, curr_next)) return res;
		}
	}

	void print20()
	{
		NODE* curr = top;
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next)
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
private:
	NODE* top;
	DUMMY_MUTEX set_lock;
};

class BACKOFF {
public:
	BACKOFF(int min_d, int max_d) : minDelay(min_d), maxDelay(max_d), limit(min_d)
	{
		if (limit == 0) {
			std::cout << "ERROR: minDelay must be > 0\n";
			exit(-1);
		}
	}

	void backoff()
	{
		int delay = rand() % limit;
		limit = limit * 2;
		if (limit > maxDelay) limit = maxDelay;
		// std::this_thread::sleep_for(std::chrono::microseconds(delay));
		for (int i = 0; i < delay; i++) _mm_pause();
	}
private:
	int minDelay, maxDelay;
	int limit;
};

class LFBO_STACK {
public:
	LFBO_STACK() {
		top = nullptr;
	}

	~LFBO_STACK() {
		clear();
	}

	void clear() {
		while (nullptr != top) pop();
	}

	bool CAS(NODE* volatile* addr, NODE* expected, NODE* desired)
	{
		return std::atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic<NODE*>*>(addr), &expected, desired);
	}

	void push(int x)
	{
		BACKOFF backoff{ 1,num_threads };
		NODE* new_node = new NODE(x);
		while (true) {
			NODE* curr_top = top;
			new_node->next = curr_top;
			if (CAS(&top, curr_top, new_node)) return;
			else backoff.backoff();
		}
	}

	int pop()
	{
		BACKOFF backoff{ 1,num_threads };
		while (true) {
			NODE* curr_top = top;
			if (nullptr == curr_top) return -2;
			NODE* curr_next = curr_top->next;
			if (curr_top != top) continue;
			int res = curr_top->value;
			if (CAS(&top, curr_top, curr_next)) {
				return res;
			}
			backoff.backoff();
		}
	}

	void print20()
	{
		NODE* curr = top;
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next)
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
private:
	NODE* top;
	DUMMY_MUTEX set_lock;
};

constexpr int ST_EMPTY = 0;
constexpr int ST_WAITING = 1;
constexpr int ST_BUSY = 2;
constexpr int TIME_OUT = 100;

class LockFreeExchanger {
public:
	LockFreeExchanger() : slot(0) {}

	int exchange(int my_item, bool* busy) {
		*busy = false;
		for(int j = 0; j < TIME_OUT; ++j) {
			long long s = slot;
			int item = (int)(s & 0xFFFFFFFF);
			int status = (int)((s >> 32) & 0x3);
			long long new_slot;
			switch (status) {
			case ST_EMPTY:
				new_slot = ((long long)my_item & 0xFFFFFFFF) | ((long long)ST_WAITING << 32);
				if (std::atomic_compare_exchange_strong(&slot, &s, new_slot)) {
					for (int i = 0; i < TIME_OUT; ++i) {
						s = slot;
						status = (int)((s >> 32) & 0x3);
						if (status == ST_BUSY) {
							int their_item = (int)(s & 0xFFFFFFFF);
							++count;
							slot = 0; // set to EMPTY
							return their_item;
						}
					}
					if (std::atomic_compare_exchange_strong(&slot, &s, 0)) { // set to EMPTY
						slot = 0; // set to EMPTY
						return -2; // TIME OUT
					}
					else {
						// someone is BUSY
						s = slot;
						int their_item = (int)(s & 0xFFFFFFFF);
						slot = 0; // set to EMPTY
						return their_item;
					}
				}
				break;
			case ST_WAITING:
				new_slot = ((long long)my_item & 0xFFFFFFFF) | ((long long)ST_BUSY << 32);
				if (std::atomic_compare_exchange_strong(&slot, &s, new_slot)) {
					int their_item = item;
					++count;
					return their_item;
				}
				break;
			case ST_BUSY:
				*busy = true;
				break;
			}
		}
		return -2; // TIME OUT
	}
private:
	alignas(64) std::atomic_llong slot;
};

class EliminationArray{
public:
	EliminationArray() { range = 1; }

	~EliminationArray() {}

	int Visit(int value) {
		int slot = rand() % range;
		bool busy;
		int ret = exchanger[slot].exchange(value, &busy);
		int old_range = range;
		if ((ret == -2) && (old_range > 1)) // TIME OUT
			range = old_range - 1;
		if ((busy) && (old_range <= num_threads / 2 - 1))
			range = old_range + 1; // MAX RANGE is # of thread / 2
		return ret;
	}
private:
	int range;
	LockFreeExchanger exchanger[MAX_THREADS / 2 - 1];
};


class LFEL_STACK {
public:
	LFEL_STACK() {
		top = nullptr;
	}

	~LFEL_STACK() {
		clear();
	}

	void clear() {
		while (nullptr != top) pop();
	}

	bool CAS(NODE* volatile* addr, NODE* expected, NODE* desired)
	{
		return std::atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic<NODE*>*>(addr), &expected, desired);
	}

	void push(int x)
	{
		NODE* new_node = new NODE(x);
		while (true) {
			NODE* old_top = top;
			new_node->next = old_top;

			if (CAS(&top, old_top, new_node)) return;

			int elim_res = elim_array.Visit(x);
			if (elim_res == -2) continue; // TIME OUT 
			else {
				delete new_node;
				return;
			}
		}
	}

	int pop()
	{
		while (true) {
			NODE* curr_top = top;
			if (nullptr == curr_top) return -2;

			NODE* next_node = curr_top->next;
			if (CAS(&top, curr_top, next_node)) {
				int res = curr_top->value;
				//delete curr_top;
				return res;
			}
			else {
				int ret = elim_array.Visit(ST_EMPTY);
				if (ret != -2) return ret;
			}
		}
	}

	void print20()
	{
		NODE* curr = top;
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next)
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
private:
	NODE* top;
	EliminationArray elim_array;
};

LFEL_STACK my_stack;

struct HISTORY {
	std::vector<int> push_values, pop_values;
};

std::atomic_int stack_size;
thread_local int thread_id;
const int NUM_TEST = 10000000;

void benchmark(const int num_thread)
{
	int key = 0;
	const int loop_count = NUM_TEST / num_thread;
	for (auto i = 0; i < loop_count; ++i) {
		if ((rand() % 2 == 0) || (i < 1000))
			my_stack.push(key++);
		else
			my_stack.pop();
	}
}

void benchmark_test(const int th_id, const int num_threads, HISTORY& h)
{
	thread_id = th_id;
	int loop_count = NUM_TEST / num_threads;
	for (int i = 0; i < loop_count; i++) {
		if ((rand() % 2) || i < 128 / num_threads) {
			h.push_values.push_back(i);
			stack_size++;
			my_stack.push(i);
		}
		else {
			volatile int curr_size = stack_size--;
			int res = my_stack.pop();
			if (res == -2) {
				stack_size++;
				if ((curr_size > num_threads * 2) && (stack_size > num_threads)) {
					std::cout << "ERROR Non_Empty Stack Returned NULL\n";
					exit(-1);
				}
			}
			else h.pop_values.push_back(res);
		}
	}
}

void check_history(std::vector <HISTORY>& h)
{
	std::unordered_multiset <int> pushed, poped, in_stack;

	for (auto& v : h)
	{
		for (auto num : v.push_values) pushed.insert(num);
		for (auto num : v.pop_values) poped.insert(num);
		while (true) {
			int num = my_stack.pop();
			if (num == -2) break;
			poped.insert(num);
		}
	}
	for (auto num : pushed) {
		if (poped.count(num) < pushed.count(num)) {
			std::cout << "Pushed Number " << num << " does not exists in the STACK.\n";
			exit(-1);
		}
		if (poped.count(num) > pushed.count(num)) {
			std::cout << "Pushed Number " << num << " is poped more than " << poped.count(num) - pushed.count(num) << " times.\n";
			exit(-1);
		}
	}
	for (auto num : poped)
		if (pushed.count(num) == 0) {
			std::multiset <int> sorted;
			for (auto num : poped)
				sorted.insert(num);
			std::cout << "There were elements in the STACK no one pushed : ";
			int count = 20;
			for (auto num : sorted)
				std::cout << num << ", ";
			std::cout << std::endl;
			exit(-1);

		}
	std::cout << "NO ERROR detected.\n";
}

int main()
{
	using namespace std::chrono;

	/*for (int n = 1; n <= MAX_THREADS; n = n * 2) {
		num_threads = n;
		my_stack.clear();
		std::vector<std::thread> tv;
		std::vector<HISTORY> history;
		history.resize(n);
		stack_size = 0;
		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < n; ++i) {
			tv.emplace_back(benchmark_test, i, n, std::ref(history[i]));
		}
		for (auto& th : tv)
			th.join();
		auto end_t = high_resolution_clock::now();
		auto exec_t = end_t - start_t;
		size_t ms = duration_cast<milliseconds>(exec_t).count();
		std::cout << n << " Threads,  " << ms << "ms. ----";
		my_stack.print20();
		check_history(history);
	}*/

	for (int n = 1; n <= MAX_THREADS; n *= 2) {
		count = 0;
		num_threads = n;
		my_stack.clear();
		std::vector<std::thread> tv;
		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < n; ++i) {
			tv.emplace_back(benchmark, n);
		}
		for (auto& th : tv)
			th.join();
		auto end_t = high_resolution_clock::now();
		auto exec_t = end_t - start_t;
		size_t ms = duration_cast<milliseconds>(exec_t).count();
		std::cout << n << " Threads,  " << ms << "ms. ----";
		std::cout << n << " Threads, Successful Exchange count : " << count << "\n";
		my_stack.print20();
	}
}