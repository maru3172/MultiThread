//#include <iostream>
//#include <thread>
//#include <mutex>
//#include <chrono>
//#include <vector>
//#include <numeric>
//#include <array>
//
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
//
//class CE_QUEUE {
//public:
//private:
//
//};
//
//CE_QUEUE queue;
//
//const int MAX_THREADS = 32;
//const int LOOP = 1'0000;
//const int RANGE = 1000;
//
//thread_local int thread_id = 0;
//int num_threads = 0;
//
//class HISTORY {
//public:
//	int op;
//	int i_value;
//	bool o_value;
//	HISTORY(int o, int i, bool re) : op(o), i_value(i), o_value(re) {}
//};
//
//std::array<std::vector<HISTORY>, MAX_THREADS> history;
//
//void check_history(int num_threads)
//{
//	std::array <int, RANGE> survive = {};
//	std::cout << "Checking Consistency : ";
//	if (history[0].size() == 0) {
//		std::cout << "No history.\n";
//		return;
//	}
//	for (int i = 0; i < num_threads; ++i) {
//		for (auto& op : history[i]) {
//			if (false == op.o_value) continue;
//			if (op.op == 3) continue;
//			if (op.op == 0) survive[op.i_value]++;
//			if (op.op == 1) survive[op.i_value]--;
//		}
//	}
//	for (int i = 0; i < RANGE; ++i) {
//		int val = survive[i];
//		if (val < 0) {
//			std::cout << "ERROR. The value " << i << " removed while it is not in the set.\n";
//			exit(-1);
//		}
//		else if (val > 1) {
//			std::cout << "ERROR. The value " << i << " is added while the set already have it.\n";
//			exit(-1);
//		}
//		else if (val == 0) {
//			if (queue.contains(i)) {
//				std::cout << "ERROR. The value " << i << " should not exists.\n";
//				exit(-1);
//			}
//		}
//		else if (val == 1) {
//			if (false == queue.contains(i)) {
//				std::cout << "ERROR. The value " << i << " shoud exists.\n";
//				exit(-1);
//			}
//		}
//	}
//	std::cout << " OK\n";
//}
//
//void benchmark(const int num_threads, int th_id)
//{
//	thread_id = th_id;
//	for (int i = 0; i < LOOP / num_threads; ++i) {
//		int value = rand() % RANGE;
//		int op = rand() % 3;
//		if (op == 0) queue.add(value);
//		else if (op == 1) queue.remove(value);
//		else queue.contains(value);
//	}
//}
//
//void benchmark_check(int num_threads, int th_id)
//{
//	thread_id = th_id;
//	for (int i = 0; i < LOOP / num_threads; ++i) {
//		int op = rand() % 3;
//		switch (op) {
//		case 0: {
//			int v = rand() % RANGE;
//			history[th_id].emplace_back(0, v, queue.add(v));
//			break;
//		}
//		case 1: {
//			int v = rand() % RANGE;
//			history[th_id].emplace_back(1, v, queue.remove(v));
//			break;
//		}
//		case 2: {
//			int v = rand() % RANGE;
//			history[th_id].emplace_back(2, v, queue.contains(v));
//			break;
//		}
//		}
//	}
//}
//
//int main()
//{
//	using namespace std::chrono;
//
//	std::cout << "Consistency Check\n";
//	for (num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
//		queue.clear();
//		std::vector<std::thread> threads;
//		for (int i = 0; i < MAX_THREADS; ++i)
//			history[i].clear();
//		auto start = high_resolution_clock::now();
//		for (int i = 0; i < num_threads; ++i)
//			threads.emplace_back(benchmark_check, num_threads, i);
//		for (auto& th : threads)
//			th.join();
//		auto stop = high_resolution_clock::now();
//		auto duration = duration_cast<milliseconds>(stop - start);
//		std::cout << "Threads: " << num_threads
//			<< ", Duration: " << duration.count() << " ms.\n";
//		std::cout << "Set: "; queue.print20();
//		check_history(num_threads);
//	}
//
//	std::cout << "\nBenchmarking\n";
//	for (num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
//		queue.clear();
//		std::vector<std::thread> threads;
//		auto start = high_resolution_clock::now();
//		for (int i = 0; i < num_threads; ++i)
//			threads.emplace_back(benchmark, num_threads, i);
//		for (auto& th : threads)
//			th.join();
//		auto end = high_resolution_clock::now();
//		auto duration = duration_cast<milliseconds>(end - start);
//
//		std::cout << "Threads: " << num_threads << ", Duration: " << duration.count() << "ms.\n";
//		std::cout << "Set : "; queue.print20();
//	}
//
//}