// Project specific
#include <Doremi/Core/Include/Network/NetworkManagerClient.hpp>

// Engine
#include <DoremiEngine/Network/Include/NetworkModule.hpp>
#include <DoremiEngine/Configuration/Include/ConfigurationModule.hpp>

// Connections
#include <Doremi/Core/Include/Network/NetworkConnectionsClient.hpp>

// Network messages
#include <Doremi/Core/Include/Network/NetMessages.hpp>
#include <Doremi/Core/Include/Network/NetworkMessagesClient.hpp>

// Events
#include <Doremi/Core/Include/EventHandler/EventHandler.hpp>
#include <Doremi/Core/Include/EventHandler/Events/ChangeMenuState.hpp>

#include <Doremi/Core/Include/MenuClasses/ServerBrowserHandler.hpp>

#include <iostream> // TODOCM remove only debug

namespace Doremi
{
    namespace Core
    {
        NetworkManagerClient::NetworkManagerClient(const DoremiEngine::Core::SharedContext& p_sharedContext)
            : Manager(p_sharedContext, "NetworkManagerClient"),
              m_nextUpdateTimer(0.0f),
              m_updateInterval(1.0f),
              m_masterNextUpdateTimer(0.0f),
              m_masterUpdateInterval(0.5f),
              m_timeoutIntervalConnecting(5.0f),
              m_timeoutIntervalConnected(5.0f),
              m_timeoutIntervalMaster(5.0f),
              m_maxConnectingMessagesPerFrame(10),
              m_maxConnectedMessagesPerFrame(10)

        {
            // Subscribe to events
            EventHandler::GetInstance()->Subscribe(EventType::ChangeMenuState, this);

            // Startup network messages and connection, TODOCM could change position of this
            NetworkMessagesClient::StartupNetworkMessagesClient(p_sharedContext);
            NetworkConnectionsClient::StartupNetworkConnectionsClient(p_sharedContext);
        }

        NetworkManagerClient::~NetworkManagerClient() {}

        void NetworkManagerClient::SetServerIP(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
        {
            NetworkConnectionsClient* t_connections = NetworkConnectionsClient::GetInstance();
            DoremiEngine::Network::NetworkModule& t_networkModule = m_sharedContext.GetNetworkModule();

            // Change IP on connecting socket
            t_connections->m_serverConnection.ConnectingAdress->SetIP(a, b, c, d);
            t_connections->m_serverConnection.ConnectingAdress->ComposeAdress();

            // TODOCM remove this when we get connected socket by message
            // Change IP on connected socket
            t_connections->m_serverConnection.ConnectedAdress->SetIP(a, b, c, d);
            t_connections->m_serverConnection.ConnectedAdress->ComposeAdress();

            // Remove the old sockets
            t_networkModule.DeleteSocket(t_connections->m_serverConnection.ConnectingSocketHandle);
            t_networkModule.DeleteSocket(t_connections->m_serverConnection.ConnectedSocketHandle);

            // Create new socket for unreliable
            t_connections->m_serverConnection.ConnectingSocketHandle = t_networkModule.CreateUnreliableSocket();
        }

        void NetworkManagerClient::Update(double p_dt)
        {
            // Recieve Messages
            ReceiveMessages();

            // Send Messages
            SendMessages(p_dt);

            // Check for timed out connections
            UpdateTimeouts(p_dt);
        }

        void NetworkManagerClient::ReceiveMessages()
        {
            // Receive connecting messages
            ReceiveConnectingMessages();

            // Receive conneted messages
            ReceiveConnectedMessages();

            // Receive messages from master
            ReceiveMasterMessages();
        }

        void NetworkManagerClient::ReceiveConnectingMessages()
        {
            DoremiEngine::Network::NetworkModule& t_networkModule = m_sharedContext.GetNetworkModule();
            NetworkConnectionsClient* t_connections = NetworkConnectionsClient::GetInstance();

            // Get message class
            NetworkMessagesClient* t_netMessages = NetworkMessagesClient::GetInstance();

            // Get connecting socket
            SocketHandle t_serverConnectingSocketHandle = t_connections->m_serverConnection.ConnectingSocketHandle;

            // Create buffer message
            NetMessageServerClientConnectingFromServer t_newMessage = NetMessageServerClientConnectingFromServer();

            // To check how much we received
            uint32_t t_dataSizeReceived = 0;

            // Counter for checking we dont read to much
            uint32_t t_numOfMessages = 0;

            // Receive messages
            // TODOCM not sure if need to send in out adress here
            while(t_networkModule.ReceiveUnreliableData(&t_newMessage, sizeof(t_newMessage), t_serverConnectingSocketHandle,
                                                        t_connections->m_serverConnection.ConnectingAdress, t_dataSizeReceived) &&
                  ++t_numOfMessages < m_maxConnectingMessagesPerFrame)
            {
                // If wrong size of message
                if(t_dataSizeReceived != sizeof(NetMessageServerClientConnectingFromServer))
                {
                    t_newMessage = NetMessageServerClientConnectingFromServer();
                    continue;
                }

                // Convert message to proper
                NetMessageServerClientConnectingFromServer& t_messageConnecting = *reinterpret_cast<NetMessageServerClientConnectingFromServer*>(&t_newMessage);

                // Check ID and interpet
                switch(t_messageConnecting.MessageID)
                {
                    case SendMessageIDToClientFromServer::VERSION_CHECK:
                    {
                        std::cout << "Received version check" << std::endl; // TODOCM remove deubgg
                        t_netMessages->ReceiveVersionCheck(t_messageConnecting);

                        break;
                    }
                    case SendMessageIDToClientFromServer::CONNECT:
                    {
                        std::cout << "Received connect" << std::endl; // TODOCM remove deubgg
                        t_netMessages->ReceiveConnect(t_messageConnecting);

                        break;
                    }
                    case SendMessageIDToClientFromServer::DISCONNECT:
                    {
                        std::cout << "Received disconnect" << std::endl; // TODOCM remove deubgg
                        t_netMessages->ReceiveDisconnect(t_messageConnecting);

                        break;
                    }
                    default:
                    {
                        std::cout << "Some error message received" << std::endl; // TODOCM remove deubgg
                        break;
                    }
                }

                // Reset message
                t_newMessage = NetMessageServerClientConnectingFromServer();
            }
        }

        void NetworkManagerClient::ReceiveConnectedMessages()
        {
            DoremiEngine::Network::NetworkModule& t_networkModule = m_sharedContext.GetNetworkModule();
            NetworkConnectionsClient* t_connections = NetworkConnectionsClient::GetInstance();
            NetworkMessagesClient* t_netMessages = NetworkMessagesClient::GetInstance();

            // TODOCM check if we need to check if we're connected

            // Create buffer message
            NetMessageBuffer t_newMessage = NetMessageBuffer();

            // To check how much we received
            uint32_t t_dataSizeReceived = 0;

            // Counter for checking we dont read to much
            uint32_t t_numOfMessages = 0;

            // Try receive messages
            while(t_networkModule.ReceiveReliableData(&t_newMessage, sizeof(t_newMessage), t_connections->m_serverConnection.ConnectedSocketHandle, t_dataSizeReceived) &&
                  ++t_numOfMessages < m_maxConnectedMessagesPerFrame)
            {
                // If wrong size of message
                if(t_dataSizeReceived != sizeof(NetMessageServerClientConnectedFromServer))
                {
                    t_newMessage = NetMessageBuffer();
                    continue;
                }

                // Convert message to proper
                NetMessageServerClientConnectedFromServer& t_messageConnecting = *reinterpret_cast<NetMessageServerClientConnectedFromServer*>(&t_newMessage);

                // Check ID and interpet
                switch(t_messageConnecting.MessageID)
                {
                    case SendMessageIDToClientFromServer::CONNECTED:
                    {
                        t_netMessages->ReceiveConnected(t_messageConnecting);

                        break;
                    }
                    case SendMessageIDToClientFromServer::LOAD_WORLD:
                    {
                        t_netMessages->ReceiveLoadWorld(t_messageConnecting);

                        break;
                    }
                    case SendMessageIDToClientFromServer::IN_GAME:
                    {
                        t_netMessages->ReceiveInGame(t_messageConnecting);

                        break;
                    }

                    default:
                        break;
                }

                // Reset message
                t_newMessage = NetMessageBuffer();
            }
        }

        void NetworkManagerClient::ReceiveMasterMessages()
        {
            DoremiEngine::Network::NetworkModule& t_networkModule = m_sharedContext.GetNetworkModule();
            NetworkConnectionsClient* t_connections = NetworkConnectionsClient::GetInstance();

            // Get message class
            NetworkMessagesClient* t_netMessages = NetworkMessagesClient::GetInstance();

            // Get connecting socket
            SocketHandle t_masterConnectingSocketHandle = t_connections->m_masterConnection.ConnectedSocketHandle;

            // Create buffer message
            NetMessageMasterClientFromMaster t_newMessage = NetMessageMasterClientFromMaster();

            // To check how much we received
            uint32_t t_dataSizeReceived = 0;

            // Counter for checking we dont read to much
            uint32_t t_numOfMessages = 0;

            // Receive messages
            // TODOCM not sure if need to send in out adress here
            while(t_networkModule.ReceiveUnreliableData(&t_newMessage, sizeof(t_newMessage), t_masterConnectingSocketHandle,
                                                        t_connections->m_masterConnection.Adress, t_dataSizeReceived) &&
                  ++t_numOfMessages < m_maxConnectingMessagesPerFrame)
            {
                // If wrong size of message
                if(t_dataSizeReceived != sizeof(NetMessageMasterClientFromMaster))
                {
                    t_newMessage = NetMessageMasterClientFromMaster();
                    continue;
                }

                // Check ID and interpet
                switch(t_newMessage.MessageID)
                {
                    case SendMessageIDToClientFromMaster::CONNECTED:
                    {
                        t_netMessages->ReceiveConnectedMaster(t_newMessage);

                        break;
                    }
                    case SendMessageIDToClientFromMaster::DISCONNECT:
                    {
                        t_netMessages->ReceiveDisconnectMaster(t_newMessage);

                        break;
                    }
                    default:
                    {
                        std::cout << "Some error message received" << std::endl; // TODOCM remove deubgg
                        break;
                    }
                }

                // Reset message
                t_newMessage = NetMessageMasterClientFromMaster();
            }
        }

        void NetworkManagerClient::SendMessages(double p_dt)
        {
            // Send connecting messages if we're in those states
            SendConnectingMessages(p_dt);

            // Send connected messages if we're in those states
            SendConnectedMessages();

            // Send messages to master
            SendMasterMessages(p_dt);
        }

        void NetworkManagerClient::SendConnectingMessages(double p_dt)
        {
            NetworkConnectionsClient* t_connections = NetworkConnectionsClient::GetInstance();
            NetworkMessagesClient* t_netMessages = NetworkMessagesClient::GetInstance();

            // Update timer
            m_nextUpdateTimer += p_dt;

            // If we're not connected we only send at intervals
            if(m_nextUpdateTimer >= m_updateInterval)
            {
                // Reduce timer
                m_nextUpdateTimer -= m_updateInterval;

                // Based on our state we send different message
                switch(t_connections->m_serverConnection.ConnectionState)
                {
                    case ServerConnectionStateFromClient::CONNECTING:
                    {
                        std::cout << "Sending connecting" << std::endl; // TODOCM remove deubgg
                        t_netMessages->SendConnectionRequest();

                        break;
                    }
                    case ServerConnectionStateFromClient::VERSION_CHECK:
                    {
                        std::cout << "Sending version check" << std::endl; // TODOCM remove deubgg
                        t_netMessages->SendVersionCheck();

                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }

        void NetworkManagerClient::SendConnectedMessages()
        {
            NetworkConnectionsClient* t_connections = NetworkConnectionsClient::GetInstance();
            NetworkMessagesClient* t_netMessages = NetworkMessagesClient::GetInstance();

            // If we're connected we always send based on connection
            switch(t_connections->m_serverConnection.ConnectionState)
            {
                case ServerConnectionStateFromClient::CONNECTED:
                {
                    t_netMessages->SendConnected();

                    break;
                }
                case ServerConnectionStateFromClient::LOAD_WORLD:
                {
                    t_netMessages->SendLoadWorld();

                    break;
                }
                case ServerConnectionStateFromClient::IN_GAME:
                {
                    t_netMessages->SendInGame();

                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        void NetworkManagerClient::SendMasterMessages(double p_dt)
        {
            NetworkConnectionsClient* t_connections = NetworkConnectionsClient::GetInstance();
            NetworkMessagesClient* t_netMessages = NetworkMessagesClient::GetInstance();

            // Update timer
            m_masterNextUpdateTimer += p_dt;

            // If we're not connected we only send at intervals
            if(m_masterNextUpdateTimer >= m_masterUpdateInterval)
            {
                // Reduce timer
                m_masterNextUpdateTimer -= m_masterUpdateInterval;

                // Based on our state we send different message
                switch(t_connections->m_masterConnection.ConnectionState)
                {
                    case MasterConnectionStateFromClient::CONNECTING:
                    {
                        t_netMessages->SendConnectionRequestMaster();

                        break;
                    }
                    case MasterConnectionStateFromClient::CONNECTED:
                    {
                        t_netMessages->SendConnectedMaster();

                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }

        void NetworkManagerClient::UpdateTimeouts(double p_dt)
        {
            // Update server
            UpdateTimeoutsServer(p_dt);

            // Update master
            UpdateTimeoutsMaster(p_dt);
        }

        void NetworkManagerClient::UpdateTimeoutsServer(double p_dt)
        {
            NetworkConnectionsClient* t_connections = NetworkConnectionsClient::GetInstance();
            NetworkMessagesClient* t_netMessages = NetworkMessagesClient::GetInstance();

            // If we're not disconnected we want to disconnect if timeout
            if(t_connections->m_serverConnection.ConnectionState > ServerConnectionStateFromClient::DISCONNECTED)
            {
                // Update timer
                t_connections->m_serverConnection.LastResponse += p_dt;
                t_connections->m_serverConnection.LastSequenceUpdate += p_dt;

                // Check if exceeded timeout
                if(t_connections->m_serverConnection.ConnectionState >= ServerConnectionStateFromClient::CONNECTED &&
                       t_connections->m_serverConnection.LastResponse > m_timeoutIntervalConnected ||
                   t_connections->m_serverConnection.ConnectionState < ServerConnectionStateFromClient::CONNECTED &&
                       t_connections->m_serverConnection.LastResponse > m_timeoutIntervalConnecting)
                {
                    std::cout << "Timeout server: " << t_connections->m_serverConnection.LastResponse << " seconds." << std::endl;

                    // Set state as disconnected
                    t_connections->m_serverConnection.ConnectionState = ServerConnectionStateFromClient::DISCONNECTED;
                    t_connections->m_serverConnection.LastResponse = 0;
                    t_connections->m_serverConnection.LastSequenceUpdate = SEQUENCE_TIMER_START;


                    // Send disconnection message to server for good measure
                    t_netMessages->SendDisconnect();

                    // Create change state event
                    ChangeMenuState* t_changeMenuEvent = new ChangeMenuState();
                    t_changeMenuEvent->state = DoremiGameStates::MAINMENU;

                    EventHandler::GetInstance()->BroadcastEvent(t_changeMenuEvent);
                }
            }
        }

        void NetworkManagerClient::UpdateTimeoutsMaster(double p_dt)
        {
            NetworkConnectionsClient* t_connections = NetworkConnectionsClient::GetInstance();

            // Update timer
            t_connections->m_masterConnection.LastResponse += p_dt;

            // If exceed timout
            if(t_connections->m_masterConnection.LastResponse >= m_timeoutIntervalMaster &&
               t_connections->m_masterConnection.ConnectionState > MasterConnectionStateFromClient::DISCONNECTED)
            {
                t_connections->m_masterConnection.ConnectionState = MasterConnectionStateFromClient::CONNECTING;
                t_connections->m_masterConnection.LastResponse = 0;

                // Send disconnection message
                NetworkMessagesClient::GetInstance()->SendDisconnectMaster();
            }
        }

        void NetworkManagerClient::OnEvent(Event* p_event)
        {
            if(p_event->eventType == EventType::ChangeMenuState)
            {
                ChangeMenuState* t_changeMenuEvent = static_cast<ChangeMenuState*>(p_event);
                NetworkConnectionsClient* t_connections = NetworkConnectionsClient::GetInstance();

                // If we are processing to run game we set us to connecting
                if(t_changeMenuEvent->state == DoremiGameStates::LOADING)
                {
                    // Disconnect from master
                    t_connections->m_masterConnection.ConnectionState = MasterConnectionStateFromClient::DISCONNECTED;
                    t_connections->m_masterConnection.LastResponse = 0;

                    // Change connection state
                    t_connections->m_serverConnection.ConnectionState = ServerConnectionStateFromClient::CONNECTING;
                    t_connections->m_serverConnection.LastSequenceUpdate = SEQUENCE_TIMER_START;

                    // Create adress from selected server
                    ServerBrowserHandler* t_ServerBrowserHandler = ServerBrowserHandler::GetInstance();
                    IP_Split t_serverIP = t_ServerBrowserHandler->GetSelectedServerIP();
                    uint16_t t_serverPort = t_ServerBrowserHandler->GetSelectedServerPort();


                    DoremiEngine::Network::NetworkModule& t_NetworkModule = m_sharedContext.GetNetworkModule();
                    t_connections->m_serverConnection.ConnectingSocketHandle = t_NetworkModule.CreateUnreliableSocket();

                    // If adress exist we remove, we don't remove them on delete for rejoin?
                    if(t_connections->m_serverConnection.ConnectingAdress != nullptr)
                    {
                        delete t_connections->m_serverConnection.ConnectingAdress;
                    }
                    if(t_connections->m_serverConnection.ConnectedAdress != nullptr)
                    {
                        delete t_connections->m_serverConnection.ConnectedAdress;
                    }

                    // Create connecting adress
                    // t_connections->m_serverConnection.ConnectingAdress = t_NetworkModule.CreateAdress(127, 0, 0, 1, 5050);
                    t_connections->m_serverConnection.ConnectingAdress =
                        t_NetworkModule.CreateAdress(t_serverIP.IP_a, t_serverIP.IP_b, t_serverIP.IP_c, t_serverIP.IP_d, t_serverPort);

                    cout << (uint32_t)t_serverIP.IP_a << "." << (uint32_t)t_serverIP.IP_b << "." << (uint32_t)t_serverIP.IP_c << "."
                         << (uint32_t)t_serverIP.IP_d << endl;
                }
                else if(t_changeMenuEvent->state == DoremiGameStates::SERVER_BROWSER)
                {
                    // Disconnect from master
                    t_connections->m_masterConnection.ConnectionState = MasterConnectionStateFromClient::CONNECTING;
                    t_connections->m_masterConnection.LastResponse = 0;

                    // TODOXX if we want multiple states(in game top menu) this wont work
                    // Change connection state
                    t_connections->m_serverConnection.ConnectionState = ServerConnectionStateFromClient::DISCONNECTED;
                    t_connections->m_serverConnection.LastResponse = 0;
                    t_connections->m_serverConnection.LastSequenceUpdate = SEQUENCE_TIMER_START;

                    // Remove sockets
                    DoremiEngine::Network::NetworkModule& t_NetworkModule = m_sharedContext.GetNetworkModule();
                    t_NetworkModule.DeleteSocket(t_connections->m_serverConnection.ConnectedSocketHandle);
                    t_NetworkModule.DeleteSocket(t_connections->m_serverConnection.ConnectingSocketHandle);
                }
                else if(t_changeMenuEvent->state != DoremiGameStates::RUNGAME) // If we didn't change to run game, do this
                {

                    // Disconnect from master
                    t_connections->m_masterConnection.ConnectionState = MasterConnectionStateFromClient::DISCONNECTED;
                    t_connections->m_masterConnection.LastResponse = 0;

                    // TODOXX if we want multiple states(in game top menu) this wont work
                    // Change connection state
                    t_connections->m_serverConnection.ConnectionState = ServerConnectionStateFromClient::DISCONNECTED;
                    t_connections->m_serverConnection.LastResponse = 0;
                    t_connections->m_serverConnection.LastSequenceUpdate = SEQUENCE_TIMER_START;


                    // Remove sockets
                    DoremiEngine::Network::NetworkModule& t_NetworkModule = m_sharedContext.GetNetworkModule();
                    t_NetworkModule.DeleteSocket(t_connections->m_serverConnection.ConnectedSocketHandle);
                    t_NetworkModule.DeleteSocket(t_connections->m_serverConnection.ConnectingSocketHandle);
                }
            }
        }
    }
}