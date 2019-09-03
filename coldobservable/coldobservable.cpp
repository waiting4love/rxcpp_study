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
	auto eventloop = rxcpp::observe_on_event_loop();
	auto values = rxcpp::observable<>::interval(
		std::chrono::seconds(2)
	).take(2);

	print_thread_id("Main Thread\n");
	
	values.subscribe_on(eventloop).subscribe(
		[](int v) { print_thread_id("[1] onNext:%d\n", v); },
		[]() { print_thread_id("[1] onComplete\n"); }
	);
	values.subscribe_on(eventloop).subscribe(
		[](int v) { print_thread_id("[2] onNext:%d\n", v); },
		[]() { print_thread_id("[2] onComplete\n"); }
	);
	values.as_blocking().subscribe(
		[](int v) { print_thread_id("[3] onNext:%d\n", v); },
		[]() { print_thread_id("[3] onComplete\n"); }
	);
	rxcpp::observable<>::timer(std::chrono::seconds(2)).subscribe();
}

//Output:
//[12144] Main Thread
//[8968] [1] onNext:1
//[12000][2] onNext : 1
//[12144][3] onNext : 1
//[8968][1] onNext : 2
//[12000][2] onNext : 2
//[12000][2] onComplete
//[8968][1] onComplete
//[12144][3] onNext : 2
//[12144][3] onComplete