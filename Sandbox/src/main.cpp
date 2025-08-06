// Note: So far all of this code has been stolen from TheCherno/Walnut to test out the functionality of GameNetworkingSockets

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <functional>
#include <memory>
#include <format>
#include <string.h>

#define STEAMNETWORKINGSOCKETS_STATIC_LINK
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#if defined(NN_PLATFORM_WINDOWS)
	#include <WinSock2.h>
	#include <ws2tcpip.h>
#else
	// TODO: Linux/MacOS
#endif

namespace Nano::Networking
{

	struct Buffer
	{
		void* Data;
		uint64_t Size;

		Buffer()
			: Data(nullptr), Size(0)
		{
		}

		Buffer(const void* data, uint64_t size)
			: Data((void*)data), Size(size)
		{
		}

		Buffer(const Buffer& other, uint64_t size)
			: Data(other.Data), Size(size)
		{
		}

		static Buffer Copy(const Buffer& other)
		{
			Buffer buffer;
			buffer.Allocate(other.Size);
			memcpy(buffer.Data, other.Data, other.Size);
			return buffer;
		}

		static Buffer Copy(const void* data, uint64_t size)
		{
			Buffer buffer;
			buffer.Allocate(size);
			memcpy(buffer.Data, data, size);
			return buffer;
		}

		void Allocate(uint64_t size)
		{
			delete[](uint8_t*)Data;
			Data = nullptr;

			if (size == 0)
				return;

			Data = new uint8_t[size];
			Size = size;
		}

		void Release()
		{
			delete[](uint8_t*)Data;
			Data = nullptr;
			Size = 0;
		}

		void ZeroInitialize()
		{
			if (Data)
				memset(Data, 0, Size);
		}

		template<typename T>
		T& Read(uint64_t offset = 0)
		{
			return *(T*)((uint32_t*)Data + offset);
		}

		template<typename T>
		const T& Read(uint64_t offset = 0) const
		{
			return *(T*)((uint32_t*)Data + offset);
		}

		uint8_t* ReadBytes(uint64_t size, uint64_t offset) const
		{
			//WL_CORE_ASSERT(offset + size <= Size, "Buffer overflow!");
			uint8_t* buffer = new uint8_t[size];
			memcpy(buffer, (uint8_t*)Data + offset, size);
			return buffer;
		}

		void Write(const void* data, uint64_t size, uint64_t offset = 0)
		{
			//WL_CORE_ASSERT(offset + size <= Size, "Buffer overflow!");
			memcpy((uint8_t*)Data + offset, data, size);
		}

		operator bool() const
		{
			return Data;
		}

		uint8_t& operator[](int index)
		{
			return ((uint8_t*)Data)[index];
		}

		uint8_t operator[](int index) const
		{
			return ((uint8_t*)Data)[index];
		}

		template<typename T>
		T* As() const
		{
			return (T*)Data;
		}

		inline uint64_t GetSize() const { return Size; }
	};

}

namespace Nano::Networking::Utils 
{

	std::vector<std::string> SplitString(const std::string_view string, const std::string_view& delimiters)
	{
		size_t first = 0;

		std::vector<std::string> result;

		while (first <= string.size())
		{
			const auto second = string.find_first_of(delimiters, first);

			if (first != second)
				result.emplace_back(string.substr(first, second - first));

			if (second == std::string_view::npos)
				break;

			first = second + 1;
		}

		return result;
	}

	std::vector<std::string> SplitString(const std::string_view string, const char delimiter)
	{
		return SplitString(string, std::string(1, delimiter));
	}

	bool IsValidIPAddress(std::string_view ipAddress)
	{
		std::string ipAddressStr(ipAddress.data(), ipAddress.size());

		SteamNetworkingIPAddr address;
		return address.ParseString(ipAddressStr.c_str());
	}

	// Platform-specific implementations
	std::string ResolveDomainName(std::string_view name)
	{
#if defined(NN_PLATFORM_WINDOWS)
		// Adapted from example at https://learn.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfo
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
		{
			printf("WSAStartup failed with %u\n", WSAGetLastError());
			return {};
		}

		bool hasPort = name.find(":") != std::string::npos;
		std::string domain, port;
		if (hasPort)
		{
			std::vector<std::string> domainAndPort = SplitString(name, ':');
			if (domainAndPort.size() != 2)
				return {};
			domain = domainAndPort[0];
			port = domainAndPort[1];
			name = domain;
		}

		addrinfo hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		addrinfo* addressResult = NULL;
		DWORD dwRetval = getaddrinfo(name.data(), nullptr, &hints, &addressResult);
		if (dwRetval != 0)
		{
			printf("getaddrinfo failed with error: %d\n", dwRetval);
			WSACleanup();
			return {};
		}

		std::string ipAddressStr;
		for (addrinfo* ptr = addressResult; ptr != NULL; ptr = ptr->ai_next)
		{
			switch (ptr->ai_family)
			{
			case AF_UNSPEC:
				// Unspecified
				break;
			case AF_INET:
			{
				sockaddr_in* sockaddr_ipv4 = (sockaddr_in*)ptr->ai_addr;
				char* ipAddress = inet_ntoa(sockaddr_ipv4->sin_addr);
				ipAddressStr = ipAddress;
				break;
			}
			case AF_INET6:
			{
				const DWORD ipbufferlength = 46;
				char ipstringbuffer[ipbufferlength];
				DWORD actualIPBufferLength = ipbufferlength;
				LPSOCKADDR sockaddr_ip = (LPSOCKADDR)ptr->ai_addr;
				INT iRetval = WSAAddressToStringA(sockaddr_ip, (DWORD)ptr->ai_addrlen, nullptr, ipstringbuffer, &actualIPBufferLength);

				if (iRetval)
				{
					printf("WSAAddressToString failed with %u\n", WSAGetLastError());
					WSACleanup();
					return {};
				}

				ipAddressStr = std::string(ipstringbuffer, actualIPBufferLength);
			}
			}

		}

		freeaddrinfo(addressResult);
		WSACleanup();

		return hasPort ? (ipAddressStr + ":" + port) : ipAddressStr;
#else
		bool hasPort = name.find(":") != std::string::npos;
		std::string domain, port;
		if (hasPort)
		{
			std::vector<std::string> domainAndPort = SplitString(name, ':');
			if (domainAndPort.size() != 2)
				return {};
			domain = domainAndPort[0];
			port = domainAndPort[1];
			name = domain;
		}

		addrinfo hints{};
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		addrinfo* res = nullptr;
		int err = getaddrinfo(name.data(), nullptr, &hints, &res);
		if (err != 0)
		{
			std::cerr << "getaddrinfo failed: " << gai_strerror(err) << "\n";
			return {};
		}

		std::string ipAddressStr;
		for (addrinfo* ptr = res; ptr != nullptr; ptr = ptr->ai_next)
		{
			char ipstr[INET6_ADDRSTRLEN] = {};

			if (ptr->ai_family == AF_INET)
			{
				sockaddr_in* ipv4 = reinterpret_cast<sockaddr_in*>(ptr->ai_addr);
				inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
				ipAddressStr = ipstr;
				break; // Prefer IPv4
			}
			else if (ptr->ai_family == AF_INET6)
			{
				sockaddr_in6* ipv6 = reinterpret_cast<sockaddr_in6*>(ptr->ai_addr);
				inet_ntop(AF_INET6, &(ipv6->sin6_addr), ipstr, sizeof(ipstr));
				ipAddressStr = ipstr;
				// Don't break — prefer IPv4 if available, but fallback to IPv6
			}
		}

		freeaddrinfo(res);

		return hasPort ? (ipAddressStr + ":" + port) : ipAddressStr;
#endif
	}

}

namespace Nano::Networking 
{

	class Client
	{
	public:
		enum class ConnectionStatus
		{
			Disconnected = 0, Connected, Connecting, FailedToConnect
		};
	public:
		using DataReceivedCallback = std::function<void(const Buffer)>;
		using ServerConnectedCallback = std::function<void()>;
		using ServerDisconnectedCallback = std::function<void()>;
	public:
		Client() = default;
		~Client()
		{
			if (m_NetworkThread.joinable())
				m_NetworkThread.join();
		}

		void ConnectToServer(const std::string& serverAddress)
		{
			if (m_Running)
				return;

			if (m_NetworkThread.joinable())
				m_NetworkThread.join();

			m_ServerAddress = serverAddress;
			m_NetworkThread = std::thread([this]() { NetworkThreadFunc(); });
		}

		void Disconnect()
		{
			m_Running = false;

			if (m_NetworkThread.joinable())
				m_NetworkThread.join();
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Set callbacks for server events
		// These callbacks will be called from the network thread
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void SetDataReceivedCallback(const DataReceivedCallback& function) { m_DataReceivedCallback = function; }
		void SetServerConnectedCallback(const ServerConnectedCallback& function) { m_ServerConnectedCallback = function; }
		void SetServerDisconnectedCallback(const ServerDisconnectedCallback& function) { m_ServerDisconnectedCallback = function; }

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Send Data
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void SendBuffer(Buffer buffer, bool reliable = true)
		{
			EResult result = m_Interface->SendMessageToConnection(m_Connection, buffer.Data, (uint32_t)buffer.Size, reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable, nullptr);
			// handle result?
		}

		void SendString(const std::string& string, bool reliable = true)
		{
			SendBuffer(Buffer(string.data(), string.size()), reliable);
		}

		template<typename T>
		void SendData(const T& data, bool reliable = true)
		{
			SendBuffer(Buffer(&data, sizeof(T)), reliable);
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Connection Status & Debugging
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		bool IsRunning() const { return m_Running; }
		ConnectionStatus GetConnectionStatus() const { return m_ConnectionStatus; }
		const std::string& GetConnectionDebugMessage() const { return m_ConnectionDebugMessage; }

		uint32_t GetID() const { return m_Connection; }

	private:
		void NetworkThreadFunc()
		{
			s_Instance = this;

			// Reset connection status
			m_ConnectionStatus = ConnectionStatus::Connecting;

			SteamDatagramErrMsg errMsg;
			if (!GameNetworkingSockets_Init(nullptr, errMsg))
			{
				m_ConnectionDebugMessage = "Could not initialize GameNetworkingSockets";
				m_ConnectionStatus = ConnectionStatus::FailedToConnect;
				return;
			}

			// Select instance to use.  For now we'll always use the default.
			m_Interface = SteamNetworkingSockets();

			if (Utils::IsValidIPAddress(m_ServerAddress))
				m_ServerIPAddress = m_ServerAddress;
			else
				m_ServerIPAddress = Utils::ResolveDomainName(m_ServerAddress);

			// Start connecting
			SteamNetworkingIPAddr address;
			if (!address.ParseString(m_ServerIPAddress.c_str()))
			{
				OnFatalError(std::format("Invalid IP address - could not parse {}", m_ServerIPAddress));
				m_ConnectionDebugMessage = "Invalid IP address";
				m_ConnectionStatus = ConnectionStatus::FailedToConnect;
				return;
			}

			SteamNetworkingConfigValue_t options;
			options.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)ConnectionStatusChangedCallback);
			m_Connection = m_Interface->ConnectByIPAddress(address, 1, &options);
			if (m_Connection == k_HSteamNetConnection_Invalid)
			{
				m_ConnectionDebugMessage = "Failed to create connection";
				m_ConnectionStatus = ConnectionStatus::FailedToConnect;
				return;
			}

			m_Running = true;
			while (m_Running)
			{
				PollIncomingMessages();
				PollConnectionStateChanges();
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}

			m_Interface->CloseConnection(m_Connection, 0, nullptr, false);
			m_ConnectionStatus = ConnectionStatus::Disconnected;
			GameNetworkingSockets_Kill();
		}

		void Shutdown()
		{
			m_Running = false;
		}

	private:
		static void ConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* info)
		{
			s_Instance->OnConnectionStatusChanged(info);
		}

		void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info)
		{
			//assert(pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid);

		// Handle connection state
			switch (info->m_info.m_eState)
			{
			case k_ESteamNetworkingConnectionState_None:
				// NOTE: We will get callbacks here when we destroy connections. You can ignore these.
				break;

			case k_ESteamNetworkingConnectionState_ClosedByPeer:
			case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
			{
				m_Running = false;
				m_ConnectionStatus = ConnectionStatus::FailedToConnect;
				m_ConnectionDebugMessage = info->m_info.m_szEndDebug;

				// Print an appropriate message
				if (info->m_eOldState == k_ESteamNetworkingConnectionState_Connecting)
				{
					// Note: we could distinguish between a timeout, a rejected connection,
					// or some other transport problem.
					std::cout << "Could not connect to remove host. " << info->m_info.m_szEndDebug << std::endl;
				}
				else if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
				{
					std::cout << "Lost connection with remote host. " << info->m_info.m_szEndDebug << std::endl;
				}
				else
				{
					// NOTE: We could check the reason code for a normal disconnection
					std::cout << "Disconnected from host. " << info->m_info.m_szEndDebug << std::endl;
				}

				// Clean up the connection.  This is important!
				// The connection is "closed" in the network sense, but
				// it has not been destroyed.  We must close it on our end, too
				// to finish up.  The reason information do not matter in this case,
				// and we cannot linger because it's already closed on the other end,
				// so we just pass 0s.
				m_Interface->CloseConnection(info->m_hConn, 0, nullptr, false);
				m_Connection = k_HSteamNetConnection_Invalid;
				m_ConnectionStatus = ConnectionStatus::Disconnected;
				break;
			}

			case k_ESteamNetworkingConnectionState_Connecting:
				// We will get this callback when we start connecting.
				// We can ignore this.
				break;

			case k_ESteamNetworkingConnectionState_Connected:
				m_ConnectionStatus = ConnectionStatus::Connected;
				if (m_ServerConnectedCallback)
					m_ServerConnectedCallback();
				break;

			default:
				break;
			}
		}

		void PollIncomingMessages()
		{
			// Process all messages
			while (m_Running)
			{
				ISteamNetworkingMessage* incomingMessage = nullptr;
				int messageCount = m_Interface->ReceiveMessagesOnConnection(m_Connection, &incomingMessage, 1);
				if (messageCount == 0)
					break;

				if (messageCount < 0)
				{
					// messageCount < 0 means critical error?
					m_Running = false;
					return;
				}

				m_DataReceivedCallback(Buffer(incomingMessage->m_pData, incomingMessage->m_cbSize));

				// Release when done
				incomingMessage->Release();
			}
		}

		void PollConnectionStateChanges()
		{
			m_Interface->RunCallbacks();
		}

		void OnFatalError(const std::string& message)
		{
			std::cout << message << std::endl;
			m_Running = false;
		}

	private:
		std::thread m_NetworkThread;
		DataReceivedCallback m_DataReceivedCallback;
		ServerConnectedCallback m_ServerConnectedCallback;
		ServerDisconnectedCallback m_ServerDisconnectedCallback;

		ConnectionStatus m_ConnectionStatus = ConnectionStatus::Disconnected;
		std::string m_ConnectionDebugMessage;

		std::string m_ServerAddress, m_ServerIPAddress;
		bool m_Running = false;

		ISteamNetworkingSockets* m_Interface = nullptr;
		HSteamNetConnection m_Connection = 0;

		inline static Client* s_Instance = nullptr;
	};

}

namespace Nano::Networking
{

	using ClientID = HSteamNetConnection;

	struct ClientInfo
	{
		ClientID ID;
		std::string ConnectionDesc;
	};

	class Server
	{
	public:
		using DataReceivedCallback = std::function<void(const ClientInfo&, const Buffer)>;
		using ClientConnectedCallback = std::function<void(const ClientInfo&)>;
		using ClientDisconnectedCallback = std::function<void(const ClientInfo&)>;
	public:
		Server(int port)
			: m_Port(port)
		{
		}

		~Server()
		{
			if (m_NetworkThread.joinable())
				m_NetworkThread.join();
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Start and Stop the server
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void Start()
		{
			if (m_Running)
				return;

			m_NetworkThread = std::thread([this]() { NetworkThreadFunc(); });
		}

		void Stop()
		{
			m_Running = false;
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Set callbacks for server events
		// These callbacks will be called from the server thread
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void SetDataReceivedCallback(const DataReceivedCallback& function) { m_DataReceivedCallback = function; }
		void SetClientConnectedCallback(const ClientConnectedCallback& function) { m_ClientConnectedCallback = function; }
		void SetClientDisconnectedCallback(const ClientDisconnectedCallback& function) { m_ClientDisconnectedCallback = function; }

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Send Data
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void SendBufferToClient(ClientID clientID, Buffer buffer, bool reliable = true)
		{
			m_Interface->SendMessageToConnection((HSteamNetConnection)clientID, buffer.Data, (ClientID)buffer.Size, reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable, nullptr);
		}

		void SendBufferToAllClients(Buffer buffer, ClientID excludeClientID = 0, bool reliable = true)
		{
			for (const auto& [clientID, clientInfo] : m_ConnectedClients)
			{
				if (clientID != excludeClientID)
					SendBufferToClient(clientID, buffer, reliable);
			}
		}

		void SendStringToClient(ClientID clientID, const std::string& string, bool reliable = true)
		{
			SendBufferToClient(clientID, Buffer(string.data(), string.size()), reliable);
		}

		void SendStringToAllClients(const std::string& string, ClientID excludeClientID = 0, bool reliable = true)
		{
			SendBufferToAllClients(Buffer(string.data(), string.size()), excludeClientID, reliable);
		}

		template<typename T>
		void SendDataToClient(ClientID clientID, const T& data, bool reliable = true)
		{
			SendBufferToClient(clientID, Buffer(&data, sizeof(T)), reliable);
		}

		template<typename T>
		void SendDataToAllClients(const T& data, ClientID excludeClientID = 0, bool reliable = true)
		{
			SendBufferToAllClients(Buffer(&data, sizeof(T)), excludeClientID, reliable);
		}
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		void KickClient(ClientID clientID)
		{
			m_Interface->CloseConnection(clientID, 0, "Kicked by host", false);
		}

		bool IsRunning() const { return m_Running; }
		const std::map<HSteamNetConnection, ClientInfo>& GetConnectedClients() const { return m_ConnectedClients; }

	private:
		void NetworkThreadFunc()
		{
			s_Instance = this;
			m_Running = true;

			SteamDatagramErrMsg errMsg;
			if (!GameNetworkingSockets_Init(nullptr, errMsg))
			{
				OnFatalError(std::format("GameNetworkingSockets_Init failed: {}", errMsg));
				return;
			}

			m_Interface = SteamNetworkingSockets();

			// Start listening
			SteamNetworkingIPAddr serverLocalAddress;
			serverLocalAddress.Clear();
			serverLocalAddress.m_port = m_Port;

			SteamNetworkingConfigValue_t options;
			options.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)Server::ConnectionStatusChangedCallback);

			// Try to start listen socket on port
			m_ListenSocket = m_Interface->CreateListenSocketIP(serverLocalAddress, 1, &options);

			if (m_ListenSocket == k_HSteamListenSocket_Invalid)
			{
				OnFatalError(std::format("Fatal error: Failed to listen on port {}", m_Port));
				return;
			}

			// Try to create poll group
			// TODO(Yan): should be optional, though good for groups which is probably the most common use case
			m_PollGroup = m_Interface->CreatePollGroup();
			if (m_PollGroup == k_HSteamNetPollGroup_Invalid)
			{
				OnFatalError(std::format("Fatal error: Failed to listen on port {}", m_Port));
				return;
			}

			std::cout << "Server listening on port " << m_Port << std::endl;

			while (m_Running)
			{
				PollIncomingMessages();
				PollConnectionStateChanges();
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}

			// Close all the connections
			std::cout << "Closing connections..." << std::endl;
			for (const auto& [clientID, clientInfo] : m_ConnectedClients)
			{
				m_Interface->CloseConnection(clientID, 0, "Server Shutdown", true);
			}

			m_ConnectedClients.clear();

			m_Interface->CloseListenSocket(m_ListenSocket);
			m_ListenSocket = k_HSteamListenSocket_Invalid;

			m_Interface->DestroyPollGroup(m_PollGroup);
			m_PollGroup = k_HSteamNetPollGroup_Invalid;
		}

		static void ConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* info) { s_Instance->OnConnectionStatusChanged(info); }
		void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* status)
		{
			// Handle connection state
			switch (status->m_info.m_eState)
			{
			case k_ESteamNetworkingConnectionState_None:
				// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
				break;

			case k_ESteamNetworkingConnectionState_ClosedByPeer:
			case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
			{
				// Ignore if they were not previously connected.  (If they disconnected
				// before we accepted the connection.)
				if (status->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
				{
					// Locate the client.  Note that it should have been found, because this
					// is the only codepath where we remove clients (except on shutdown),
					// and connection change callbacks are dispatched in queue order.
					auto itClient = m_ConnectedClients.find(status->m_hConn);
					//assert(itClient != m_mapClients.end());

					// Either ClosedByPeer or ProblemDetectedLocally - should be communicated to user callback
					// User callback
					if (m_ClientDisconnectedCallback)
						m_ClientDisconnectedCallback(itClient->second);

					m_ConnectedClients.erase(itClient);
				}
				else
				{
					//assert(info->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
				}

				// Clean up the connection.  This is important!
				// The connection is "closed" in the network sense, but
				// it has not been destroyed.  We must close it on our end, too
				// to finish up.  The reason information do not matter in this case,
				// and we cannot linger because it's already closed on the other end,
				// so we just pass 0s.
				m_Interface->CloseConnection(status->m_hConn, 0, nullptr, false);
				break;
			}

			case k_ESteamNetworkingConnectionState_Connecting:
			{
				// This must be a new connection
				// assert(m_mapClients.find(info->m_hConn) == m_mapClients.end());

				// Try to accept incoming connection
				if (m_Interface->AcceptConnection(status->m_hConn) != k_EResultOK)
				{
					m_Interface->CloseConnection(status->m_hConn, 0, nullptr, false);
					std::cout << "Couldn't accept connection (it was already closed?)" << std::endl;
					break;
				}

				// Assign the poll group
				if (!m_Interface->SetConnectionPollGroup(status->m_hConn, m_PollGroup))
				{
					m_Interface->CloseConnection(status->m_hConn, 0, nullptr, false);
					std::cout << "Failed to set poll group" << std::endl;
					break;
				}

				// Retrieve connection info
				SteamNetConnectionInfo_t connectionInfo;
				m_Interface->GetConnectionInfo(status->m_hConn, &connectionInfo);

				// Register connected client
				auto& client = m_ConnectedClients[status->m_hConn];
				client.ID = (ClientID)status->m_hConn;
				client.ConnectionDesc = connectionInfo.m_szConnectionDescription;

				// User callback
				if (m_ClientConnectedCallback)
					m_ClientConnectedCallback(client);

				break;
			}

			case k_ESteamNetworkingConnectionState_Connected:
				// We will get a callback immediately after accepting the connection.
				// Since we are the server, we can ignore this, it's not news to us.
				break;

			default:
				break;
			}
		}

		// Server functionality
		void PollIncomingMessages()
		{
			// Process all messages
			while (m_Running)
			{
				ISteamNetworkingMessage* incomingMessage = nullptr;
				int messageCount = m_Interface->ReceiveMessagesOnPollGroup(m_PollGroup, &incomingMessage, 1);
				if (messageCount == 0)
					break;

				if (messageCount < 0)
				{
					// messageCount < 0 means critical error?
					m_Running = false;
					return;
				}

				// assert(numMsgs == 1 && pIncomingMsg);

				auto itClient = m_ConnectedClients.find(incomingMessage->m_conn);
				if (itClient == m_ConnectedClients.end())
				{
					std::cout << "ERROR: Received data from unregistered client\n";
					continue;
				}

				if (incomingMessage->m_cbSize)
				{
					if (m_DataReceivedCallback)
						m_DataReceivedCallback(itClient->second, Buffer(incomingMessage->m_pData, incomingMessage->m_cbSize));
				}

				// Release when done
				incomingMessage->Release();
			}
		}

		void SetClientNick(HSteamNetConnection hConn, const char* nick)
		{
			// Set the connection name, too, which is useful for debugging
			m_Interface->SetConnectionName(hConn, nick);
		}

		void PollConnectionStateChanges()
		{
			m_Interface->RunCallbacks();
		}

		void OnFatalError(const std::string& message)
		{
			std::cout << message << std::endl;
			m_Running = false;
		}

	private:
		std::thread m_NetworkThread;
		DataReceivedCallback m_DataReceivedCallback;
		ClientConnectedCallback m_ClientConnectedCallback;
		ClientDisconnectedCallback m_ClientDisconnectedCallback;

		int m_Port = 0;
		bool m_Running = false;
		std::map<HSteamNetConnection, ClientInfo> m_ConnectedClients;

		ISteamNetworkingSockets* m_Interface = nullptr;
		HSteamListenSocket m_ListenSocket = 0u;
		HSteamNetPollGroup m_PollGroup = 0u;

		inline static Server* s_Instance = nullptr;
	};

}

#define NN_SERVER 0
#define NN_CLIENT (!NN_SERVER)

using namespace Nano::Networking;

#if NN_CLIENT
int main(int argc, char* argv[])
{
	(void)argc; (void)argv;

	Client client;
	client.ConnectToServer("127.0.0.1:8000");

	while (true)
	{
	}

	client.Disconnect();

	return 0;
}
#elif NN_SERVER
int main(int argc, char* argv[])
{
	(void)argc; (void)argv;

	Server server(8000);
	server.Start();

	server.SetClientConnectedCallback([&](const ClientInfo& info)
	{
		std::cout << "Client connected: " << info.ConnectionDesc << std::endl;
	});
	server.SetClientDisconnectedCallback([&](const ClientInfo& info)
	{
		std::cout << "Client disconnected: " << info.ConnectionDesc << std::endl;
	});

	while (true)
	{
		for (auto client : server.GetConnectedClients())
		{
			std::cout << "Still connected client: " << client.second.ConnectionDesc << std::endl;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	}

	server.Stop();
	return 0;
}
#else
	#error Invalid configuration
#endif