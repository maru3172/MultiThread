#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <numeric>
#include <array>
#include <memory>

const int MAX_THREADS = 16;
using namespace std::chrono;

class NODE_SP {
public:
	NODE_SP(int x) : next(nullptr), value(x), removed(false) {}

	void lock() { mtx.lock(); }
	void unlock() { mtx.unlock(); }
	std::shared_ptr<NODE_SP> next;
	std::mutex mtx;
	int value;
	bool removed;
};


//class NODE {
//public:
//	NODE(int x) : next(nullptr), value(x), removed(false) {}
//
//	void lock() { mtx.lock(); }
//	void unlock() { mtx.unlock(); }
//	int value;
//	NODE* next;
//	std::mutex mtx;
//	bool removed;
//};

//class C_SET {
//public:
//	C_SET() {
//		head = new NODE(std::numeric_limits<int>::min());
//		tail = new NODE(std::numeric_limits<int>::max());
//		head->next = tail;
//	}
//
//	~C_SET()
//	{
//		clear();
//		delete head;
//		delete tail;
//	}
//
//	void clear()
//	{
//		NODE* curr = head->next;
//		while (curr != tail) {
//			NODE* temp = curr;
//			curr = curr->next;
//			delete temp;
//		}
//		head->next = tail;
//	}
//
//	bool add(int x)
//	{
//		mtx.lock();
//		auto prev = head;
//		auto curr = prev->next;
//
//		while (curr->value < x) {
//			prev = curr;
//			curr = curr->next;
//		}
//
//		if (curr->value == x) {
//			mtx.unlock();
//			return false;
//		}
//
//		auto newNode = new NODE(x);
//		newNode->next = curr;
//		prev->next = newNode;
//		mtx.unlock();
//		return true;
//	}
//
//	bool remove(int x)
//	{
//		mtx.lock();
//		auto prev = head;
//		auto curr = prev->next;
//
//		while (curr->value < x) {
//			prev = curr;
//			curr = curr->next;
//		}
//
//		if (curr->value == x) {
//			prev->next = curr->next;
//			delete curr;
//			mtx.unlock();
//			return true;
//		}
//
//		mtx.unlock();
//		return false;
//	}
//
//	bool contains(int x)
//	{
//		mtx.lock();
//		auto prev = head;
//		auto curr = prev->next;
//
//		while (curr->value < x) {
//			prev = curr;
//			curr = curr->next;
//		}
//
//		if (curr->value == x) {
//			mtx.unlock();
//			return true;
//		}
//		mtx.unlock();
//		return false;
//	}
//
//	void print20()
//	{
//		auto curr = head->next;
//		for (int i = 0; i < 20 && curr != tail; ++i) {
//			std::cout << curr->value << ", ";
//			curr = curr->next;
//		}
//		std::cout << std::endl;
//	}
//
//private:
//	NODE* head, * tail;
//	std::mutex mtx;
//};
//
//class F_SET {
//public:
//	F_SET() {
//		head = new NODE(std::numeric_limits<int>::min());
//		tail = new NODE(std::numeric_limits<int>::max());
//		head->next = tail;
//	}
//
//	~F_SET()
//	{
//		clear();
//		delete head;
//		delete tail;
//	}
//
//	void clear()
//	{
//		NODE* curr = head->next;
//		while (curr != tail) {
//			NODE* temp = curr;
//			curr = curr->next;
//			delete temp;
//		}
//		head->next = tail;
//	}
//
//	bool add(int x)
//	{
//		auto prev = head;
//		prev->lock();
//		auto curr = prev->next;
//		curr->lock();
//
//		while (curr->value < x) {
//			prev->unlock();
//			prev = curr;
//			curr = curr->next;
//			curr->lock();
//		}
//		
//		if (curr->value == x) {
//			prev->unlock(); curr->unlock();
//			return false;
//		}
//
//		auto newNode = new NODE(x);
//		newNode->next = curr;
//		prev->next = newNode;
//		prev->unlock(); curr->unlock();
//		return true;
//	}
//
//	bool remove(int x)
//	{
//		auto prev = head;
//		prev->lock();
//		auto curr = prev->next;
//		curr->lock();
//
//		while (curr->value < x) {
//			prev->unlock();
//			prev = curr;
//			curr = curr->next;
//			curr->lock();
//		}
//		
//		if (curr->value == x) {
//			prev->lock(); curr->lock();
//			prev->next = curr->next;
//			prev->unlock(); curr->unlock();
//			delete curr;
//			return true;
//		}
//
//		return false;
//	}
//
//	bool contains(int x)
//	{
//		auto prev = head;
//		prev->lock();
//		auto curr = prev->next;
//		curr->lock();
//
//		while (curr->value < x) {
//			prev = curr;
//			curr = curr->next;
//		}
//
//		if (curr->value == x) {
//			prev->unlock(); curr->unlock();
//			return true;
//		}
//		prev->unlock(); curr->unlock();
//		return false;
//	}
//
//	void print20()
//	{
//		auto curr = head->next;
//		for (int i = 0; i < 20 && curr != tail; ++i) {
//			std::cout << curr->value << ", ";
//			curr = curr->next;
//		}
//		std::cout << std::endl;
//	}
//
//private:
//	bool validate(NODE* prev, NODE* curr)
//	{
//		NODE* node = prev;
//		while (node) {
//			if (node == prev)
//				return prev->next == curr;
//			node = node->next;
//		}
//		return false;
//	}
//
//	NODE* head, * tail;
//};
//
//class O_SET {
//public:
//	O_SET() {
//		head = new NODE(std::numeric_limits<int>::min());
//		tail = new NODE(std::numeric_limits<int>::max());
//		head->next = tail;
//	}
//
//	~O_SET()
//	{
//		clear();
//		delete head;
//		delete tail;
//	}
//
//	void clear()
//	{
//		NODE* curr = head->next;
//		while (curr != tail) {
//			NODE* temp = curr;
//			curr = curr->next;
//			delete temp;
//		}
//		head->next = tail;
//	}
//
//	bool add(int x)
//	{
//		while (true)
//		{
//			auto prev = head;
//			auto curr = prev->next;
//
//			while (curr->value < x) {
//				prev = curr;
//				curr = curr->next;
//			}
//
//			prev->lock(); curr->lock();
//			if (false == validate(x, prev, curr)) {
//				prev->unlock(); curr->unlock();
//				continue;
//			}
//			if (curr->value == x) {
//				prev->unlock(); curr->unlock();
//				return false;
//			}
//
//			auto newNode = new NODE(x);
//			newNode->next = curr;
//			prev->next = newNode;
//			prev->unlock(); curr->unlock();
//			return true;
//		}
//	}
//
//	bool remove(int x)
//	{
//		while (true)
//		{
//
//			auto prev = head;
//			auto curr = prev->next;
//
//			while (curr->value < x) {
//				prev = curr;
//				curr = curr->next;
//			}
//
//			prev->lock(); curr->lock();
//			if (false == validate(x, prev, curr)) {
//				prev->unlock(); curr->unlock();
//				continue;
//			}
//
//			if (curr->value == x) {
//				prev->next = curr->next;
//				prev->unlock(); curr->unlock();
//				// delete curr;
//				return true;
//			}
//
//			prev->unlock(); curr->unlock();
//			return false;
//		}
//	}
//
//	bool contains(int x)
//	{
//		while (true)
//		{
//			auto prev = head;
//			auto curr = prev->next;
//
//			while (curr->value < x) {
//				prev = curr;
//				curr = curr->next;
//			}
//			prev->lock(); curr->lock();
//			if (false == validate(x, prev, curr)) {
//				prev->unlock(); curr->unlock();
//				continue;
//			}
//
//			if (curr->value == x) {
//				prev->unlock(); curr->unlock();
//				return true;
//			}
//
//			prev->unlock(); curr->unlock();
//			return false;
//		}
//	}
//
//	void print20()
//	{
//		auto curr = head->next;
//		for (int i = 0; i < 20 && curr != tail; ++i) {
//			std::cout << curr->value << ", ";
//			curr = curr->next;
//		}
//		std::cout << std::endl;
//	}
//
//	bool validate(int x, NODE* p, NODE* c)
//	{
//		auto prev = head;
//		auto curr = prev->next;
//		while (curr->value < x) {
//			prev = curr;
//			curr = curr->next;
//		}
//		return ((prev == p) && (curr == c));
//	}
//
//private:
//	NODE* head, * tail;
//};
//
//class L_SET {
//public:
//	L_SET() {
//		head = new NODE(std::numeric_limits<int>::min());
//		tail = new NODE(std::numeric_limits<int>::max());
//		head->next = tail;
//	}
//
//	~L_SET()
//	{
//		clear();
//		delete head;
//		delete tail;
//	}
//
//	void clear()
//	{
//		NODE* curr = head->next;
//		while (curr != tail) {
//			NODE* temp = curr;
//			curr = curr->next;
//			delete temp;
//		}
//		head->next = tail;
//	}
//
//	bool add(int x)
//	{
//		while (true)
//		{
//			auto prev = head;
//			auto curr = prev->next;
//
//			while (curr->value < x) {
//				prev = curr;
//				curr = curr->next;
//			}
//
//			prev->lock(); curr->lock();
//			if (false == validate(x, prev, curr)) {
//				prev->unlock(); curr->unlock();
//				continue;
//			}
//			if (curr->value == x) {
//				prev->unlock(); curr->unlock();
//				return false;
//			}
//
//			auto newNode = new NODE(x);
//			newNode->next = curr;
//			prev->next = newNode;
//			prev->unlock(); curr->unlock();
//			return true;
//		}
//	}
//
//	bool remove(int x)
//	{
//		while (true)
//		{
//
//			auto prev = head;
//			auto curr = prev->next;
//
//			while (curr->value < x) {
//				prev = curr;
//				curr = curr->next;
//			}
//
//			prev->lock(); curr->lock();
//			if (false == validate(x, prev, curr)) {
//				prev->unlock(); curr->unlock();
//				continue;
//			}
//
//			if (curr->value == x) {
//				prev->next = curr->next;
//				prev->unlock(); curr->unlock();
//				// delete curr;
//				return true;
//			}
//
//			prev->unlock(); curr->unlock();
//			return false;
//		}
//	}
//
//	bool contains(int x)
//	{
//		while (true)
//		{
//			auto prev = head;
//			auto curr = prev->next;
//
//			while (curr->value < x) {
//				prev = curr;
//				curr = curr->next;
//			}
//			prev->lock(); curr->lock();
//			if (false == validate(x, prev, curr)) {
//				prev->unlock(); curr->unlock();
//				continue;
//			}
//
//			if (curr->value == x) {
//				prev->unlock(); curr->unlock();
//				return true;
//			}
//
//			prev->unlock(); curr->unlock();
//			return false;
//		}
//	}
//
//	void print20()
//	{
//		auto curr = head->next;
//		for (int i = 0; i < 20 && curr != tail; ++i) {
//			std::cout << curr->value << ", ";
//			curr = curr->next;
//		}
//		std::cout << std::endl;
//	}
//
//	bool validate(int x, NODE* p, NODE* c)
//	{
//		return (p->removed == false) && (c->removed == false) && (p->next == c);
//	}
//
//private:
//	NODE* head, * tail;
//};

#include <queue>

//class L_SET_FL {
//public:
//	void my_delete(NODE* node)
//	{
//		std::lock_guard<std::mutex> lg(fl_mtx);
//		free_list.push(node);
//	}
//
//	void recycle()
//	{
//
//	}
//
//	L_SET_FL() {
//		head = new NODE(std::numeric_limits<int>::min());
//		tail = new NODE(std::numeric_limits<int>::max());
//		head->next = tail;
//	}
//
//	~L_SET_FL()
//	{
//		clear();
//		delete head;
//		delete tail;
//	}
//
//	void clear()
//	{
//		NODE* curr = head->next;
//		while (curr != tail) {
//			NODE* temp = curr;
//			curr = curr->next;
//			delete temp;
//		}
//		head->next = tail;
//	}
//
//	bool add(int x)
//	{
//		while (true)
//		{
//			auto prev = head;
//			auto curr = prev->next;
//
//			while (curr->value < x) {
//				prev = curr;
//				curr = curr->next;
//			}
//
//			prev->lock(); curr->lock();
//			if (false == validate(x, prev, curr)) {
//				prev->unlock(); curr->unlock();
//				continue;
//			}
//			if (curr->value == x) {
//				prev->unlock(); curr->unlock();
//				return false;
//			}
//
//			auto newNode = new NODE(x);
//			newNode->next = curr;
//			prev->next = newNode;
//			prev->unlock(); curr->unlock();
//			return true;
//		}
//	}
//
//	bool remove(int x)
//	{
//		while (true)
//		{
//
//			auto prev = head;
//			auto curr = prev->next;
//
//			while (curr->value < x) {
//				prev = curr;
//				curr = curr->next;
//			}
//
//			prev->lock(); curr->lock();
//			if (false == validate(x, prev, curr)) {
//				prev->unlock(); curr->unlock();
//				continue;
//			}
//
//			if (curr->value == x) {
//				curr->removed = false;
//				prev->next = curr->next;
//				prev->unlock(); curr->unlock();
//				my_delete(curr);
//				return true;
//			}
//
//			prev->unlock(); curr->unlock();
//			return false;
//		}
//	}
//
//	bool contains(int x)
//	{
//		while (true)
//		{
//			auto prev = head;
//			auto curr = prev->next;
//
//			while (curr->value < x) {
//				prev = curr;
//				curr = curr->next;
//			}
//			prev->lock(); curr->lock();
//			if (false == validate(x, prev, curr)) {
//				prev->unlock(); curr->unlock();
//				continue;
//			}
//
//			if (curr->value == x) {
//				prev->unlock(); curr->unlock();
//				return true;
//			}
//
//			prev->unlock(); curr->unlock();
//			return false;
//		}
//	}
//
//	void print20()
//	{
//		auto curr = head->next;
//		for (int i = 0; i < 20 && curr != tail; ++i) {
//			std::cout << curr->value << ", ";
//			curr = curr->next;
//		}
//		std::cout << std::endl;
//	}
//
//	bool validate(int x, NODE* p, NODE* c)
//	{
//		return (p->removed == false) && (c->removed == false) && (p->next == c);
//	}
//
//private:
//	NODE* head, * tail;
//	std::queue<NODE*> free_list;
//	std::mutex fl_mtx;
//};

class L_SET_SP {
public:

	L_SET_SP() {
		head = std::make_shared<NODE_SP>(std::numeric_limits<int>::min());
		tail = std::make_shared<NODE_SP>(std::numeric_limits<int>::max());
		head->next = tail;
	}

	~L_SET_SP()
	{
		clear();
	}

	void clear()
	{
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
			if (false == validate(prev, curr)) {
				prev->unlock(); curr->unlock();
				continue;
			}
			if (curr->value == x) {
				prev->unlock(); curr->unlock();
				return false;
			}

			auto newNode = std::make_shared<NODE_SP>(x);
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
			if (false == validate(prev, curr)) {
				prev->unlock(); curr->unlock();
				continue;
			}

			if (curr->value == x) {
				curr->removed = true;
				prev->next = curr->next;
				prev->unlock(); curr->unlock();
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
			if (false == validate(prev, curr)) {
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

	bool validate(const std::shared_ptr<NODE_SP>& p, const std::shared_ptr<NODE_SP>& c)
	{
		return (p->removed == false) && (c->removed == false) && (p->next == c);
	}

private:
	std::shared_ptr<NODE_SP> head, tail;
};

L_SET_SP set;

const int LOOP = 400'0000;
const int RANGE = 1000;

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
	//for (int num_thread = MAX_THREADS; num_thread >= 1; num_thread /= 2) {
	{
		set.clear();
		//std::vector<std::thread> threads;
		auto start = high_resolution_clock::now();
		//for (int i = 0; i < num_thread; ++i)
		//	threads.emplace_back(benchmark, 1);
		//for (auto& th : threads)
		//	th.join();
		benchmark_check(1, 0);
		check_history(1);
		auto end = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(end - start);

		std::cout << "Threads: " << 1 << ", Duration: " << duration.count() << "ms.\n";
		std::cout << "Set : "; set.print20();
		//}
	}

	std::cout << "\n\nConsistency Check\n";
	//for (int num_thread = MAX_THREADS; num_thread >= 1; num_thread /= 2) {
	{
		set.clear();
		for (auto& h : history)
			h.clear();
		//std::vector<std::thread> threads;
		auto start = high_resolution_clock::now();
		//for (int i = 0; i < num_thread; ++i)
		//	threads.emplace_back(benchmark, num_thread);
		//for (auto& th : threads)
		//	th.join();
		benchmark_check(1, 0);
		check_history(1);
		auto end = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(end - start);

		std::cout << "Threads: " << 1 << ", Duration: " << duration.count() << "ms.\n";
		std::cout << "Set : "; set.print20();
	}
	//}
}