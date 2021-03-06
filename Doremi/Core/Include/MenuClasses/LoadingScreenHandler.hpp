#pragma once

// Project specific
#include <DirectXMath.h>
#include <vector>
#include <Doremi/Core/Include/MenuClasses/Button.hpp>
#include <Doremi/Core/Include/MenuClasses/ScreenObject.hpp>
#include <Doremi/Core/Include/MenuClasses/Bar.hpp>

namespace DoremiEngine
{
    namespace Core
    {
        class SharedContext;
    }
}

namespace Doremi
{
    namespace Core
    {

        /**
        Only an example of what a manager might look like
        Doesn't do anything, and could be removed once examples are no longer necessary*/
        class LoadingScreenHandler
        {
        public:
            static LoadingScreenHandler* GetInstance();
            static void StartLoadingScreenHandler(const DoremiEngine::Core::SharedContext& p_sharedContext);

            void Update(double p_dt);

            auto& GetScreenObjects() { return m_screenObjects; }
            auto& GetBars() { return m_bars; }
            void Reset();

        private:
            explicit LoadingScreenHandler(const DoremiEngine::Core::SharedContext& p_sharedContext);

            ~LoadingScreenHandler();


            static LoadingScreenHandler* m_singleton;
            const DoremiEngine::Core::SharedContext& m_sharedContext;

            ScreenObject m_background;

            Bar m_loadingBar;

            std::vector<ScreenObject*> m_screenObjects;
            std::vector<Bar*> m_bars;
        };
    }
}
