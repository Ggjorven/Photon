#include <iostream>

#include "NanoNetworking/Core/Core.hpp"
#include "NanoNetworking/Core/Utils.hpp"

#include "NanoNetworking/Client/Client.hpp"
#include "NanoNetworking/Server/Server.hpp"

#define NN_SERVER 1
#define NN_CLIENT (!NN_SERVER)

using namespace Nano::Networking;

void MessageReceived(void* userData, MessageType type, const std::string& message)
{
	std::cout << message << std::endl;
}

#if NN_CLIENT
int main(int argc, char* argv[])
{
	(void)argc; (void)argv;

	Client client(nullptr, nullptr, nullptr, nullptr, MessageReceived);
	const ConnectionInfo& info = client.Connect("127.0.0.1:8080", 10, 5000);

	info.Wait();
	if (info.Failed()) 
	{
		std::cout << "Failed to connect" << std::endl;
		return 1;
	}

	while (client.IsConnected())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	}

	return 0;
}
#elif NN_SERVER
int main(int argc, char* argv[])
{
	(void)argc; (void)argv;

	Server server(nullptr, nullptr, nullptr, nullptr, MessageReceived);
	server.SetClientConnectedCallback([](void*, const ClientInfo& info)
	{
		std::cout << "Client connected: " << info.ConnectionDesc << std::endl;
	});
	server.SetClientDisconnectedCallback([](void*, const ClientInfo& info)
	{
		std::cout << "Client disconnected: " << info.ConnectionDesc << std::endl;
	});
	
	const ServerInfo& info = server.Start(8080);

	info.Wait();
	while (server.IsUp())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	}

	return 0;
}
#else
	#error Invalid configuration
#endif