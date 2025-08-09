#pragma once

#include "NanoNetworking/Core/Core.hpp"

#include <cstdint>
#include <string>
#include <thread>
#include <chrono>
#include <future>
#include <expected>
#include <functional>
#include <string_view>

namespace Nano::Networking
{

	class Client;

	////////////////////////////////////////////////////////////////////////////////////
	// ConnectionInfo
	////////////////////////////////////////////////////////////////////////////////////
	class ConnectionInfo // Note: A wrapper around a future with some nice to have getters
	{
	public:
		std::string IpAddress;
		ConnectionStatus Status = ConnectionStatus::Disconnected;

	public:
		// Methods
		inline void Wait() const { m_ConnectionWaiting.wait(); }
		inline void WaitFor(uint64_t milliseconds) const { m_ConnectionWaiting.wait_for(std::chrono::milliseconds(milliseconds)); }

		inline bool IsDone() const { return m_ConnectionWaiting.valid(); }

		// Getters
		inline bool Failed() const { return ((Status == ConnectionStatus::FailedToConnect) || (Status == ConnectionStatus::FailedToInitialize) || (Status == ConnectionStatus::InvalidIP) || (Status == ConnectionStatus::Disconnected));  }

	private:
		std::future<void> m_ConnectionWaiting;

		friend class Client;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// Client
	////////////////////////////////////////////////////////////////////////////////////
	class Client
	{
	public:
		//using DataReceivedCallback = std::function<void(const Buffer)>;
		using DataReceivedCallbackFn = void (*)(void* userData, const Buffer buffer);
		//using ServerConnectedCallback = std::function<void()>;
		using ServerConnectedCallbackFn = void (*)(void* userData);
		//using ServerDisconnectedCallback = std::function<void()>;
		using ServerDisconnectedCallbackFn = void (*)(void* userData);
		using MessageCallbackFn = void (*)(void* userData, MessageType type, const std::string& message);
	public:
		// Constructor & Destructor
		Client(void* userData = nullptr, DataReceivedCallbackFn dataReceivedCallback = nullptr, ServerConnectedCallbackFn serverConnectedCallback = nullptr, ServerDisconnectedCallbackFn serverDisconnectedCallback = nullptr, MessageCallbackFn messageCallback = nullptr);
		~Client();

		// Connection methods
		const ConnectionInfo& Connect(std::string_view serverAddress, uint64_t pollingRateMs = 10, uint64_t timeoutMs = 10'000); // Note: ConnectionInfo can be safely discarded, but should be checked for failure
		void Disconnect(); // Note: Can safely be called even when not connected to anything.

		// Upload methods
		SendResult SendBufferToConnection(Buffer buffer);
		SendResult SendReliableBufferToConnection(Buffer buffer);

		// Note: Only std::string has a specialized method, the other will just be the address and size
		template<typename T> SendResult SendToConnection(const T& data) { return SendBufferToConnection(Buffer(&data, sizeof(T))); }
		template<typename T> SendResult SendReliableToConnection(const T& data) { return SendReliableBufferToConnection(Buffer(&data, sizeof(T)));}

		// Setters
		inline void SetUserData(void* data) { m_User.Data = data; }
		inline void SetDataReceivedCallback(const DataReceivedCallbackFn& function) { m_User.DataReceivedCallback = function; }
		inline void SetServerConnectedCallback(const ServerConnectedCallbackFn& function) { m_User.ServerConnectedCallback = function; }
		inline void SetServerDisconnectedCallback(const ServerDisconnectedCallbackFn& function) { m_User.ServerDisconnectedCallback = function; }
		inline void SetMessageCallback(const MessageCallbackFn& function) { m_User.MessageCallback = function; }

		// Getters
		inline bool IsConnected() const { return (m_Info.Status == ConnectionStatus::Connected); }

		inline const ConnectionInfo& GetConnectionInfo() const { return m_Info; }

	private:
		// Private methods
		void Thread(ConnectionInfo& info, std::promise<void>&& promise, uint64_t pollingRateMs, uint64_t timeoutMs);

		// Polling methods
		void PollIncomingMessages();
		void PollConnectionStateChanges();

		// Upload method
		SendResult SendBuffer(Buffer buffer, int network);

		// Static callbacks
		friend void Client_ConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* info);

	private:
		std::thread m_NetworkThread;

		struct
		{
			void* Data;
			DataReceivedCallbackFn DataReceivedCallback;
			ServerConnectedCallbackFn ServerConnectedCallback;
			ServerDisconnectedCallbackFn ServerDisconnectedCallback;
			MessageCallbackFn MessageCallback;
		} m_User;

		ConnectionInfo m_Info = {};

		// Steam connection
		ISteamNetworkingSockets* m_Interface = nullptr;
		HSteamNetConnection m_Connection = 0;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// Specialized templates
	////////////////////////////////////////////////////////////////////////////////////
	template<> 
	inline SendResult Client::SendToConnection<std::string>(const std::string& str) 
	{ 
		return SendBufferToConnection(Buffer(str.data(), str.size())); 
	}

	template<> 
	inline SendResult Client::SendReliableToConnection<std::string>(const std::string& str)
	{ 
		return SendReliableBufferToConnection(Buffer(str.data(), str.size())); 
	}

}