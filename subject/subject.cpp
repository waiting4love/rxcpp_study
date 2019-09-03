// subject.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

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

int main()
{
	rxcpp::subjects::subject<int> subject;
	auto threads = rxcpp::observe_on_new_thread();

	auto obs = subject.get_observable().subscribe_on(threads);
	obs.subscribe([](int x) {
		print_thread_id("[1] emit: %d\n", x);
	});
	obs.subscribe([](int x) {
		print_thread_id("[2] emit: %d\n", x);
	});

	auto sub = subject.get_subscriber();
	sub.on_next(10);
	sub.on_next(20);
	sub.on_next(30);
	sub.on_completed();

	printf("Done");
}

