#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <numeric>
#include <array>

const int MAX_THREADS = 16;
using namespace std::chrono;

class NODE {
public:
	NODE(int x) : next(nullptr), value(x), removed(false) {}

	void lock() { mtx.lock(); }
	void unlock() { mtx.unlock(); }
	int value;
	NODE* next;
	std::mutex mtx;
	bool removed;
};

class NODE_SP {
public:
	NODE_SP(int x) : next(nullptr), value(x), removed(false) {}

	void lock() { mtx.lock(); }
	void unlock() { mtx.unlock(); }
	int value;
	NODE* next;
	std::mutex mtx;
	bool removed;
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

class F_SET {
public:
	F_SET() {
		head = new NODE(std::numeric_limits<int>::min());
		tail = new NODE(std::numeric_limits<int>::max());
		head->next = tail;
	}

	~F_SET()
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
		auto prev = head;
		prev->lock();
		auto curr = prev->next;
		curr->lock();

		while (curr->value < x) {
			prev->unlock();
			prev = curr;
			curr = curr->next;
			curr->lock();
		}
		
		if (curr->value == x) {
			prev->unlock(); curr->unlock();
			return false;
		}

		auto newNode = new NODE(x);
		newNode->next = curr;
		prev->next = newNode;
		prev->unlock(); curr->unlock();
		return true;
	}

	bool remove(int x)
	{
		auto prev = head;
		prev->lock();
		auto curr = prev->next;
		curr->lock();

		while (curr->value < x) {
			prev->unlock();
			prev = curr;
			curr = curr->next;
			curr->lock();
		}
		
		if (curr->value == x) {
			prev->next = curr->next;
			prev->unlock(); curr->unlock();
			delete curr;
			return true;
		}

		prev->unlock();	curr->unlock();
		return false;
	}

	bool contains(int x)
	{
		auto prev = head;
		prev->lock();
		auto curr = prev->next;
		curr->lock();

		while (curr->value < x) {
			prev->unlock();
			prev = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->value == x) {
			prev->unlock(); curr->unlock();
			return true;
		}
		prev->unlock(); curr->unlock();
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
	bool validate(NODE* prev, NODE* curr)
	{
		NODE* node = prev;
		while (node) {
			if (node == prev)
				return prev->next == curr;
			node = node->next;
		}
		return false;
	}

	NODE* head, * tail;
};

class O_SET {
public:
	O_SET() {
		head = new NODE(std::numeric_limits<int>::min());
		tail = new NODE(std::numeric_limits<int>::max());
		head->next = tail;
	}

	~O_SET()
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
		while (true)
		{
			auto prev = head;
			auto curr = prev->next;

			while (curr->value < x) {
				prev = curr;
				curr = curr->next;
			}

			prev->lock(); curr->lock();
			if (false == validate(x, prev, curr)) {
				prev->unlock(); curr->unlock();
				continue;
			}
			if (curr->value == x) {
				prev->unlock(); curr->unlock();
				return false;
			}

			auto newNode = new NODE(x);
			newNode->next = curr;
			prev->next = newNode;
			prev->unlock(); curr->unlock();
			return true;
		}
	}

	bool remove(int x)
	{
		while (true)
		{

			auto prev = head;
			auto curr = prev->next;

			while (curr->value < x) {
				prev = curr;
				curr = curr->next;
			}

			prev->lock(); curr->lock();
			if (false == validate(x, prev, curr)) {
				prev->unlock(); curr->unlock();
				continue;
			}

			if (curr->value == x) {
				prev->next = curr->next;
				prev->unlock(); curr->unlock();
				// delete curr;
				return true;
			}

			prev->unlock(); curr->unlock();
			return false;
		}
	}

	bool contains(int x)
	{
		while (true)
		{
			auto prev = head;
			auto curr = prev->next;

			while (curr->value < x) {
				prev = curr;
				curr = curr->next;
			}
			prev->lock(); curr->lock();
			if (false == validate(x, prev, curr)) {
				prev->unlock(); curr->unlock();
				continue;
			}

			if (curr->value == x) {
				prev->unlock(); curr->unlock();
				return true;
			}

			prev->unlock(); curr->unlock();
			return false;
		}
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

	bool validate(int x, NODE* p, NODE* c)
	{
		auto prev = head;
		auto curr = prev->next;
		while (curr->value < x) {
			prev = curr;
			curr = curr->next;
		}
		return ((prev == p) && (curr == c));
	}

private:
	NODE* head, * tail;
};

class L_SET {
public:
	L_SET() {
		head = new NODE(std::numeric_limits<int>::min());
		tail = new NODE(std::numeric_limits<int>::max());
		head->next = tail;
	}

	~L_SET()
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

	bool validate(int x, NODE* p, NODE* c)
	{
		return (!p->removed) && (!c->removed) && (p->next == c);
	}

	bool add(int x)
	{
		while (true) {
			auto prev = head;
			auto curr = prev->next;
			while (curr->value < x) {
				prev = curr;
				curr = curr->next;
			}

			prev->lock(); curr->lock();
			if (false == validate(x, prev, curr)) {
				prev->unlock(); curr->unlock();
				continue;
			}
			if (curr->value == x) {
				prev->unlock();	curr->unlock();
				return false;
			}
			auto newNode = new NODE(x);
			newNode->next = curr;
			prev->next = newNode;
			prev->unlock();	curr->unlock();
			return true;
		}
	}

	bool remove(int x)
	{
		while (true) {
			auto prev = head;
			auto curr = prev->next;
			while (curr->value < x) {
				prev = curr;
				curr = curr->next;
			}

			prev->lock(); curr->lock();
			if (false == validate(x, prev, curr)) {
				prev->unlock(); curr->unlock();
				continue;
			}
			if (curr->value != x) {
				prev->unlock();	curr->unlock();
				return false;
			}
			
			prev->next = curr->next;
			prev->unlock();	curr->unlock();
			return true;
		}
	}

	bool contains(int x)
	{
		while (true) {
			auto prev = head;
			auto curr = prev->next;
			while (curr->value < x) {
				prev = curr;
				curr = curr->next;
			}
			prev->lock(); curr->lock();
			if (false == validate(x, prev, curr)) {
				prev->unlock(); curr->unlock();
				continue;
			}
			if (curr->value == x) {
				prev->unlock();	curr->unlock();
				return true;
			}
			prev->unlock();	curr->unlock();
			return false;
		}
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
};

#include <queue>

class LF_NODE;

class AMR // Atomic Markable Reference
{
public:
	AMR(LF_NODE* ptr = nullptr, bool mark = false)
	{
		long long val = reinterpret_cast<long long>(ptr);
		if (mark) val |= 1;
		ptr_and_mark = val;
	}

	LF_NODE* get_ptr()
	{
		long long val = ptr_and_mark;
		return reinterpret_cast<LF_NODE*>(val & 0xFFFFFFFFFFFFFFFE);
	}

	bool get_mark()
	{
		return (1 == (ptr_and_mark & 1));
	}

	bool attempt_mark(LF_NODE* expected_ptr, bool new_mark)
	{
		return CAS(expected_ptr, expected_ptr, false, new_mark);
	}

	LF_NODE* get_ptr_and_mark(bool* mark)
	{
		long long val = ptr_and_mark;
		*mark = (1 == (val & 1));
		return reinterpret_cast<LF_NODE*>(val & 0xFFFFFFFFFFFFFFFE);
	}

	bool CAS(LF_NODE* expected_ptr, LF_NODE* new_ptr, bool expected_mark, bool new_mark)
	{
		long long expected_val = reinterpret_cast<long long>(expected_ptr);
		if (expected_mark) expected_val |= 1;
		long long new_val = reinterpret_cast<long long>(new_ptr);
		if (new_mark) new_val |= 1;
		return std::atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic<long long>*>(&ptr_and_mark), &expected_val, new_val);
	}
private:
	volatile long long ptr_and_mark;
};

class LF_NODE {
public:
	LF_NODE(int x) : value(x) {}

	int value;
	AMR next;
};

class LF_SET {
public:
	LF_SET() {
		head = new LF_NODE(std::numeric_limits<int>::min());
		tail = new LF_NODE(std::numeric_limits<int>::max());
		head->next = tail;
	}

	~LF_SET()
	{
		clear();
		delete head;
		delete tail;
	}

	void clear()
	{
		LF_NODE* curr = head->next.get_ptr();
		while (curr != tail) {
			LF_NODE* temp = curr;
			curr = curr->next.get_ptr();
			delete temp;
		}
		head->next = tail;
	}

	void find(LF_NODE*& prev, LF_NODE*& curr, int x)
	{
		while (true) {
			retry:
			prev = head;
			curr = prev->next.get_ptr();
			while (true){
				bool curr_mark;
				auto succ = curr->next.get_ptr_and_mark(&curr_mark);
				while (curr_mark) {
					if (!prev->next.CAS(curr, succ, false, false))
						goto retry;
					curr = succ;
					succ = curr->next.get_ptr_and_mark(&curr_mark);
				}
				if (curr->value >= x)
					return;
				prev = curr;
				curr = succ;
			}
		}
	}

	bool add(int x)
	{
		while (true)
		{
			LF_NODE* prev, * curr;
			find(prev, curr, x);

			if (curr->value == x)
				return false;

			auto newNode = new LF_NODE(x);
			newNode->next = curr;
			if(prev->next.CAS(curr, newNode, false, false))
				return true;
		}
	}

	bool remove(int x)
	{
		while (true)
		{
			LF_NODE* prev, * curr;
			find(prev, curr, x);

			if (curr->value == x) {
				if (!curr->next.attempt_mark(curr->next.get_ptr(), true))
					continue;
				if (prev->next.CAS(curr, curr->next.get_ptr(), false, false))
					return true;
			}
			return false;
		}
	}

	bool contains(int x)
	{
		while (true)
		{
			LF_NODE* prev, * curr;
			find(prev, curr, x);

			if (curr->value == x && !curr->next.get_mark()) {
				return true;
			}
			return false;
		}
	}

	void print20()
	{
		auto curr = head->next.get_ptr();
		for (int i = 0; i < 20 && curr != tail; ++i) {
			std::cout << curr->value << ", ";
			curr = curr->next.get_ptr();
		}
		std::cout << std::endl;
	}
private:
	LF_NODE* head, * tail;
};

LF_SET set;

const int LOOP = 400'0000;
const int RANGE = 100;

class HISTORY {
public:
	int op;
	int i_value;
	bool o_value;
	HISTORY(int o, int i, bool re) : op(o), i_value(i), o_value(re) {}
};

std::array<std::vector<HISTORY>, MAX_THREADS> history;

void check_history(int num_threads)
{
	std::array <int, RANGE> survive = {};
	std::cout << "Checking Consistency : ";
	if (history[0].size() == 0) {
		std::cout << "No history.\n";
		return;
	}
	for (int i = 0; i < num_threads; ++i) {
		for (auto& op : history[i]) {
			if (false == op.o_value) continue;
			if (op.op == 3) continue;
			if (op.op == 0) survive[op.i_value]++;
			if (op.op == 1) survive[op.i_value]--;
		}
	}
	for (int i = 0; i < RANGE; ++i) {
		int val = survive[i];
		if (val < 0) {
			std::cout << "ERROR. The value " << i << " removed while it is not in the set.\n";
			exit(-1);
		}
		else if (val > 1) {
			std::cout << "ERROR. The value " << i << " is added while the set already have it.\n";
			exit(-1);
		}
		else if (val == 0) {
			if (set.contains(i)) {
				std::cout << "ERROR. The value " << i << " should not exists.\n";
				exit(-1);
			}
		}
		else if (val == 1) {
			if (false == set.contains(i)) {
				std::cout << "ERROR. The value " << i << " shoud exists.\n";
				exit(-1);
			}
		}
	}
	std::cout << " OK\n";
}

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

void benchmark_check(int num_threads, int th_id)
{
	for (int i = 0; i < LOOP / num_threads; ++i) {
		int op = rand() % 3;
		switch (op) {
		case 0: {
			int v = rand() % RANGE;
			history[th_id].emplace_back(0, v, set.add(v));
			break;
		}
		case 1: {
			int v = rand() % RANGE;
			history[th_id].emplace_back(1, v, set.remove(v));
			break;
		}
		case 2: {
			int v = rand() % RANGE;
			history[th_id].emplace_back(2, v, set.contains(v));
			break;
		}
		}
	}
}

int main()
{
	using namespace std::chrono;
	for (int num_thread = MAX_THREADS; num_thread >= 1; num_thread /= 2) {
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

	std::cout << "\n\nConsistency Check\n";
	for (int num_thread = MAX_THREADS; num_thread >= 1; num_thread /= 2) {
		set.clear();
		for (auto& h : history)
			h.clear();
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