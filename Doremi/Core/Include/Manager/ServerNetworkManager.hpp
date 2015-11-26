#pragma once
#include <Doremi/Core/Include/Manager/Manager.hpp>
#include <DoremiEngine/Network/Include/NetworkModule.hpp>

namespace Doremi
{
    namespace Core
    {
        class ServerNetworkManager : public Manager
        {
            public:
            /**
            TODOCM doc
            */
            ServerNetworkManager(const DoremiEngine::Core::SharedContext& p_sharedContext);

            /**
            TODOCM doc
            */
            virtual ~ServerNetworkManager();

            /**
            TODOCM doc
            */
            void Update(double p_dt) override;
        };
    }
}
