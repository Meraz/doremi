#include <AdressImplementation.hpp>

namespace DoremiEngine
{
    namespace Network
    {
        AdressImplementation::AdressImplementation()
        {
            // Set standards as local IP, for debug reasons
            SetNetPort(0);

            m_IP = htonl(INADDR_ANY);

            ComposeAdress();
        }

        AdressImplementation::AdressImplementation(uint16_t p_port)
        {
            // Set standards as local IP, for debug reasons
            SetNetPort(p_port);

            m_IP = htonl(INADDR_ANY);

            ComposeAdress();
        }

        AdressImplementation::AdressImplementation(uint32_t p_a, uint32_t p_b, uint32_t p_c, uint32_t p_d, uint16_t p_port)
        {
            // Set IP
            SetIP(p_a, p_b, p_c, p_d);

            // Set Port
            SetNetPort(p_port);

            // Create adress struct for later use
            ComposeAdress();
        }

        AdressImplementation::~AdressImplementation() {}

        void AdressImplementation::SetIP(uint32_t p_a, uint32_t p_b, uint32_t p_c, uint32_t p_d)
        {
            // TODOCM maybe save IP parts, and compose string when needed
            // Save IP as string
            m_IPString = std::to_string(p_a) + "." + std::to_string(p_b) + "." + std::to_string(p_c) + "." + std::to_string(p_d);

            // Moves bits so that the IP segments(255 max, one byte) to be placed after eachother
            int32_t ToConvertIP = ((p_a << 24) | (p_b << 16) | (p_c << 8) | p_d);

            // Performs big-endian on the IP (network necessary)
            m_IP = htonl(ToConvertIP);

            // Set ip parts
            m_IP_a = p_a;
            m_IP_b = p_b;
            m_IP_c = p_c;
            m_IP_d = p_d;
        }

        void AdressImplementation::SetNetPort(uint16_t p_port)
        {
            m_port = p_port;
            m_Adress.sin_port = htons(m_port);
        }

        void AdressImplementation::SetAdress(SOCKADDR_IN p_adress)
        {
            // TODOCM save port and IP here aswell if possible
            m_Adress = p_adress;

            // Set ip parts
            m_IP_a = m_Adress.sin_addr.S_un.S_un_b.s_b1;
            m_IP_b = m_Adress.sin_addr.S_un.S_un_b.s_b2;
            m_IP_c = m_Adress.sin_addr.S_un.S_un_b.s_b3;
            m_IP_d = m_Adress.sin_addr.S_un.S_un_b.s_b4;
        }

        void AdressImplementation::ComposeAdress()
        {
            m_Adress = {0};
            m_Adress.sin_family = AF_INET;
            m_Adress.sin_port = htons(m_port);
            m_Adress.sin_addr.s_addr = m_IP;
        }
    }
}