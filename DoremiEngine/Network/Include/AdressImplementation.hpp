#pragma once
// Project specific
#include <Adress.hpp>

// Standard libraries
#if PLATFORM == PLATFORM_WINDOWS
#include <WinSock2.h>
#elif PLATFORM == PLATFORM_UNIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#endif

namespace DoremiEngine
{
    namespace Network
    {
        /**
            Adress struct for comosing and holding adress information
        */
        class AdressImplementation : public Adress
        {
        public:
            /**
                Constructs Adress
            */
            AdressImplementation();

            /**
                Construct Adress with any input IP, and set port
            */
            AdressImplementation(uint16_t p_port);

            /**
               Constructs Adress with IP in format "p_a.p_b.p_c.p_d" as in "255.255.255.255" with
                port
            */
            AdressImplementation(uint32_t p_a, uint32_t p_b, uint32_t p_c, uint32_t p_d, uint16_t p_port);

            /**
                TODOCM doc
            */
            ~AdressImplementation();

            /**
                TODOCM doct
            */
            uint16_t GetPort() const override { return m_port; };

            /**
                Returns IP
            */
            uint32_t GetIP() const override { return m_IP; };

            uint8_t GetIP_A() const override { return m_IP_a; }

            uint8_t GetIP_B() const override { return m_IP_b; }

            uint8_t GetIP_C() const override { return m_IP_c; }

            uint8_t GetIP_D() const override { return m_IP_d; }

            /**
                Returns composed Adress used for socket binding
            */
            SOCKADDR_IN GetAdress() const { return m_Adress; };

            /**
                Set the IP in format "p_a.p_b.p_c.p_d" as in "255.255.255.255", use ComposeAdress to
                be able to use GetAdress after this
            */
            void SetIP(uint32_t p_a, uint32_t p_b, uint32_t p_c, uint32_t p_d) override;

            /**
                Set the port, use ComposeAdress to be able to use GetAdress after this
            */
            void SetNetPort(uint16_t p_port) override;

            /**
                Set the actuall composed adress, warning may not use IP and PORT in a good way now
                TODOCM extract the port and IP if possible
                TODOCM move to firend class
            */
            void SetAdress(SOCKADDR_IN p_adress);

            /**
               Compose the adress used for socket binding when the functions SetIP or/and
                SetNetPort was used
            */
            void ComposeAdress() override;

            /**
                Returns the IP as a string
            */
            std::string GetIPToString() const override { return m_IPString; };

            /**
                Check if adress have same IP and Port
                TODOCM fix up this code, not the most efficiant way
            */
            bool AdressImplementation::operator==(const Adress& p_adress) const override
            {
                AdressImplementation* castedAdress = (AdressImplementation*)&p_adress;

                bool SameAdress = m_Adress.sin_addr.s_addr == castedAdress->m_Adress.sin_addr.s_addr;

                bool SamePort = m_Adress.sin_port == castedAdress->m_Adress.sin_port;

                return SameAdress && SamePort;
            }

            /**
                Check if adress ONLY have same IP
                TODOCM maybe fix this code as well, not the most efficiant way
            */
            bool AdressImplementation::operator*=(const Adress& p_adress) const override
            {
                AdressImplementation* castedAdress = (AdressImplementation*)&p_adress;

                bool SameAdress = m_Adress.sin_addr.s_addr == castedAdress->m_Adress.sin_addr.s_addr;

                return SameAdress;
            }

        private:
            /**
                IP in form of Big Endian
            */
            uint32_t m_IP;

            /**
                IP in parts
            */
            uint8_t m_IP_a;
            uint8_t m_IP_b;
            uint8_t m_IP_c;
            uint8_t m_IP_d;

            /**
                Port for either in/out
            */
            uint16_t m_port;

            /**
                Composed adress used for socket binding
            */
            SOCKADDR_IN m_Adress;

            /**
                string of IP used for debug
            */
            std::string m_IPString;
        };
    }
}