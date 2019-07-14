// MuchThread.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "thread"
#include "future"
#include <windows.h>
#include <mutex>

std::mutex mut;
std::condition_variable data_cond;
//当 std::condition_variable 对象的某个 wait 函数被调用的时候，它使用 std::unique_lock(通过 std::mutex) 来锁住当前线程。
//当前线程会一直被阻塞，直到另外一个线程在相同的 std::condition_variable 对象上调用了 notification 函数来唤醒当前线程。
struct Socket
{
	int listen_fd;
	int client_fd;
	int len;
};

void Zero(Socket *socket)
{
	while (1) //Socket 监听
	{
		std::cout << "开始监听1" << std::endl;
		//监听socket接收
		//触发即跳出该循环
		Sleep(1000);
		std::cout << "监听到了2" << std::endl;
		socket->listen_fd = 1;
		if (socket->listen_fd)
		{
			data_cond.notify_one();
			std::unique_lock<std::mutex> lk(mut);//-------------------④
		}
	}
}

void First(Socket *socket)
{
		//设定接收大小
		while (1) //socket开始接受数据
		{
			//对数据进行分析
			std::unique_lock<std::mutex> lk(mut);//-------------------④
			data_cond.wait(lk);//-------------------⑤
			std::cout << "收数据 并解包3" << std::endl;
			
			Sleep(1000);
			std::cout << "解完包了,把标志位传给下一个线程4" << std::endl;
			data_cond.notify_one(); //唤醒线程
		}
		//分析完之后对Second解锁 

}
void Second(Socket *socket)
{
	while (1)
	{
		std::unique_lock<std::mutex> lk(mut);//-------------------④
		data_cond.wait(lk);//-------------------⑤
		std::cout << "开始进行修改或者查询5" << std::endl;
		Sleep(1000);
		std::cout << "修改或查询完毕6" << std::endl;
		Sleep(1000);

	}
}

int main()
{
	Socket socket;
	//socket 初始化
	std::thread thread0(Zero, &socket);
	std::thread thread1(First,&socket); //接收数据并分析数据
	std::thread thread2(Second,&socket); //进行写入和修改分区
	thread1.join();
	thread2.join();
	thread0.join();

}
/*
#include <iostream>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>
#include "Windows.h"

std::mutex mut;
std::queue<int> data_queue;
std::condition_variable data_cond;

void data_preparation_thread() {
	int data = 0;
	while (true) {
		data++;
		std::lock_guard<std::mutex> lk(mut);
		data_queue.push(data);
		data_cond.notify_one();
		std::cout << "after  notify_one" << std::endl;
		//std::this_thread::sleep_for(1000);
		Sleep(1000);
	}
}

void data_process_thread() {
	while (true) {
		std::unique_lock<std::mutex> lk(mut);
		std::cout << "before wait" << std::endl;
		data_cond.wait(lk, [] {return !data_queue.empty(); });
		std::cout << "after  wait" << std::endl;
		int data = data_queue.front();
		std::cout << data << std::endl;
		data_queue.pop();
		lk.unlock();
		//假设处理数据data的函数process要花费大量时间，所以提前解锁
		//process(data);
	}
}
int main() {
	std::thread t1(data_preparation_thread);
	std::thread t2(data_process_thread);
	t1.join();
	t2.join();
}*/