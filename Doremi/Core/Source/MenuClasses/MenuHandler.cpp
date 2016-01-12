// Project specific
#include <Doremi/Core/Include/MenuClasses/MenuHandler.hpp>
#include <DoremiEngine/Graphic/Include/GraphicModule.hpp>

#include <Doremi/Core/Include/InputHandlerClient.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Manager/MeshManager.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Mesh/MaterialInfo.hpp>

// Third party

// Standard
#include <iostream>

using namespace std;

namespace Doremi
{
    namespace Core
    {
        MenuHandler::MenuHandler(const DoremiEngine::Core::SharedContext& p_sharedContext, DirectX::XMFLOAT2 p_resolution)
            : m_sharedContext(p_sharedContext), m_resolution(p_resolution)
        {
        }

        MenuHandler::~MenuHandler() {}
        MenuHandler* MenuHandler::m_singleton = nullptr;

        void MenuHandler::StartMenuHandler(const DoremiEngine::Core::SharedContext& p_sharedContext, DirectX::XMFLOAT2 p_resolution)
        {
            if(m_singleton != nullptr)
            {
                std::runtime_error("MenuHandler StartPlayerHandler called multiple times.");
            }
            m_singleton = new MenuHandler(p_sharedContext, p_resolution);
        }

        MenuHandler* MenuHandler::GetInstance()
        {
            if(m_singleton == nullptr)
            {
                std::runtime_error("MenuHandler not initialized, GetInstance called.");
            }

            return m_singleton;
        }

        void MenuHandler::Initialize(std::vector<string> p_buttonTextureNames)
        {
            using namespace DirectX;
            m_currentButton = 0;
            size_t length = p_buttonTextureNames.size();
            int offset = 5;
            // G�r h�jden p� varje knapp bero p� uppl�sningen
            float t_buttonHeight = (m_resolution.y - offset * 2) / length;
            // En knapp t�cker halva sk�rmen. Det kommer finnas 1/4 p� varje sida av knapparna
            float t_buttonWidth = m_resolution.x / 2;
            // Positionera till v�nster om mitten
            float t_buttonXPosition = m_resolution.x / 4;
            for(size_t i = 0; i < length; i++)
            {
                // L�gg in materialinfo � meshinfo f�r varje knapp i dess klass instantiering. L�gg till i listan f�r knappar
                DoremiEngine::Graphic::MaterialInfo* t_materialInfo =
                    m_sharedContext.GetGraphicModule().GetSubModuleManager().GetMeshManager().BuildMaterialInfo(p_buttonTextureNames[i]);
                DoremiEngine::Graphic::MeshInfo* t_meshInfo = m_sharedContext.GetGraphicModule().GetSubModuleManager().GetMeshManager().BuildMeshInfo("hej");
                // G�ra offset p� den f�rsta knappen resten l�ses av size adjustment
                m_buttonList.push_back(Button(XMFLOAT2(t_buttonXPosition, t_buttonHeight * i + offset),
                                              XMFLOAT2(t_buttonWidth, t_buttonHeight - offset), t_materialInfo, t_meshInfo, (MenuStates::MenuState)i));
            }
            m_inputHandler = new InputHandlerClient(m_sharedContext);
        }

        int MenuHandler::Update(double p_dt)
        {
            int mouseX;
            int mouseY;
            m_inputHandler->Update();
            m_inputHandler->GetMousePos(mouseX, mouseY);
            size_t length = m_buttonList.size();
            // Check if cursor is inside one of the buttons if it is then save that buttons index
            for(size_t i = 0; i < length; i++)
            {
                if(m_buttonList[i].CheckIfInside(mouseX, mouseY))
                {
                    m_currentButton = i;
                    break;
                }
                else
                {
                    m_currentButton = -1;
                }
            }
            // check if player has clicked the mouse and is hovering over a button
            if(m_inputHandler->CheckBitMaskInputFromGame((int)UserCommandPlaying::LeftClick) && m_currentButton != -1)
            {
                return m_buttonList[m_currentButton].m_menuState;
            }
            else
            {
                return MenuStates::MAINMENU;
            }
        }
    }
}