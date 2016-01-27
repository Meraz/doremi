#pragma once
#include <Doremi/Core/Include/LevelLoader.hpp>

namespace Doremi
{
    namespace Core
    {
        class LevelLoaderClient : public LevelLoader
        {
        public:
            LevelLoaderClient(const DoremiEngine::Core::SharedContext& p_sharedContext);

            virtual ~LevelLoaderClient();

            void LoadLevel(const std::string& p_fileName);

            CharacterDataNames LoadCharacter(const std::string& p_fileName);

        protected:
            void BuildComponents(int p_entityId, int p_meshCouplingID, std::vector<DoremiEngine::Graphic::Vertex>& p_vertexBuffer) override;
            void BuildLights();
            void LoadFileInternal(const std::string& p_fileName);
        };
    }
}