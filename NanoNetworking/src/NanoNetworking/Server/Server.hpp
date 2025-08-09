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

		// Setters
		inline void SetUserData(void* userData) { m_User.Data = userData; }
		inline void SetDataReceivedCallback(const DataReceivedCallbackFn& function) { m_User.DataReceivedCallback = function; }
		inline void SetClientConnectedCallback(const ClientConnectedCallbackFn& function) { m_User.ClientConnectedCallback = function; }
		inline void SetClientDisconnectedCallback(const ClientDisconnectedCallbackFn& function) { m_User.ClientDisconnectedCallback = function; }
		inline void SetMessageCallback(const MessageCallbackFn& function) { m_User.MessageCallback = function; }

		// Getters
		inline ServerStatus GetStatus() const { return m_Status; }
		inline bool IsUp() const { return (m_Status == ServerStatus::Up); }

	private:
		// Private methods
		void Thread(uint16_t port, uint64_t pollingRateMs);

		// Polling methods
		void PollIncomingMessages();
		void PollConnectionStateChanges();

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

		std::map<HSteamNetConnection, ClientInfo> m_ConnectedClients;

		ISteamNetworkingSockets* m_Interface = nullptr;
		HSteamListenSocket m_ListenSocket = 0u;
		HSteamNetPollGroup m_PollGroup = 0u;
	};

}