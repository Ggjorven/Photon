#include "nnpch.h"
#include "Client.hpp"

#include "NanoNetworking/Core/Logging.hpp"
#include "NanoNetworking/Core/Utils.hpp"

#include <unordered_map>

namespace Nano::Networking
{

	static std::unordered_map<HSteamNetConnection, Client*> s_ConnectionToClient = {};

	////////////////////////////////////////////////////////////////////////////////////
	// Helper method
	////////////////////////////////////////////////////////////////////////////////////
	template<typename TFunc, typename ...TArgs>
	static auto CallCallback(TFunc&& func, TArgs&& ...args)
	{ 
		if (func)
			return func(std::forward<TArgs>(args)...);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	////////////////////////////////////////////////////////////////////////////////////
	void Client_ConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* statusInfo)
	{
		Client& client = *s_ConnectionToClient[statusInfo->m_hConn];
		ConnectionInfo& info = client.m_Info;
		auto& user = client.m_User;

		switch (statusInfo->m_info.m_eState)
		{
		// Note: These are not helpful, so we ignore
		case k_ESteamNetworkingConnectionState_None:
		case k_ESteamNetworkingConnectionState_Connecting:
			break;

		// Connection closed
		case k_ESteamNetworkingConnectionState_ClosedByPeer:
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		{
			info.Status = ConnectionStatus::FailedToConnect;

			// Print an appropriate message
			if (statusInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting)
				CallCallback(user.MessageCallback, user.Data, MessageType::Error, std::format("Could not connect to remote host. {}", statusInfo->m_info.m_szEndDebug));
			else if (statusInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
				CallCallback(user.MessageCallback, user.Data, MessageType::Error, std::format("Lost connection with remote host. {}", statusInfo->m_info.m_szEndDebug));
			else
				CallCallback(user.MessageCallback, user.Data, MessageType::Error, std::format("Disconnected from host. {}", statusInfo->m_info.m_szEndDebug));

			// Cleanup
			client.m_Interface->CloseConnection(statusInfo->m_hConn, 0, nullptr, false);
			client.m_Connection = k_HSteamNetConnection_Invalid;
			info.Status = ConnectionStatus::Disconnected;
			break;
		}

		// Connection success
		case k_ESteamNetworkingConnectionState_Connected:
		{
			info.Status = ConnectionStatus::Connected;
			CallCallback(user.ServerConnectedCallback, user.Data);
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
	// Constructor & Destructor
	////////////////////////////////////////////////////////////////////////////////////
	Client::Client(void* userData, DataReceivedCallbackFn dataReceivedCallback, ServerConnectedCallbackFn serverConnectedCallback, ServerDisconnectedCallbackFn serverDisconnectedCallback, MessageCallbackFn messageCallback)
		: m_User(userData, dataReceivedCallback, serverConnectedCallback, serverDisconnectedCallback, messageCallback)
	{
	}

	Client::~Client()
	{
		if (m_NetworkThread.joinable()) 
			m_NetworkThread.join();
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Connection methods
	////////////////////////////////////////////////////////////////////////////////////
	const ConnectionInfo& Client::Connect(std::string_view serverAddress, uint64_t pollingRateMs, uint64_t timeoutMs)
	{
		// Resolve IP Address
		{
			if (Utils::IsValidIPAddress(serverAddress))
				m_Info.IpAddress = std::string(serverAddress);
			else
			{
				auto result = Utils::ResolveDomainName(serverAddress);
				if (!result.has_value())
				{
					m_Info.Status = ConnectionStatus::InvalidIP;
					return m_Info;
				}

				m_Info.IpAddress = result.value();
			}
		}

		// Thread
		{
			std::promise<void> promise;
			m_Info.m_ConnectionWaiting = promise.get_future();

			m_NetworkThread = std::thread([this, promise = std::move(promise), pollingRateMs, timeoutMs]() mutable { Thread(m_Info, std::move(promise), pollingRateMs, timeoutMs); });
		}

		return m_Info;
	}

	void Client::Disconnect()
	{
		// Note: The Thread() handles the disconnection
		m_Info.Status = ConnectionStatus::Disconnected;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Upload methods
	////////////////////////////////////////////////////////////////////////////////////
	SendResult Client::SendBufferToConnection(Buffer buffer)
	{
		return SendBuffer(buffer, k_nSteamNetworkingSend_Unreliable);
	}

	SendResult Client::SendReliableBufferToConnection(Buffer buffer)
	{
		return SendBuffer(buffer, k_nSteamNetworkingSend_Reliable);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Private methods
	////////////////////////////////////////////////////////////////////////////////////
	void Client::Thread(ConnectionInfo& info, std::promise<void>&& promise, uint64_t pollingRateMs, uint64_t timeoutMs)
	{
		info.Status = ConnectionStatus::Connecting;
		
		// Initialize
		{
			SteamDatagramErrMsg errMsg;
			if (!GameNetworkingSockets_Init(nullptr, errMsg))
			{
				info.Status = ConnectionStatus::FailedToInitialize;
				promise.set_value();
				return;
			}

			m_Interface = SteamNetworkingSockets(); // This is the default for most use cases, use SteamGameServerNetworkingSockets() when using steam API and stuff..
		}

		// Steam usable IP Address
		SteamNetworkingIPAddr address = {};
		{
			if (!address.ParseString(info.IpAddress.c_str()))
			{
				info.Status = ConnectionStatus::InvalidIP;
				promise.set_value();
				return;
			}
		}

		// Connect
		SteamNetworkingConfigValue_t options = {};
		{
			options.SetInt32(k_ESteamNetworkingConfig_TimeoutInitial, static_cast<int32_t>(timeoutMs));
			options.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, reinterpret_cast<void*>(&Client_ConnectionStatusChangedCallback));
			
			m_Connection = m_Interface->ConnectByIPAddress(address, 1, &options);
			if (m_Connection == k_HSteamNetConnection_Invalid)
			{
				info.Status = ConnectionStatus::FailedToConnect;
				promise.set_value();
				return;
			}
		}

		// Connecting
		s_ConnectionToClient[m_Connection] = this;
		
		// Poll message while connecting
		while (info.Status == ConnectionStatus::Connecting)
		{
			PollIncomingMessages();
			PollConnectionStateChanges();
			std::this_thread::sleep_for(std::chrono::milliseconds(pollingRateMs));
		}
		promise.set_value();

		// Poll message while connected
		while (info.Status == ConnectionStatus::Connected)
		{
			PollIncomingMessages();
			PollConnectionStateChanges();
			std::this_thread::sleep_for(std::chrono::milliseconds(pollingRateMs));
		}

		// Close connection
		m_Interface->CloseConnection(m_Connection, 0, nullptr, false);
		s_ConnectionToClient.erase(m_Connection);
		m_Connection = k_HSteamNetConnection_Invalid;

		GameNetworkingSockets_Kill();
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Polling methods
	////////////////////////////////////////////////////////////////////////////////////
	void Client::PollIncomingMessages()
	{
		while (m_Info.Status == ConnectionStatus::Connected)
		{
			ISteamNetworkingMessage* incomingMessage = nullptr;
			int messageCount = m_Interface->ReceiveMessagesOnConnection(m_Connection, &incomingMessage, 1);

			if (messageCount == 0)
				break;
			else if (messageCount == -1)
			{
				CallCallback(m_User.MessageCallback, m_User.Data, MessageType::Fatal, "Invalid connection handle passed in.");
				return;
			}

			CallCallback(m_User.DataReceivedCallback, m_User.Data, Buffer(incomingMessage->m_pData, incomingMessage->m_cbSize));
			incomingMessage->Release();
		}
	}

	void Client::PollConnectionStateChanges()
	{
		m_Interface->RunCallbacks();
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Upload method
	////////////////////////////////////////////////////////////////////////////////////
	SendResult Client::SendBuffer(Buffer buffer, int network)
	{
		EResult result = m_Interface->SendMessageToConnection(m_Connection, buffer.Data, static_cast<uint32_t>(buffer.Size), network, nullptr);

		if (result == k_EResultOK)
			return SendResult::Okay;

		// Custom results
		// FUTURE TODO: ...

		return SendResult::Failed;
	}

}