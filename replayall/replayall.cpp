// hotobservable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
// Turn a cold observable hot, send all earlier emitted values to any new subscriber, and allow connections to the source to be independent of subscriptions.

#include "pch.h"
#include <iostream>
#include <rxcpp/rx.hpp>
#include <memory>

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


int main()
{
	auto values = rxcpp::observable<>::interval(
		std::chrono::milliseconds(50), rxcpp::observe_on_new_thread()) |
		rxcpp::operators::take(5) | rxcpp::operators::replay();

	// subscribe from the beginning
	values.subscribe(
		[](int v) { print_thread_id("[1] onNext:%d\n", v); },
		[]() { print_thread_id("[1] onComplete\n"); }
	);

	print_thread_id("Start emitting!\n");
	// start emitting values
	values.connect();
	print_thread_id("Connected!\n");
	// Wait before subscribing
	rxcpp::observable<>::timer(std::chrono::milliseconds(100))
		.subscribe(
			[&](long) {
		values.as_blocking().subscribe(
			[](long v) {print_thread_id("[2] OnNext: %ld\n", v); },
			[]() {print_thread_id("[2] OnCompleted\n"); });
	});

}

// output
// [2128] Start emitting!
// [2128] Connected!
// [9484][1] onNext:1
// [9484][1] onNext : 2
// [9484][1] onNext : 3
// [9484][1] onNext : 4
// [2128][2] OnNext : 1
// [2128][2] OnNext : 2
// [2128][2] OnNext : 3
// [2128][2] OnNext : 4
// [9484][1] onNext : 5
// [9484][2] OnNext : 5
// [9484][1] onComplete
// [9484][2] OnCompleted