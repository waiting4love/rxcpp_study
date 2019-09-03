// scheduler_subscribe_observe_on.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
// 您可能已经在Rx代码中使用了调度程序，而没有明确说明要使用的调度程序的类型。
// 这是因为处理并发的所有Observable运算符都有多个重载。
// 如果不使用以调度程序作为参数的重载，Rx将使用最小并发原则选择默认调度程序。
// 这意味着选择引入满足运算符需求的最少并发性的调度程序。
// 例如，对于返回具有有限和少量消息的observable的运算符，Rx调用ImmediateScheduler。
// 对于返回潜在大量或无限数量消息的运算符，将调用CurrentThread。
// 在以下示例中，源可观察序列均使用EventLoopScheduler在其自己的线程中运行。

// 这将迅速排队观察者。此代码使用observe_on运算符，该运算符允许您指定要用于向观察者发送推送通知（OnNext）的上下文。
// 默认情况下，observe_on运算符确保在当前线程上尽可能多地调用OnNext。
// 您可以使用其重载并将OnNext输出重定向到不同的上下文。此外，您可以使用subscribe_on运算符返回将操作委派给特定调度程序的代理observable。
// 例如，对于UI密集型应用程序，您可以使用subscribe_on委派要在后台运行的调度程序上执行的所有后台操作，并将其传递给Concurrency.EventLoopScheduler。

// 您还应注意，通过使用observe_on运算符，将为通过原始可观察序列发出的每条消息安排一个操作。
// 这可能会改变时序信息，并给系统带来额外的压力。
// 如果您有一个查询组成在许多不同的执行上下文上运行的各种可观察序列，并且您正在查询中进行过滤，
// 则最好在查询中稍后放置observe_on。这是因为查询可能会过滤掉大量消息，并且在查询的前面放置observe_on运算符会对将要过滤掉的消息执行额外的操作。
// 在查询结束时调用observe_on运算符将产生最小的性能影响。

#include "pch.h"
#include <iostream>
#include <rxcpp/rx.hpp>
#include <thread>
#include <mutex>
#include <string>

std::mutex mu_cout;

void print_thread_id(std::string title) {
	std::lock_guard<std::mutex> guard(mu_cout);
	std::cout << title << std::this_thread::get_id() << std::endl;
}

// 可以看到map工作在主线程，而subscribe方法在第二线程。
// 说明observeOn只对其下方的operators和subscribe起作用
int observe_on_test() {
	//------- Print the main thread id
	printf("Main Thread Id is %d\n", std::this_thread::get_id());
	//-------- We are using observe_on here
	//-------- The Map will use the main thread 
	//-------- Subscribed Lambda will use a new thread
	rxcpp::observable<>::range(0, 15).
		map([](int i) {printf("Map %d : %d\n", std::this_thread::get_id(), i); return i; }).
		take(5).
		observe_on(rxcpp::synchronize_new_thread()).
		subscribe(
			[&](int i) { printf("Subs %d : %d\n", std::this_thread::get_id(), i); });
	//----------- Wait for Two Seconds
	rxcpp::observable<>::timer(std::chrono::milliseconds(2000)).subscribe([&](long) {});

	return 0;
}

// map和Subs都在第二线程，说明subscribe_on影响前后的
int subscribe_on_test() {
	//------- Print the main thread id
	printf("Main Thread Id is %d\n", std::this_thread::get_id());
	//-------- We are using subscribe_on here 
	//-------- The Map and subscribed Lambda will 
	//--------- use the secondary thread 
	rxcpp::observable<>::range(0, 15).
		map([](int i) { printf("Map %d : %d\n", std::this_thread::get_id(), i); return i; }).
		take(5).
		subscribe_on(rxcpp::synchronize_new_thread()).
		subscribe(
			[&](int i) { printf("Subs %d : %d\n", std::this_thread::get_id(), i); });
	//----------- Wait for Two Seconds 
	rxcpp::observable<>::timer(std::chrono::milliseconds(2000)).subscribe([&](long) {});
	return 0;
}

int main() {
	return subscribe_on_test();
}