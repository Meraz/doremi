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
                std::runtime_error("MenuHandler StartMenuHandler called multiple times.");
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
            // initialize currentbutton
            m_currentButton = -1;
            size_t length = p_buttonTextureNames.size();
            int offset = 5;
            // G�r h�jden p� varje knapp bero p� uppl�sningen
            // Orienterad efter origin. Length anv�nds och fungerar �ven om den listan �r f�r l�ng eftersom vi g�r efter extents fr�n origin.
            float t_buttonHeight = ((m_resolution.y - offset * 2) / length);
            // En knapp t�cker halva sk�rmen. Extents �t b�da h�llen ger halva sk�rmen. Extentsen blir d� en 4dedel
            float t_buttonWidth = m_resolution.x / 4;
            // Positionera I mitten av sk�rmen
            float t_buttonXPosition = m_resolution.x / 2;
            length = floor(length / 2);
            for(size_t i = 0; i < length; i++)
            {
                // L�gg in materialinfo � meshinfo f�r varje knapp i dess klass instantiering. L�gg till i listan f�r knappar
                // Klassisk klur function i Y led. dirx startar resolution.y l�ngs ner. Vi vill b�rja h�gst upp. Sedan subtrahera en hel knapp per i,
                // och en offset f�r att f�rsta ska skjutas ned.
                XMFLOAT2 t_position = XMFLOAT2(t_buttonXPosition, m_resolution.y - t_buttonHeight * i * 2 - t_buttonHeight + offset);
                // S�tt size p� knappen. Detta �r extentsen...
                XMFLOAT2 t_size = XMFLOAT2(t_buttonWidth, t_buttonHeight - offset);
                // Skapa en buttonmaterial struct. Denna h�ller 2 buildmaterialinfos f�r att g�ra kortare parameterlistor
                Doremi::Core::ButtonMaterials t_buttonMaterials;
                // Ladda materialinfo x2 Anv�nder i+length som en ful h�rdkodning... D�rf�r m�ste listan med namn vara i r�tt ordning d�r
                // highlighttexturerna kommer sist.
                t_buttonMaterials.m_vanillaMaterial =
                    m_sharedContext.GetGraphicModule().GetSubModuleManager().GetMeshManager().BuildMaterialInfo(p_buttonTextureNames[i]);
                t_buttonMaterials.m_highLightedMaterial =
                    m_sharedContext.GetGraphicModule().GetSubModuleManager().GetMeshManager().BuildMaterialInfo(p_buttonTextureNames[i + length]);
                // Ladda in meshen
                DoremiEngine::Graphic::MeshInfo* t_meshInfo =
                    m_sharedContext.GetGraphicModule().GetSubModuleManager().GetMeshManager().BuildQuadMeshInfo("Quad");
                // Skapa knapp � stoppa in i listan Menustate �r riskmodd. H�rdkodat mot vilken ordning som namnen laddas in. Finns kommentarer till
                // detta androp om ordning
                m_buttonList.push_back(Button(t_position, t_size, t_buttonMaterials, t_meshInfo, (MenuStates::MenuState)i));
            }
            m_inputHandler = new InputHandlerClient(m_sharedContext);
        }

        int MenuHandler::GetCurrentButton() { return m_currentButton; }

        std::vector<Button> MenuHandler::GetButtons() { return m_buttonList; }

        int MenuHandler::Update(double p_dt)
        {
            int mouseX;
            int mouseY;
            // Get mouse cursor position
            m_inputHandler->Update();
            m_inputHandler->GetMousePos(mouseX, mouseY);
            size_t length = m_buttonList.size();
            m_currentButton = -1;
            // Check if cursor is inside one of the buttons if it is then save that buttons index
            for(size_t i = 0; i < length; i++)
            {
                if(m_buttonList[i].CheckIfInside(mouseX, m_resolution.y - mouseY))
                {
                    m_currentButton = i;
                }
                else
                {
                    // do nothing
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