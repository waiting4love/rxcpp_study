// runloop.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
// 在主线程中自己维护一个事件循环，通过rxcpp::schedulers::run_loop 和 rxcpp::observe_on_run_loop
// 让订阅代码工作在主线程上

#include "pch.h"
#include <iostream>
#include <rxcpp/rx.hpp>

#include <mutex>
#include <string>
#include <stdarg.h> 

std::mutex mu_cout;

void print_thread_id(const char* fmt, ...) {
	std::lock_guard<std::mutex> guard(mu_cout);
	std::cout << "[" << std::this_thread::get_id() << "] ";
	va_list vl;
	va_start(vl, fmt);
	vprintf(fmt, vl);
	va_end(vl);
}

int main() {
	//------------ Print the Main Thread Id
	print_thread_id("Main Thread\n");
	//------- Instantiate a run_loop object
	//------- which will loop in the main thread
	rxcpp::schedulers::run_loop rlp;
	//------ Create a Coordination functionfor run loop
	auto main_thread = rxcpp::observe_on_run_loop(rlp);
	auto worker_thread = rxcpp::synchronize_new_thread();
	rxcpp::composite_subscription subscription;
	rxcpp::observable<>::range(0, 15)
		.map(
			[](int i) {	print_thread_id("Map: %d\n", i); return i * 5; }
		)
		.take(5)
		.subscribe_on(worker_thread)
		.observe_on(main_thread).
		subscribe(
			subscription,
			[&](int i) {print_thread_id("Sub: %d\n", i);}
		);

	while (subscription.is_subscribed() || !rlp.empty()) {
		while (!rlp.empty() && rlp.peek().when < rlp.now()) {
			rlp.dispatch();
		}
	}
	return 0;
}

// output
// [10632] Main Thread
// [10880] Map: 0
// [10880] Map : 1
// [10632] Sub : 0
// [10880] Map : 2
// [10632] Sub : 5
// [10632] Sub : 10
// [10880] Map : 3
// [10880] Map : 4
// [10632] Sub : 15
// [10632] Sub : 20