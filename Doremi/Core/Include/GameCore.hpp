#pragma once
// Project specific

// Standard libraries
#include <vector>
namespace Doremi
{
    namespace Core
    {
		class Manager;
        class EntityInterface;
        class GameCore
        {
        public:
            GameCore();
            virtual ~GameCore();
            /** Initializes entity system and different engine DLLs*/
            void Initialize();
            /** Starts the game. Game loop*/
            void StartCore();

        private:
            std::vector<Manager*> m_managers;
        };
    }
}