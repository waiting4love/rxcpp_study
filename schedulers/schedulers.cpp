// schedulers.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <rxcpp/rx.hpp>
int main() {
	//-------- Create a Coordination function
	auto coordination = rxcpp::identity_current_thread();
	//-------Instantiate a coordinator and create a worker
	auto worker = coordination.create_coordinator().get_worker();
	//--------- start and the period
	auto start = coordination.now() + std::chrono::milliseconds(1);
	auto period = std::chrono::milliseconds(1);
	//----------- Create an Observable (Replay ) 
	auto values = rxcpp::observable<>::interval(start, period).
		take(5).replay(2, coordination);
	//--------------- Subscribe first time using a Worker
	worker.schedule([&](const rxcpp::schedulers::schedulable&) {
		values.subscribe(
			[](long v) { printf("#1 -- %d : %ld\n", std::this_thread::get_id(), v); },
			[]() { printf("#1 --- OnCompleted\n"); });
	});
	worker.schedule([&](const rxcpp::schedulers::schedulable&) {
		values.subscribe(
			[](long v) {printf("#2 -- %d : %ld\n", std::this_thread::get_id(), v); },
			[]() {printf("#2 --- OnCompleted\n"); }); });
	//----- Start the emission of values
	worker.schedule([&](const rxcpp::schedulers::schedulable&) { values.connect(); });
	//------- Add blocking subscription to see results 
	values.as_blocking().subscribe();
	return 0;
}