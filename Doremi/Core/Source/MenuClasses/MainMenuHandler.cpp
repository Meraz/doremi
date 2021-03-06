// Project specific
#include <Doremi/Core/Include/MenuClasses/MainMenuHandler.hpp>
#include <DoremiEngine/Graphic/Include/GraphicModule.hpp>
#include <Doremi/Core/Include/EventHandler/EventHandler.hpp>
#include <Doremi/Core/Include/InputHandlerClient.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Manager/MeshManager.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Mesh/MaterialInfo.hpp>
#include <DoremiEngine/Graphic/Include/GraphicModule.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Manager/DirectXManager.hpp>
#include <DoremiEngine/Graphic/Include/GraphicModule.hpp>

// Event
#include <Doremi/Core/Include/EventHandler/Events/ChangeMenuState.hpp>


#include <Doremi/Core/Include/PlayerHandlerClient.hpp>

// Third party

// Standard
#include <iostream>

using namespace std;

namespace Doremi
{
    namespace Core
    {
        MainMenuHandler::MainMenuHandler(const DoremiEngine::Core::SharedContext& p_sharedContext, DirectX::XMFLOAT2 p_resolution)
            : m_sharedContext(p_sharedContext), m_resolution(p_resolution), m_isFullscreen(false), m_currentButton(-1)
        {
        }

        MainMenuHandler::~MainMenuHandler() {}
        MainMenuHandler* MainMenuHandler::m_singleton = nullptr;

        void MainMenuHandler::StartMainMenuHandler(const DoremiEngine::Core::SharedContext& p_sharedContext, DirectX::XMFLOAT2 p_resolution)
        {
            if(m_singleton != nullptr)
            {
                std::runtime_error("MainMenuHandler StartMainMenuHandler called multiple times.");
            }
            m_singleton = new MainMenuHandler(p_sharedContext, p_resolution);
        }

        MainMenuHandler* MainMenuHandler::GetInstance()
        {
            if(m_singleton == nullptr)
            {
                std::runtime_error("MainMenuHandler not initialized, GetInstance called.");
            }

            return m_singleton;
        }

        struct TextureBoundly
        {
            TextureBoundly(const std::string& p_normal, const std::string& p_highlight, const std::string& p_pressed)
                : Normal(p_normal), Highlighted(p_highlight), Pressed(p_pressed)
            {
            }
            std::string Normal;
            std::string Highlighted;
            std::string Pressed;
        };

        void MainMenuHandler::Initialize()
        {
            using namespace DirectX;
            DoremiEngine::Graphic::MeshManager& t_meshManager = m_sharedContext.GetGraphicModule().GetSubModuleManager().GetMeshManager();

            // initialize currentbutton
            m_currentButton = -1;

            // Create background
            DoremiEngine::Graphic::SpriteData t_data;

            t_data.halfsize = XMFLOAT2(0.5f, 0.5f);
            t_data.origo = XMFLOAT2(0.0f, 0.0f);
            t_data.position = XMFLOAT2(0.5f, 0.5f);
            t_data.txtPos = XMFLOAT2(0.0f, 0.0f);
            t_data.txtSize = XMFLOAT2(1.0f, 1.0f);

            DoremiEngine::Graphic::SpriteInfo* t_spriteInfo = t_meshManager.BuildSpriteInfo(t_data);
            DoremiEngine::Graphic::MaterialInfo* t_matInfo = t_meshManager.BuildMaterialInfo("ANB_Menu_Background.dds");

            m_background = ScreenObject(t_matInfo, t_spriteInfo);
            m_screenObjects.push_back(&m_background);


            // Create menu box
            t_data.halfsize = XMFLOAT2(0.15f, 0.4f);
            t_data.position = XMFLOAT2(0.70f, 0.5f);
            t_spriteInfo = t_meshManager.BuildSpriteInfo(t_data);
            t_matInfo = t_meshManager.BuildMaterialInfo("ANB_Menu_MenubarBackground.dds");

            m_menuBar = ScreenObject(t_matInfo, t_spriteInfo);
            m_screenObjects.push_back(&m_menuBar);


            // Initialize menu
            std::vector<TextureBoundly> p_buttonTextureNames;

            // textures under in the same way
            p_buttonTextureNames.push_back(TextureBoundly("ANB_Menu_BTN_PLAY.dds", "ANB_Menu_BTN_PLAY_Highlight.dds", "ANB_Menu_BTN_PLAY_Clicked"));
            p_buttonTextureNames.push_back(
                TextureBoundly("ANB_Menu_BTN_OPTIONS.dds", "ANB_Menu_BTN_OPTIONS_Highlight.dds", "ANB_Menu_BTN_OPTIONS_Clicked"));
            p_buttonTextureNames.push_back(TextureBoundly("ANB_Menu_BTN_EXIT.dds", "ANB_Menu_BTN_EXIT_Highlight.dds", "ANB_Menu_BTN_EXIT_Clicked"));
            // p_buttonTextureNames.push_back(TextureBoundly("Fullscreen.dds", "FullscreenHighlighted.dds", ""));


            DoremiButtonActions statesForButtons[3];
            statesForButtons[0] = DoremiButtonActions::GO_TO_SERVER_BROWSER;
            statesForButtons[1] = DoremiButtonActions::GO_TO_OPTIONS;
            statesForButtons[2] = DoremiButtonActions::EXIT;
            // statesForButtons[3] = DoremiButtonActions::SET_FULLSCREEN;

            size_t length = p_buttonTextureNames.size();

            // Basic position
            t_data.halfsize = XMFLOAT2(0.12f, 0.08f);
            t_data.origo = XMFLOAT2(0.0f, 0.0f);
            t_data.position = XMFLOAT2(0.7f, 0.0f);
            t_data.txtPos = XMFLOAT2(0.0f, 0.0f);
            t_data.txtSize = XMFLOAT2(1.0f, 1.0f);


            for(size_t i = 0; i < length; i++)
            {
                // Skapa en buttonmaterial struct. Denna h�ller 2 buildmaterialinfos f�r att g�ra kortare parameterlistor
                Doremi::Core::ButtonMaterials t_buttonMaterials;

                // Load materials
                t_buttonMaterials.m_vanillaMaterial = t_meshManager.BuildMaterialInfo(p_buttonTextureNames[i].Normal);
                t_buttonMaterials.m_highLightedMaterial = t_meshManager.BuildMaterialInfo(p_buttonTextureNames[i].Highlighted);
                t_buttonMaterials.m_selectedLightedMaterial = nullptr;
                ;

                t_data.position.y = 1.0f / (float)(length + 1) + i / (float)(length + 1);

                DoremiEngine::Graphic::SpriteInfo* t_spriteInfo = t_meshManager.BuildSpriteInfo(t_data);

                m_buttonList.push_back(Button(t_buttonMaterials, t_spriteInfo, statesForButtons[i]));
            }
        }

        int MainMenuHandler::GetCurrentButton() { return m_currentButton; }

        int MainMenuHandler::Update(double p_dt) // TODOKO Dont need to return int anymore
        {
            uint32_t t_mousePosX;
            uint32_t t_mousePosY;

            // Get mouse cursor position
            InputHandlerClient* t_inputHandler = static_cast<PlayerHandlerClient*>(PlayerHandler::GetInstance())->GetInputHandler();
            t_inputHandler->GetMousePos(t_mousePosX, t_mousePosY);

            // Get screen resolution
            XMFLOAT2 t_screenResolution = m_sharedContext.GetGraphicModule().GetSubModuleManager().GetDirectXManager().GetScreenResolution();

            // Make position to screen cordinates
            float t_mouseScreenPosX = static_cast<float>(t_mousePosX) / t_screenResolution.x;
            float t_mouseScreenPosY = static_cast<float>(t_mousePosY) / t_screenResolution.y;

            // Set standard not to be selected
            m_currentButton = -1;

            // Check if cursor is inside one of the buttons if it is then save that buttons index
            size_t length = m_buttonList.size();
            for(size_t i = 0; i < length; i++)
            {
                if(m_buttonList[i].CheckIfInside(t_mouseScreenPosX, t_mouseScreenPosY))
                {
                    m_currentButton = i;
                }
                else
                {
                    // do nothing
                }
            }

            // check if player has clicked the mouse and is hovering over a button
            if(t_inputHandler->CheckForOnePress((int)UserCommandPlaying::LeftClick) && m_currentButton != -1)
            {
                switch(m_buttonList[m_currentButton].m_buttonAction)
                {
                    case Core::DoremiButtonActions::GO_TO_MAINMENU:
                    {
                        // passing state change event
                        Core::ChangeMenuState* menuEvent = new Core::ChangeMenuState();
                        menuEvent->state = DoremiGameStates::MAINMENU;
                        Core::EventHandler::GetInstance()->BroadcastEvent(menuEvent);

                        break;
                    }
                    case Core::DoremiButtonActions::GO_TO_SERVER_BROWSER:
                    {
                        // passing state change event
                        Core::ChangeMenuState* menuEvent = new Core::ChangeMenuState();
                        menuEvent->state = DoremiGameStates::SERVER_BROWSER;
                        Core::EventHandler::GetInstance()->BroadcastEvent(menuEvent);

                        break;
                    }
                    case Core::DoremiButtonActions::GO_TO_OPTIONS:
                    {
                        // passing state change event
                        Core::ChangeMenuState* menuEvent = new Core::ChangeMenuState();
                        menuEvent->state = DoremiGameStates::OPTIONS;
                        Core::EventHandler::GetInstance()->BroadcastEvent(menuEvent);

                        break;
                    }
                    case Core::DoremiButtonActions::EXIT:
                    {
                        // passing state change event
                        Core::ChangeMenuState* menuEvent = new Core::ChangeMenuState();
                        menuEvent->state = DoremiGameStates::EXIT;
                        Core::EventHandler::GetInstance()->BroadcastEvent(menuEvent);

                        break;
                    }
                    case Core::DoremiButtonActions::SET_FULLSCREEN:
                    {
                        m_sharedContext.GetGraphicModule().GetSubModuleManager().GetDirectXManager().SetFullscreen(m_isFullscreen);
                        m_isFullscreen = !m_isFullscreen;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            else
            {
                // Nothing
            }
            return 0;
        }
    }
}