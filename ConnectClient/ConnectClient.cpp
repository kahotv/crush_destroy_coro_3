#include <cstdio>
#include <tuple>
#include <iostream>
#include <chrono>
#include <coroutine>
#include <thread>
#include <asio.hpp>
#include <asio/basic_waitable_timer.hpp>
#include <asio/experimental/awaitable_operators.hpp>

using namespace asio;
using namespace asio::ip;
using namespace asio::experimental;
using namespace asio::experimental::awaitable_operators;
namespace this_coro = asio::this_coro;

awaitable<void> corosleep(int ms)
{
	asio::steady_timer timer(co_await this_coro::executor);
	timer.expires_after(std::chrono::milliseconds(ms));
	auto [ec] = co_await timer.async_wait(as_tuple(use_awaitable));
}
awaitable<void> work(int num, int port)
{
	auto ctx = co_await this_coro::executor;
	tcp::endpoint ep({ address_v4::loopback() }, port);
	char buf[] = "abcdefg";

	std::vector<tcp::socket> v;

	for (int i = 0; i < num; i++)
	{
		tcp::socket sock(ctx);
		auto [ec] = co_await sock.async_connect(ep, as_tuple(use_awaitable));
		if (!ec)
		{
			sock.async_send(asio::buffer(buf), as_tuple(use_awaitable));
		}
		v.push_back(std::move(sock));
	}

	// close all socket
	co_await corosleep(1000);
}

int main()
{
	int num = 100;
	int port = 55555;
	asio::io_context ctx;

	co_spawn(ctx, work(num, port), detached);

	ctx.run();
	ctx.stop();
	printf("test end\n");
	system("pause");
}
