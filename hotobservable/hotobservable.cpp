// hotobservable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
// publish_synchronized 把cold observable变成hot, 新的订阅会收到最近的数据
#include "pch.h"
#include <iostream>
#include <rxcpp/rx.hpp>
#include <memory>
#include <thread>
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
	auto threads = rxcpp::observe_on_new_thread();
	auto values = rxcpp::observable<>::interval(
		std::chrono::milliseconds(500), threads
		)
		.take(5)
		.publish();
	// Subscribe from the beginning
	values.subscribe(
		[](long v) {print_thread_id("[1] OnNext: %ld\n", v); },
		[]() {print_thread_id("[1] OnCompleted\n"); });
	// Another subscription from the beginning
	values.subscribe(
		[](long v) {print_thread_id("[2] OnNext: %ld\n", v); },
		[]() {print_thread_id("[2] OnCompleted\n"); });
	// Start emitting
	print_thread_id("connect...\n");
	values.connect();
	print_thread_id("connected!\n");
	// Wait before subscribing
	rxcpp::observable<>::timer(std::chrono::milliseconds(750)).subscribe([&](long) {
		values.subscribe(
			[](long v) {print_thread_id("[3] OnNext: %ld\n", v); },
			[]() {print_thread_id("[3] OnCompleted\n"); });
	});
	// Add blocking subscription to see results
	values.as_blocking().subscribe(
		[](long v) {print_thread_id("[4] OnNext: %ld\n", v); },
		[]() {print_thread_id("[4] OnCompleted\n"); });
}

// output:
// [5416] connect...
// [5416] connected!
// [3520][1] OnNext: 1
// [3520][2] OnNext : 1
// [3520][1] OnNext : 2
// [3520][2] OnNext : 2
// [3520][1] OnNext : 3
// [3520][2] OnNext : 3
// [3520][3] OnNext : 3
// [3520][4] OnNext : 3
// [3520][1] OnNext : 4
// [3520][2] OnNext : 4
// [3520][3] OnNext : 4
// [3520][4] OnNext : 4
// [3520][1] OnNext : 5
// [3520][2] OnNext : 5
// [3520][3] OnNext : 5
// [3520][4] OnNext : 5
// [3520][1] OnCompleted
// [3520][2] OnCompleted
// [3520][3] OnCompleted
// [3520][4] OnCompleted