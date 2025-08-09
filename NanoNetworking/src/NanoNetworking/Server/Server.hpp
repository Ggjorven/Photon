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

	class Server;

	////////////////////////////////////////////////////////////////////////////////////
	// ClientInfo
	////////////////////////////////////////////////////////////////////////////////////
	using ClientID = uint32_t;
	static_assert(std::is_same_v<HSteamNetConnection, ClientID>, "Current implementation requires HSteamNetConnection to be a uint32_t.");

	struct ClientInfo
	{
	public:
		ClientID ID;
		std::string ConnectionDesc;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// Server
	////////////////////////////////////////////////////////////////////////////////////
	class Server
	{
	public:
		//using DataReceivedCallback = std::function<void(const ClientInfo&, const Buffer)>;
		using DataReceivedCallbackFn = void (*)(void* userData, const ClientInfo& info, const Buffer data);
		//using ClientConnectedCallback = std::function<void(const ClientInfo&)>;
		using ClientConnectedCallbackFn = void (*)(void* userData, const ClientInfo& info);
		//using ClientDisconnectedCallback = std::function<void(const ClientInfo&)>;
		using ClientDisconnectedCallbackFn = void (*)(void* userData, const ClientInfo& info);
		using MessageCallbackFn = void (*)(void* userData, MessageType type, const std::string& message);
	public:
		// Constructor & Destructor
		Server(void* userData = nullptr, DataReceivedCallbackFn dataReceivedCallback = nullptr, ClientConnectedCallbackFn serverConnectedCallback = nullptr, ClientDisconnectedCallbackFn serverDisconnectedCallback = nullptr, MessageCallbackFn messageCallback = nullptr);
		~Server();

		// Methods
		void Start(uint16_t port, uint64_t pollingRateMs = 10);
		void Stop();

		void KickClient(ClientID clientID, const std::string& reason = "Kicked by host");

		// Upload methods
		void SendBufferToClient(ClientID clientID, Buffer buffer);
		void SendReliableBufferToClient(ClientID clientID, Buffer buffer);

		void SendBufferToAllClients(Buffer buffer);
		void SendReliableBufferToAllClients(Buffer buffer);
		
		// Note: Only std::string has a specialized method, the other will just be the address and size
		template<typename T> void SendToClient(ClientID clientID, const T& data);
		template<typename T> void SendReliableToClient(ClientID clientID, const T& data);
		template<typename T> void SendToAllClients(const T& data);
		template<typename T> void SendReliableToAllClients(const T& data);

		// Setters
		inline void SetUserData(void* userData) { m_User.Data = userData; }
		inline void SetDataReceivedCallback(const DataReceivedCallbackFn& function) { m_User.DataReceivedCallback = function; }
		inline void SetClientConnectedCallback(const ClientConnectedCallbackFn& function) { m_User.ClientConnectedCallback = function; }
		inline void SetClientDisconnectedCallback(const ClientDisconnectedCallbackFn& function) { m_User.ClientDisconnectedCallback = function; }
		inline void SetMessageCallback(const MessageCallbackFn& function) { m_User.MessageCallback = function; }

		// Getters
		inline ServerStatus GetStatus() const { return m_Status; }
		inline bool IsUp() const { return (m_Status == ServerStatus::Up); }

		inline std::map<ClientID, ClientInfo> GetConnectedClients() const { return m_ConnectedClients; }

	private:
		// Private methods
		void Thread(uint16_t port, uint64_t pollingRateMs);

		// Polling methods
		void PollIncomingMessages();
		void PollConnectionStateChanges();

		// Upload method
		SendResult SendBuffer(ClientID clientID, Buffer buffer, int network);

		// Static callbacks
		friend void Server_ConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* info);

	private:
		std::thread m_NetworkThread;

		struct
		{
			void* Data;
			DataReceivedCallbackFn DataReceivedCallback;
			ClientConnectedCallbackFn ClientConnectedCallback;
			ClientDisconnectedCallbackFn ClientDisconnectedCallback;
			MessageCallbackFn MessageCallback;
		} m_User;

		ServerStatus m_Status = ServerStatus::Down;

		std::map<ClientID, ClientInfo> m_ConnectedClients;

		ISteamNetworkingSockets* m_Interface = nullptr;
		HSteamListenSocket m_ListenSocket = 0u;
		HSteamNetPollGroup m_PollGroup = 0u;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// Templated methods
	////////////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline void Server::SendToClient(ClientID clientID, const T& data)
	{
		SendBuffer(clientID, Buffer(&data, sizeof(T)), k_nSteamNetworkingSend_Unreliable);
	}

	template<typename T>
	inline void Server::SendReliableToClient(ClientID clientID, const T& data)
	{
		SendBuffer(clientID, Buffer(&data, sizeof(T)), k_nSteamNetworkingSend_Reliable);
	}

	template<typename T>
	inline void Server::SendToAllClients(const T& data)
	{
		for (const auto& [clientID, _] : m_ConnectedClients)
			SendBuffer(clientID, Buffer(&data, sizeof(T)), k_nSteamNetworkingSend_Unreliable);
	}

	template<typename T>
	inline void Server::SendReliableToAllClients(const T& data)
	{
		for (const auto& [clientID, _] : m_ConnectedClients)
			SendBuffer(clientID, Buffer(&data, sizeof(T)), k_nSteamNetworkingSend_Reliable);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Specialized methods
	////////////////////////////////////////////////////////////////////////////////////
	template<>
	inline void Server::SendToClient<std::string>(ClientID clientID, const std::string& str)
	{
		SendBuffer(clientID, Buffer(str.data(), str.size()), k_nSteamNetworkingSend_Unreliable);
	}

	template<>
	inline void Server::SendReliableToClient<std::string>(ClientID clientID, const std::string& str)
	{
		SendBuffer(clientID, Buffer(str.data(), str.size()), k_nSteamNetworkingSend_Reliable);
	}

	template<>
	inline void Server::SendToAllClients<std::string>(const std::string& str)
	{
		for (const auto& [clientID, _] : m_ConnectedClients)
			SendBuffer(clientID, Buffer(str.data(), str.size()), k_nSteamNetworkingSend_Unreliable);
	}

	template<>
	inline void Server::SendReliableToAllClients<std::string>(const std::string& str)
	{
		for (const auto& [clientID, _] : m_ConnectedClients)
			SendBuffer(clientID, Buffer(str.data(), str.size()), k_nSteamNetworkingSend_Reliable);
	}

}