#include "nnpch.h"
#include "Server.hpp"

#include "NanoNetworking/Core/Logging.hpp"
#include "NanoNetworking/Core/Utils.hpp"

#include <unordered_map>

namespace Nano::Networking
{

	static std::unordered_map<HSteamListenSocket, Server*> s_SocketToServer = {};

	////////////////////////////////////////////////////////////////////////////////////
	// Helper method
	////////////////////////////////////////////////////////////////////////////////////
	template<typename TFunc, typename ...TArgs>
	auto CallCallback(TFunc&& func, TArgs&& ...args)
	{
		if (func)
			return func(std::forward<TArgs>(args)...);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	////////////////////////////////////////////////////////////////////////////////////
	void ConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* statusInfo)
	{
		Server& server = *s_SocketToServer[statusInfo->m_info.m_hListenSocket];

		switch (statusInfo->m_info.m_eState)
		{
		// Note: These are not helpful or already gotten by a different function
		case k_ESteamNetworkingConnectionState_None:
		case k_ESteamNetworkingConnectionState_Connected:
			break;

		// Connection closed
		case k_ESteamNetworkingConnectionState_ClosedByPeer:
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		{
			if (statusInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
			{
				// Note: This must be valid, since it was previously connected
				auto itClient = server.m_ConnectedClients.find(statusInfo->m_hConn);

				CallCallback(server.m_User.ClientDisconnectedCallback, server.m_User.Data, itClient->second);
				server.m_ConnectedClients.erase(itClient);
			}
			
			server.m_Interface->CloseConnection(statusInfo->m_hConn, 0, nullptr, false);
			break;
		}

		// Connecting user
		case k_ESteamNetworkingConnectionState_Connecting:
		{
			// Try to accept incoming connection
			if (server.m_Interface->AcceptConnection(statusInfo->m_hConn) != k_EResultOK)
			{
				server.m_Interface->CloseConnection(statusInfo->m_hConn, 0, nullptr, false);
				CallCallback(server.m_User.MessageCallback, server.m_User.Data, MessageType::Warn, "Couldn't accept connection (it was already closed?)");
				break;
			}

			// Assign the poll group
			if (!server.m_Interface->SetConnectionPollGroup(statusInfo->m_hConn, server.m_PollGroup))
			{
				server.m_Interface->CloseConnection(statusInfo->m_hConn, 0, nullptr, false);
				CallCallback(server.m_User.MessageCallback, server.m_User.Data, MessageType::Error, "Failed to set poll group");
				break;
			}

			// Retrieve connection info
			SteamNetConnectionInfo_t connectionInfo;
			server.m_Interface->GetConnectionInfo(statusInfo->m_hConn, &connectionInfo);

			// Register connected client
			auto& client = server.m_ConnectedClients[statusInfo->m_hConn];
			client.ID = static_cast<ClientID>(statusInfo->m_hConn);
			client.ConnectionDesc = connectionInfo.m_szConnectionDescription;

			CallCallback(server.m_User.ClientConnectedCallback, server.m_User.Data, client);
			break;
		}

		default:
			break;
		}
	}

}

namespace Nano::Networking
{

	////////////////////////////////////////////////////////////////////////////////////
	// Constructor
	////////////////////////////////////////////////////////////////////////////////////
	Server::Server(void* userData, DataReceivedCallbackFn dataReceivedCallback, ClientConnectedCallbackFn serverConnectedCallback, ClientDisconnectedCallbackFn serverDisconnectedCallback, MessageCallbackFn messageCallback)
		: m_User(userData, dataReceivedCallback, serverConnectedCallback, serverDisconnectedCallback, messageCallback)
	{
	}

	Server::~Server()
	{
		if (m_NetworkThread.joinable())
			m_NetworkThread.join();
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////////////////
	void Server::Start(uint16_t port, uint64_t pollingRateMs)
	{
		m_NetworkThread = std::thread([this, port, pollingRateMs]() { Thread(port, pollingRateMs); });
	}

	void Server::Stop()
	{
		m_Status = ServerStatus::Down;
	}

	void Server::KickClient(ClientID clientID, const std::string& reason)
	{
		m_Interface->CloseConnection(static_cast<HSteamNetConnection>(clientID), 0, reason.c_str(), false);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Private methods
	////////////////////////////////////////////////////////////////////////////////////
	void Server::Thread(uint16_t port, uint64_t pollingRateMs)
	{
		// Initialize
		{
			m_Status = ServerStatus::Initializing;

			SteamDatagramErrMsg errMsg;
			if (!GameNetworkingSockets_Init(nullptr, errMsg))
			{
				m_Status = ServerStatus::FailedToInitialize;
				return;
			}

			m_Interface = SteamNetworkingSockets(); // This is the default for most use cases, use SteamGameServerNetworkingSockets() when using steam API and stuff..
		}

		// Start listening
		{

			SteamNetworkingIPAddr serverLocalAddress = {};
			serverLocalAddress.Clear();
			serverLocalAddress.m_port = port;

			SteamNetworkingConfigValue_t options = {};
			options.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, static_cast<void*>(ConnectionStatusChangedCallback));

			m_ListenSocket = m_Interface->CreateListenSocketIP(serverLocalAddress, 1, &options);

			if (m_ListenSocket == k_HSteamListenSocket_Invalid)
			{
				m_Status = ServerStatus::FailedToListen;
				CallCallback(m_User.MessageCallback, m_User.Data, MessageType::Error, std::format("Failed to listen on port: {0}", port));
				return;
			}
		}

		// Create poll group
		{
			m_PollGroup = m_Interface->CreatePollGroup();
			if (m_PollGroup == k_HSteamNetPollGroup_Invalid)
			{
				m_Status = ServerStatus::FailedToListen;
				CallCallback(m_User.MessageCallback, m_User.Data, MessageType::Error, std::format("Failed to listen on port: {0}", port));
				return;
			}

			CallCallback(m_User.MessageCallback, m_User.Data, MessageType::Info, std::format("Server listening on port: {0}", port));
		}

		s_SocketToServer[m_ListenSocket] = this;

		// Poll message while connecting
		while (m_Status == ServerStatus::Initializing)
		{
			PollIncomingMessages();
			PollConnectionStateChanges();
			std::this_thread::sleep_for(std::chrono::milliseconds(pollingRateMs));
		}

		// Connected
		//if (m_Status == ServerStatus::Up)
		//	promise.set_value(); // Notify ConnectionInfo that we're connected

		// Poll message while connected
		while (m_Status == ServerStatus::Up)
		{
			PollIncomingMessages();
			PollConnectionStateChanges();
			std::this_thread::sleep_for(std::chrono::milliseconds(pollingRateMs));
		}

		// Close all the connections
		CallCallback(m_User.MessageCallback, m_User.Data, MessageType::Trace, "Closing connections...");
		for (const auto& [clientID, clientInfo] : m_ConnectedClients)
			m_Interface->CloseConnection(clientID, 0, "Server shutdown.", true);

		m_ConnectedClients.clear();

		m_Interface->CloseListenSocket(m_ListenSocket);
		m_ListenSocket = k_HSteamListenSocket_Invalid;

		m_Interface->DestroyPollGroup(m_PollGroup);
		m_PollGroup = k_HSteamNetPollGroup_Invalid;
		s_SocketToServer.erase(m_ListenSocket);
		//m_Status = ServerStatus::Down;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Polling methods
	////////////////////////////////////////////////////////////////////////////////////
	void Server::PollIncomingMessages()
	{
		while (m_Status == ServerStatus::Up)
		{
			ISteamNetworkingMessage* incomingMessage = nullptr;
			int messageCount = m_Interface->ReceiveMessagesOnPollGroup(m_PollGroup, &incomingMessage, 1);
			
			if (messageCount == 0)
				break;
			else if (messageCount == -1)
			{
				CallCallback(m_User.MessageCallback, m_User.Data, MessageType::Fatal, "Invalid connection handle passed in.");
				return;
			}

			auto itClient = m_ConnectedClients.find(incomingMessage->m_conn);
			if (itClient == m_ConnectedClients.end())
			{
				CallCallback(m_User.MessageCallback, m_User.Data, MessageType::Error, "Received data from unregistered client.");
				continue;
			}

			if (incomingMessage->m_cbSize) // If it is an actual message pass to callback
				CallCallback(m_User.DataReceivedCallback, m_User.Data, itClient->second, Buffer(incomingMessage->m_pData, incomingMessage->m_cbSize));
			incomingMessage->Release();
		}
	}

	void Server::PollConnectionStateChanges()
	{
		m_Interface->RunCallbacks();
	}

}