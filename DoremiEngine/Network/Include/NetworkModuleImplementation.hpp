#pragma once

#define PLATFORM_WINDOWS 1
#define PLATFORM_UNIX 2


#ifdef WIN32
#define PLATFORM PLATFORM_WINDOWS
#else // Add mac as well
#define PLATFORM PLATFORM_UNIX
#endif

// Project specific
#include <NetworkModule.hpp>

// Standard libraries
#include <Socket.hpp>
#include <string>
#include <map>

namespace DoremiEngine
{
    namespace Network
    {
        /**
            Computing the data-flow management of incomming and outcomming packages
            Creates and removes connections

            Reliable flow
            Server: CreateReliableConnection -> AcceptConnection
            Client: ConnectToReliable

            Then send/recv

            Unreliable flow
            Server: CreateUnreliableWaitingSocket
            Client: CreteUnreliableSocket SendUnreliableData(will bind the socket implicit)

            Then sendUnreliable/recvUnreliable
        */
        class NetworkModuleImplementation : public NetworkModule
        {
        public:
            /**
                TODOCM docs
            */
            NetworkModuleImplementation();

            /**
                TODOCM docs
            */
            virtual ~NetworkModuleImplementation();

            /**
                TODOCM docs
            */
            void Startup() override;

            /**
                TODOCM docs
            */
            Adress* CreateAdress() override;

            /**
                TODOCM docs
            */
            Adress* CreateAdress(const Adress& m_adress) override;

            /**
                TODOCM docs
            */
            Adress* CreateAdress(uint16_t p_port);

            /**
                TODOCM docs
            */
            Adress* CreateAdress(uint32_t p_a, uint32_t p_b, uint32_t p_c, uint32_t p_d, uint16_t p_port);

            /**
                TODOCM docs
            */
            bool SendReliableData(void* t_data, const uint32_t& t_dataSize, const size_t& p_sendToSocket) override;

            /**
                TODOCM docs
            */
            bool ReceiveReliableData(void* t_data, const uint32_t& t_dataSize, const size_t& p_ReceiveFromSocket, uint32_t& p_dataSizeReceived) override;

            /**
                TODOCM docs
            */
            bool SendUnreliableData(void* p_data, const uint32_t& p_dataSize, const size_t& p_sendToSocketHandle, const Adress* p_adressToSendTo) override;

            /**
                TODOCM docs
            */
            bool ReceiveUnreliableData(void* p_data, const uint32_t& p_dataSize, const size_t& p_ReceiveFromSocketHandle, Adress* p_AdressOut,
                                       uint32_t& p_dataSizeReceived) override;

            /**
                TODOCM docs
            */
            bool ReceiveUnreliableData(void* p_data, const uint32_t& p_dataSize, const size_t& p_ReceiveFromSocketHandle, uint32_t& p_dataSizeReceived) override;

            /**
                TODOCM docs
            */
            bool ConnectToReliable(const Adress* p_adressToConnectTo, size_t& o_socketHandle) override;

            /**
                TODOCM docs
            */
            size_t CreateReliableConnection(const Adress* p_adressToConnectTo, uint8_t p_maxWaitingConnections) override;

            /**
                TODOCM docs
            */
            bool AcceptConnection(size_t p_socketID, size_t& p_outSocketID, Adress* p_adressOut) override;

            /**
                TODOCM docs
            */
            size_t CreateUnreliableSocket() override;

            /**
                TODOCM docs
            */
            size_t CreateUnreliableWaitingSocket(const Adress* p_adressToConnectTo) override;

            /**
                TODOCM doc
            */
            void DeleteSocket(size_t p_socketID) override;

            /**
                TODOCM docs
            */
            void Shutdown() override;

        private:
            /**
                TODOCM docs
            */
            Socket* GetSocketFromMap(size_t p_handle);

            /**
                TODOCM docs
            */
            bool m_isInitialized;

            /**
                TODOCM docs
            */
            std::map<size_t, Socket*> m_socketHandleMap; // TODOCM change way of storing socket
        };
    }
}