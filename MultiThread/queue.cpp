#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include <Windows.h>
#include <queue>

const int MAX_THREADS = 16;
class NODE {
public:
	NODE(int x) : next(nullptr), value(x) {}

	int value;
	NODE* volatile next;
};

class DUMMY_MUTEX {
public:
	void lock() {}
	void unlock() {}
};

class C_QUEUE {
public:
	C_QUEUE()
	{
		head = tail = new NODE(-1);
	}

	~C_QUEUE()
	{
		clear();
		delete head;
	}

	void Enqueue(int x)
	{
		NODE* new_node = new NODE(x);
		set_lock.lock();
		tail->next = new_node;
		tail = new_node;
		set_lock.unlock();
	}
	
	int Dequeue()
	{
		NODE* temp;
		set_lock.lock();
		if (nullptr == head->next) {
			set_lock.unlock();
			return -1;
		}
		int res = head->next->value;
		temp = head;
		head = head->next;
		set_lock.unlock();
		delete temp;
		return res;
	}

	void clear()
	{
		NODE* curr = head->next;
		while (nullptr != curr) {
			NODE* next = curr->next;
			delete curr;
			curr = next;
		}
		tail = head;
		head->next = nullptr;
	}

	void print20()
	{
		NODE* curr = head->next;
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next)
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
private:
	NODE* head, * tail;
	std::mutex set_lock;
};

class LF_QUEUE {
public:
	LF_QUEUE()
	{
		head = tail = new NODE(-1);
	}

	~LF_QUEUE()
	{
		clear();
		delete head;
	}

	void clear()
	{
		NODE* curr = head->next;
		while (nullptr != curr) {
			NODE* next = curr->next;
			delete curr;
			curr = next;
		}
		tail = head;
		head->next = nullptr;
	}

	bool CAS(NODE* volatile* addr, NODE* expected, NODE* new_value)
	{
		return std::atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic<NODE*>*>(addr), &expected, new_value);
	}

	void Enqueue(int x)
	{
		NODE* new_node = new NODE(x);
		while (true) {
			NODE* old_tail = tail;
			NODE* old_next = old_tail->next;
			if (old_tail != tail) continue;
			if (old_next == nullptr) {
				if (CAS(&old_tail->next, nullptr, new_node)) {
					CAS(&tail, old_tail, new_node);
					return;
				}
			}
			else
				CAS(&tail, old_tail, old_next);
		}
	}

	int Dequeue()
	{
		while (true) {
			NODE* old_head = head;
			NODE* old_next = old_head->next;
			NODE* old_tail = tail;
			if (old_head != head)
				continue;
			if (old_next == nullptr)
				return -1;
			if (old_tail == old_head) {
				CAS(&tail, old_tail, old_next);
				continue;
			}
			int res = old_next->value;
			if (CAS(&head, old_head, old_next)) {
				delete old_head;
				return res;
			}
		}
	}

	void print20()
	{
		NODE* curr = head->next;
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next)
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
private:
	NODE* volatile head, * volatile tail;
};

class STNODE;
class STPTR {
public:
	void set_ptr(STNODE* p)
	{
		raw = reinterpret_cast<long long>(p) << 32;
	}

	STNODE* get_ptr()
	{
		return reinterpret_cast<STNODE*>(raw >> 32);
	}

	STNODE* get_ptr(int* stamp)
	{
		long long cur_raw = raw;
		*stamp = static_cast<int>(cur_raw & 0xFFFFFFFF);
		return reinterpret_cast<STNODE*>(cur_raw >> 32);
	}

	bool CAS(STNODE* old_value, STNODE* new_value, int old_stamp, int new_stamp)
	{
		long long old_raw = (reinterpret_cast<long long>(old_value) << 32) | old_stamp;
		long long new_raw = (reinterpret_cast<long long>(new_value) << 32) | new_stamp;
		return std::atomic_compare_exchange_strong(&raw, &old_raw, new_raw);
	}

	std::atomic<long long> raw;
};

class STNODE {
public:
	STNODE(int x) : value(x) {}

	int value;
	STPTR next;
};

class LFST_QUEUE32 {
public:
	LFST_QUEUE32()
	{
		head.set_ptr(new STNODE(-1));
		tail.set_ptr(head.get_ptr());
	}

	~LFST_QUEUE32()
	{
		clear();
		delete head.get_ptr();
	}

	void clear()
	{
		STNODE* curr = head.get_ptr()->next.get_ptr();
		while (nullptr != curr) {
			STNODE* next = curr->next.get_ptr();
			delete curr;
			curr = next;
		}
		tail.set_ptr(head.get_ptr());
		head.get_ptr()->next.set_ptr(nullptr);
	}

	void Enqueue(int x)
	{
		STNODE* new_node = new STNODE(x);
		while (true) {
			int tail_stamp = 0;
			STNODE* old_tail = tail.get_ptr(&tail_stamp);
			int next_stamp = 0;
			STNODE* old_next = old_tail->next.get_ptr(&next_stamp);
			if (old_tail != tail.get_ptr()) continue;
			if (old_next == nullptr) {
				if (old_tail->next.CAS(nullptr, new_node, next_stamp, next_stamp + 1)) {
					tail.CAS(old_tail, new_node, tail_stamp, tail_stamp + 1);
					return;
				}
			}
			else
				tail.CAS(old_tail, old_next, tail_stamp, tail_stamp + 1);
		}
	}

	int Dequeue()
	{
		while (true) {
			int head_stamp = 0;
			STNODE* old_head = head.get_ptr(&head_stamp);
			int next_stamp = 0;
			STNODE* old_next = old_head->next.get_ptr(&next_stamp);
			int tail_stamp = 0;
			STNODE* old_tail = tail.get_ptr(&tail_stamp);
			if (old_head != head.get_ptr()) continue;
			if (old_next == nullptr) return -1;
			if (old_tail == old_head) {
				tail.CAS(old_tail, old_next, tail_stamp, tail_stamp + 1);
				continue;
			}
			int res = old_next->value;
			if (head.CAS(old_head, old_next, head_stamp, head_stamp + 1)) {
				// delete old_head;
				return res;
			}
		}
	}

	void print20()
	{
		STNODE* curr = head.get_ptr()->next.get_ptr();
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next.get_ptr())
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
private:
	STPTR head, tail;
};

class STNODE64;
class STPTR64 {
public:
	void set_ptr(STNODE64* p)
	{
		ptr = p;
	}

	STNODE64* get_ptr()
	{
		return ptr;
	}

	STNODE64* get_ptr(long long* stamp)
	{
		STPTR64 temp{ 0, 0 };
		STPTR64 old{ 0,0 };
		if (!CAS128(&temp, &old, this)) {
			std::cout << "CAS128 failed in get_ptr!\n";
			exit(-1);
		}
		*stamp = temp.stamp;
		return temp.ptr;
	}

	bool CAS128(STPTR64* addr, STPTR64* expected, STPTR64* new_value)
	{
		return InterlockedCompareExchange128(reinterpret_cast<long long*>(addr), new_value->stamp, reinterpret_cast<long long>(new_value->ptr), reinterpret_cast<long long*>(expected));
	}

	bool CAS(STNODE64* old_value, STNODE64* new_value, long long old_stamp, long long new_stamp)
	{
		STPTR64 old_p{ old_value, old_stamp };
		STPTR64 new_p{ new_value, new_stamp };
		return CAS128(this, &old_p, &new_p);
	}

	alignas(16) STNODE64* volatile ptr;
	long long stamp;
};

thread_local std::queue<STNODE64*> free_nodes;

class STNODE64 {
public:
	STNODE64(long long v) : value(v) {}

	long long value;
	STPTR64 next;
};

class LFST_QUEUE64 {
public:
	LFST_QUEUE64()
	{
		head.set_ptr(new STNODE64(-1));
		tail.set_ptr(head.get_ptr());
	}

	~LFST_QUEUE64()
	{
		clear();
		delete head.get_ptr();
	}

	void clear()
	{
		STNODE64* curr = head.get_ptr()->next.get_ptr();
		while (nullptr != curr) {
			STNODE64* next = curr->next.get_ptr();
			delete curr;
			curr = next;
		}
		tail.set_ptr(head.get_ptr());
		head.get_ptr()->next.set_ptr(nullptr);
	}

	void Enqueue(int x)
	{
		STNODE64* new_node;
		if (free_nodes.empty()) new_node = new STNODE64(x);
		else {
			new_node = free_nodes.front();
			free_nodes.pop();
			new_node->value = x;
			new_node->next.set_ptr(nullptr);
		}
		while (true) {
			long long tail_stamp = 0;
			STNODE64* old_tail = tail.get_ptr(&tail_stamp);
			long long next_stamp = 0;
			STNODE64* old_next = old_tail->next.get_ptr(&next_stamp);
			if (old_tail != tail.get_ptr()) continue;
			if (old_next == nullptr) {
				if (old_tail->next.CAS(nullptr, new_node, next_stamp, next_stamp + 1)) {
					tail.CAS(old_tail, new_node, tail_stamp, tail_stamp + 1);
					return;
				}
			}
			else
				tail.CAS(old_tail, old_next, tail_stamp, tail_stamp + 1);
		}
	}

	int Dequeue()
	{
		while (true) {
			long long head_stamp = 0;
			STNODE64* old_head = head.get_ptr(&head_stamp);
			long long next_stamp = 0;
			STNODE64* old_next = old_head->next.get_ptr(&next_stamp);
			long long tail_stamp = 0;
			STNODE64* old_tail = tail.get_ptr(&tail_stamp);
			if (old_head != head.get_ptr()) continue;
			if (old_next == nullptr) return -1;
			if (old_tail == old_head) {
				tail.CAS(old_tail, old_next, tail_stamp, tail_stamp + 1);
				continue;
			}
			int res = old_next->value;
			if (head.CAS(old_head, old_next, head_stamp, head_stamp + 1)) {
				free_nodes.push(old_head);
				return res;
			}
		}
	}

	void print20()
	{
		STNODE64* curr = head.get_ptr()->next.get_ptr();
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next.get_ptr())
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
private:
	STPTR64 head, tail;
};

//class SH_NODE {
//public:
//	SH_NODE() : v(-1) {}
//	SH_NODE(int x) : v(x) {}
//
//	int v;
//	std::atomic<std::shared_ptr<SH_NODE>> next;
//};
//
//class LF_SH_QUEUE {
//public:
//	LF_SH_QUEUE()
//	{
//		head = std::make_shared<SH_NODE>(-1);
//		tail.store(head);
//	}
//	bool CAS(std::atomic<std::shared_ptr<SH_NODE>>& next, std::shared_ptr<SH_NODE> old_p, const std::shared_ptr<SH_NODE>& new_p)
//	{
//		return next.compare_exchange_strong(old_p, new_p);
//	}
//	void ENQ(int x)
//	{
//		std::shared_ptr<SH_NODE> e = std::make_shared<SH_NODE>(x);
//		while (true) {
//			std::shared_ptr<SH_NODE> last = tail;
//			std::shared_ptr<SH_NODE> next = last->next;
//			std::shared_ptr<SH_NODE> comp = tail;
//			if (last != comp) continue;
//			if (nullptr == next) {
//				if (true == CAS(last->next, nullptr, e)) {
//					CAS(tail, last, e);
//					return;
//				}
//			}
//			else
//				CAS(tail, last, next);
//		}
//	}
//
//	int DEQ()
//	{
//		while (true) {
//			std::shared_ptr<SH_NODE>  first = head;
//			std::shared_ptr<SH_NODE>  last = tail;
//			std::shared_ptr<SH_NODE>  next = first->next;
//			std::shared_ptr<SH_NODE> comp = head;
//			if (first != comp) continue;
//			if (nullptr == next) return -1;
//			if (first == last) {
//				CAS(tail, last, next);
//				continue;
//			}
//			int value = next->v;
//			if (false == CAS(head, first, next))
//				continue;
//			return value;
//		}
//	}
//
//	void print20()
//	{
//		std::shared_ptr<SH_NODE> p = head;
//		p = p->next;
//		for (int i = 0; i < 20; ++i) {
//			if (p == nullptr) break;
//			std::cout << p->v << ", ";
//			p = p->next;
//		}
//		std::cout << std::endl;
//	}
//
//	void clear()
//	{
//		head.store(tail);
//	}
//private:
//	std::atomic<std::shared_ptr<SH_NODE>> head;
//	std::atomic<std::shared_ptr<SH_NODE>> tail;
//};

LFST_QUEUE64 my_queue;

const int NUM_TEST = 10000000;

void benchmark(const int num_thread, int th_id)
{
	const int loop_count = NUM_TEST / num_thread;

	int key = 0;
	for (int i = 0; i < loop_count; i++) {
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
	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		my_queue.clear();
		auto st = high_resolution_clock::now();
		std::vector<std::thread> threads;
		for (int i = 0; i < num_threads; ++i)
			threads.emplace_back(benchmark, num_threads, i);
		for (int i = 0; i < num_threads; ++i)
			threads[i].join();
		auto ed = high_resolution_clock::now();
		auto time_span = duration_cast<milliseconds>(ed - st).count();
		std::cout << "Thread Count = " << num_threads << ", Exec Time = " << time_span << "ms.\n";
		std::cout << "Result : ";  my_queue.print20();
	}
}