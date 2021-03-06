#pragma once
// Project specific
#include <DirectXMath.h>
#include <DoremiEngine/Graphic/Include/Interface/Mesh/MaterialInfo.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Texture/SpriteInfo.hpp>
#include <Doremi/Core/Include/Helper/DoremiStates.hpp>
namespace Doremi
{
    namespace Core
    {
        enum class DoremiButtonActions
        {
            GO_TO_MAINMENU,
            GO_TO_SERVER_BROWSER,
            GO_TO_OPTIONS,
            START_GAME,
            PAUSE,
            VICTORY,
            EXIT,
            SET_FULLSCREEN,
            RESOLUTION_PRESS,
            RESOLUTION_SELECT,
            REFRESH_PRESS,
            REFRESH_SELECT,
            MONITOR_PRESS,
            MONITOR_SELECT,
            OPTIONS_APPLY,
            OPTIONS_CANCEL,
        };

        struct ButtonMaterials
        {
            DoremiEngine::Graphic::MaterialInfo* m_vanillaMaterial;
            DoremiEngine::Graphic::MaterialInfo* m_highLightedMaterial;
            DoremiEngine::Graphic::MaterialInfo* m_selectedLightedMaterial;
        };

        using namespace DirectX;

        /**
            Only an example of what a manager might look like
            Doesn't do anything, and could be removed once examples are no longer necessary
        */
        class Button
        {
        public:
            Button(ButtonMaterials p_buttonMaterials, DoremiEngine::Graphic::SpriteInfo* p_spriteInfo, Core::DoremiButtonActions p_menuState);
            Button(ButtonMaterials p_buttonMaterials, DoremiEngine::Graphic::SpriteInfo* p_spriteInfo, Core::DoremiButtonActions p_menuState, XMFLOAT2 p_incrementLogic);
            Button();

            ~Button();

            // Checks if mouse is inside this button
            bool CheckIfInside(float p_mousePosX, float p_mousePosY);

            void SetSelected();

            Core::DoremiButtonActions m_buttonAction;

            DoremiEngine::Graphic::MaterialInfo* m_materialInfo;

            // Add "Mesh" info here, but its buffer
            DoremiEngine::Graphic::SpriteInfo* m_spriteInfo;

            ButtonMaterials m_buttonMaterials;

            XMFLOAT2 m_incrementLogic;

        private:
        };
    }
}
