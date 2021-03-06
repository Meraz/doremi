#include <Doremi/Core/Include/MenuClasses/OptionsHandler.hpp>
#include <DoremiEngine/Core/Include/SharedContext.hpp>
#include <Doremi/Core/Include/PlayerHandlerClient.hpp>
#include <Doremi/Core/Include/EntityComponent/EntityHandler.hpp>
#include <Doremi/Core/Include/EntityComponent/Components/HealthComponent.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Manager/MeshManager.hpp>
#include <DoremiEngine/Graphic/Include/GraphicModule.hpp>
#include <Doremi/Core/Include/InputHandlerClient.hpp>
#include <Doremi/Core/Include/EventHandler/Events/ChangeMenuState.hpp>
#include <Doremi/Core/Include/EventHandler/EventHandler.hpp>
#include <Doremi/Core/Include/CameraHandler.hpp>
#include <DoremiEngine/Configuration/Include/ConfigurationModule.hpp>
#include <Doremi/Core/Include/PlayerHandlerClient.hpp>
#include <Doremi/Core/Include/AudioHandler.hpp>
#include <algorithm>
#include <iostream>

namespace Doremi
{
    namespace Core
    {
        OptionsHandler* OptionsHandler::m_singleton = nullptr;

        OptionsHandler* OptionsHandler::GetInstance()
        {
            if(m_singleton == nullptr)
            {
                std::runtime_error("GetInstance called before StartOptionsHandler");
            }
            return m_singleton;
        }

        void OptionsHandler::StartOptionsHandler(const DoremiEngine::Core::SharedContext& p_sharedContext)
        {
            if(m_singleton != nullptr)
            {
                std::runtime_error("StartOptionsHandler called multiple times");
            }
            m_singleton = new OptionsHandler(p_sharedContext);
        }

        OptionsHandler::OptionsHandler(const DoremiEngine::Core::SharedContext& p_sharedContext)
            : m_sharedContext(p_sharedContext),
              t_resolutionDropdownIsActive(false),
              t_refreshDropdownIsActive(false),
              t_monitorDropdownIsActive(false),
              m_highlightedButton(nullptr)
        {
            DoremiEngine::Graphic::MeshManager& t_meshManager = p_sharedContext.GetGraphicModule().GetSubModuleManager().GetMeshManager();
            DoremiEngine::Graphic::DirectXManager& t_directxManager = p_sharedContext.GetGraphicModule().GetSubModuleManager().GetDirectXManager();

            // Load start
            // Get current resolution
            XMFLOAT2 t_resolution = t_directxManager.GetScreenResolution();
            m_currentResolutionWidth = static_cast<uint32_t>(t_resolution.x);
            m_currentResolutionHeight = static_cast<uint32_t>(t_resolution.y);

            // Get current Monitor
            m_currentMonitor = t_directxManager.GetCurrentMonitor();

            // Get current refresh rate
            m_currentRefreshRate = t_directxManager.GetRefreshRate();

            // Create background
            DoremiEngine::Graphic::SpriteData t_data;

            t_data.halfsize = XMFLOAT2(0.5f, 0.5f);
            t_data.origo = XMFLOAT2(0.0f, 0.0f);
            t_data.position = XMFLOAT2(0.5f, 0.5f);
            t_data.txtPos = XMFLOAT2(0.0f, 0.0f);
            t_data.txtSize = XMFLOAT2(1.0f, 1.0f);

            DoremiEngine::Graphic::SpriteInfo* t_spriteInfo = t_meshManager.BuildSpriteInfo(t_data);
            DoremiEngine::Graphic::MaterialInfo* t_matInfo = t_meshManager.BuildMaterialInfo("ANB_Menu_Background.dds");
            m_screenObjects.push_back(new ScreenObject(t_matInfo, t_spriteInfo));


            // Create menu box
            t_data.halfsize = XMFLOAT2(0.15f, 0.4f);
            t_data.position = XMFLOAT2(0.70f, 0.5f);
            t_spriteInfo = t_meshManager.BuildSpriteInfo(t_data);
            t_matInfo = t_meshManager.BuildMaterialInfo("ANB_Menu_OPTIONS_MenubarBackground.dds");
            m_screenObjects.push_back(new ScreenObject(t_matInfo, t_spriteInfo));

            // Create fullscreen checkbox
            DoremiEngine::Configuration::ConfiguartionInfo configInfo = p_sharedContext.GetConfigurationModule().GetAllConfigurationValues();
            ButtonMaterials t_materials;
            t_materials.m_vanillaMaterial = t_meshManager.BuildMaterialInfo("ANB_Menu_CHECKBOX_Inactive.dds");
            t_materials.m_highLightedMaterial = t_meshManager.BuildMaterialInfo("ANB_Menu_CHECKBOX_Highlight.dds");
            t_materials.m_selectedLightedMaterial = t_meshManager.BuildMaterialInfo("ANB_Menu_CHECKBOX_Checked.dds");

            t_data.halfsize = XMFLOAT2(0.006f, 0.0105f);
            t_data.position = XMFLOAT2(0.747f, 0.478f);

            t_spriteInfo = t_meshManager.BuildSpriteInfo(t_data);

            Button t_button = Button(t_materials, t_spriteInfo, DoremiButtonActions::SET_FULLSCREEN);

            m_fullscreenCheckbox = CheckBox(t_button, CheckBoxActions::FULLSCREEN);
            m_fullscreenCheckbox.SetPressedByBool(static_cast<bool>(configInfo.Fullscreen));

            m_checkBoxes.push_back(&m_fullscreenCheckbox);

            // Add recording hearing on off textures
            t_data.position = XMFLOAT2(0.9f, 0.725f);
            t_spriteInfo = t_meshManager.BuildSpriteInfo(t_data);
            DoremiEngine::Graphic::MaterialInfo* t_hearing = t_meshManager.BuildMaterialInfo("RecordingHeard.dds");
            DoremiEngine::Graphic::MaterialInfo* t_notHearing = t_meshManager.BuildMaterialInfo("RecordingNotHeard.dds");

            ScreenObject t_objectHearing = ScreenObject(t_hearing, t_spriteInfo);
            ScreenObject t_objectNotHearing = ScreenObject(t_notHearing, t_spriteInfo);
            std::vector<ScreenObject> t_objectToHold;
            t_objectToHold.push_back(t_objectHearing);
            t_objectToHold.push_back(t_objectNotHearing);
            m_recordingHearing = ObjectHolder(t_objectToHold);
            m_objectHolders.push_back(&m_recordingHearing);

            // recording volume bar
            t_data.halfsize = XMFLOAT2(0.08f, 0.02f);
            t_data.origo = XMFLOAT2(0.0f, 0.0f);
            t_data.position = XMFLOAT2(0.75f, 0.7f);
            t_data.txtPos = XMFLOAT2(0.0f, 0.0f);
            t_data.txtSize = XMFLOAT2(1.0f, 1.0f);

            t_spriteInfo = t_meshManager.BuildSpriteInfo(t_data);
            t_matInfo = t_meshManager.BuildMaterialInfo("HealthbarCropped_Health.dds");
            ScreenObject t_barBar = ScreenObject(t_matInfo, t_spriteInfo);

            t_spriteInfo = t_meshManager.BuildSpriteInfo(t_data);
            t_matInfo = t_meshManager.BuildMaterialInfo("HealthbarCropped_Frame.dds");
            ScreenObject t_frontBar = ScreenObject(t_matInfo, t_spriteInfo);

            t_spriteInfo = t_meshManager.BuildSpriteInfo(t_data);
            t_matInfo = t_meshManager.BuildMaterialInfo("HealthbarCropped_Background.dds");
            ScreenObject t_backBar = ScreenObject(t_matInfo, t_spriteInfo);

            m_soundBar = Bar(t_barBar, t_backBar, t_frontBar, 0.75f, 0.08f);
            m_bars.push_back(&m_soundBar);
            // Some extra stuff!
            CreateButtons(p_sharedContext);
            CreateScreenResolutionOption(p_sharedContext);
            CreateRefreshOption(p_sharedContext);
            CreateMonitorOption(p_sharedContext);
            CreateSliders(p_sharedContext);
        }

        OptionsHandler::~OptionsHandler() {}

        void OptionsHandler::CreateButtons(const DoremiEngine::Core::SharedContext& p_sharedContext)
        {
            DoremiEngine::Graphic::MeshManager& t_meshManager = p_sharedContext.GetGraphicModule().GetSubModuleManager().GetMeshManager();
            // Basic position
            DoremiEngine::Graphic::SpriteData t_data;

            t_data.halfsize = XMFLOAT2(0.05f, 0.04f);
            t_data.origo = XMFLOAT2(0.0f, 0.0f);
            t_data.position = XMFLOAT2(0.64f, 0.84f);
            t_data.txtPos = XMFLOAT2(0.0f, 0.0f);
            t_data.txtSize = XMFLOAT2(1.0f, 1.0f);

            // Skapa en buttonmaterial struct. Denna h�ller 2 buildmaterialinfos f�r att g�ra kortare parameterlistor
            Doremi::Core::ButtonMaterials t_buttonMaterialsMiddle;

            // Load materials for the actual square to show resoluton
            t_buttonMaterialsMiddle.m_vanillaMaterial = t_meshManager.BuildMaterialInfo("ANB_Menu__BTN_APPLY.dds");
            t_buttonMaterialsMiddle.m_highLightedMaterial = t_meshManager.BuildMaterialInfo("ANB_Menu_BTN_APPLY_Hightlight.dds");
            t_buttonMaterialsMiddle.m_selectedLightedMaterial = t_meshManager.BuildMaterialInfo("ANB_Menu__BTN_APPLY_Clicked.dds");

            DoremiEngine::Graphic::SpriteInfo* t_spriteInfoMiddleButton = t_meshManager.BuildSpriteInfo(t_data);

            m_applyButton = Button(t_buttonMaterialsMiddle, t_spriteInfoMiddleButton, DoremiButtonActions::OPTIONS_APPLY);

            m_optionsButtons.push_back(&m_applyButton);



            // cancel
            t_data.position = XMFLOAT2(0.77f, 0.84f);

            t_buttonMaterialsMiddle.m_vanillaMaterial = t_meshManager.BuildMaterialInfo("ANB_Menu_BTN_CANCEL.dds");
            t_buttonMaterialsMiddle.m_highLightedMaterial = t_meshManager.BuildMaterialInfo("ANB_Menu_BTN_CANCELl_Highlight.dds");
            t_buttonMaterialsMiddle.m_selectedLightedMaterial = t_meshManager.BuildMaterialInfo("ANB_Menu_BTN_CANCEL_Clicked.dds");

            t_spriteInfoMiddleButton = t_meshManager.BuildSpriteInfo(t_data);


            m_cancelButton = Button(t_buttonMaterialsMiddle, t_spriteInfoMiddleButton, DoremiButtonActions::OPTIONS_CANCEL);

            m_optionsButtons.push_back(&m_cancelButton);
        }

        void OptionsHandler::CreateScreenResolutionOption(const DoremiEngine::Core::SharedContext& p_sharedContext)
        {
            DoremiEngine::Graphic::MeshManager& t_meshManager = p_sharedContext.GetGraphicModule().GetSubModuleManager().GetMeshManager();

            // Create the square you press to get the dropdown

            // Basic position
            DoremiEngine::Graphic::SpriteData t_data;

            t_data.halfsize = XMFLOAT2(0.05f, 0.015f);
            t_data.origo = XMFLOAT2(0.0f, 0.0f);
            t_data.position = XMFLOAT2(0.75f, 0.289f);
            t_data.txtPos = XMFLOAT2(0.0f, 0.0f);
            t_data.txtSize = XMFLOAT2(1.0f, 1.0f);

            // Skapa en buttonmaterial struct. Denna h�ller 2 buildmaterialinfos f�r att g�ra kortare parameterlistor
            Doremi::Core::ButtonMaterials t_buttonMaterialsMiddle;

            // Load materials for the actual square to show resoluton
            t_buttonMaterialsMiddle.m_vanillaMaterial = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Inactive.dds");
            t_buttonMaterialsMiddle.m_highLightedMaterial = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Highlight.dds");
            t_buttonMaterialsMiddle.m_selectedLightedMaterial = nullptr;

            DoremiEngine::Graphic::SpriteInfo* t_spriteInfoMiddleButton = t_meshManager.BuildSpriteInfo(t_data);

            m_resolutionItem.button = Button(t_buttonMaterialsMiddle, t_spriteInfoMiddleButton, DoremiButtonActions::RESOLUTION_PRESS, XMFLOAT2(0, 0.0065f));


            // Create the overlaying text
            DoremiEngine::Graphic::MaterialInfo* t_matinfoText = t_meshManager.BuildMaterialInfo("FontNormal.dds");

            XMFLOAT2 t_tableCharSize = XMFLOAT2(0.0625f, 0.0714285714285714f);
            m_resolutionItem.text =
                Text(t_matinfoText, XMFLOAT2(0.006f, 0.009f), XMFLOAT2(0.75f, 0.289f), XMFLOAT2(0.0f, 0.0f), t_tableCharSize, XMFLOAT2(0.0f, 0.0f));
            m_resolutionItem.text.SetText(p_sharedContext, std::to_string(m_currentResolutionWidth) + "x" + std::to_string(m_currentResolutionHeight));

            m_basicItems.push_back(&m_resolutionItem);
        }

        void OptionsHandler::CreateRefreshOption(const DoremiEngine::Core::SharedContext& p_sharedContext)
        {
            DoremiEngine::Graphic::MeshManager& t_meshManager = p_sharedContext.GetGraphicModule().GetSubModuleManager().GetMeshManager();

            // Create the square you press to get the dropdown

            // Basic position
            DoremiEngine::Graphic::SpriteData t_data;

            t_data.halfsize = XMFLOAT2(0.05f, 0.015f);
            t_data.origo = XMFLOAT2(0.0f, 0.0f);
            t_data.position = XMFLOAT2(0.75f, 0.335f);
            t_data.txtPos = XMFLOAT2(0.0f, 0.0f);
            t_data.txtSize = XMFLOAT2(1.0f, 1.0f);

            // Skapa en buttonmaterial struct. Denna h�ller 2 buildmaterialinfos f�r att g�ra kortare parameterlistor
            Doremi::Core::ButtonMaterials t_buttonMaterialsMiddle;

            // Load materials for the actual square to show resoluton
            t_buttonMaterialsMiddle.m_vanillaMaterial = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Inactive.dds");
            t_buttonMaterialsMiddle.m_highLightedMaterial = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Highlight.dds");
            t_buttonMaterialsMiddle.m_selectedLightedMaterial = nullptr;

            DoremiEngine::Graphic::SpriteInfo* t_spriteInfoMiddleButton = t_meshManager.BuildSpriteInfo(t_data);

            m_refreshItem.button = Button(t_buttonMaterialsMiddle, t_spriteInfoMiddleButton, DoremiButtonActions::REFRESH_PRESS, XMFLOAT2(0, 0.0065f));


            // Create the overlaying text
            DoremiEngine::Graphic::MaterialInfo* t_matinfoText = t_meshManager.BuildMaterialInfo("FontNormal.dds");

            XMFLOAT2 t_tableCharSize = XMFLOAT2(0.0625f, 0.0714285714285714f);
            m_refreshItem.text =
                Text(t_matinfoText, XMFLOAT2(0.006f, 0.009f), XMFLOAT2(0.75f, 0.335f), XMFLOAT2(0.0f, 0.0f), t_tableCharSize, XMFLOAT2(0.0f, 0.0f));
            m_refreshItem.text.SetText(p_sharedContext, std::to_string(m_currentRefreshRate));

            m_basicItems.push_back(&m_refreshItem);
        }

        void OptionsHandler::CreateMonitorOption(const DoremiEngine::Core::SharedContext& p_sharedContext)
        {
            DoremiEngine::Graphic::MeshManager& t_meshManager = p_sharedContext.GetGraphicModule().GetSubModuleManager().GetMeshManager();

            // Create the square you press to get the dropdown

            // Basic position
            DoremiEngine::Graphic::SpriteData t_data;

            t_data.halfsize = XMFLOAT2(0.05f, 0.015f);
            t_data.origo = XMFLOAT2(0.0f, 0.0f);
            t_data.position = XMFLOAT2(0.75f, 0.384f);
            t_data.txtPos = XMFLOAT2(0.0f, 0.0f);
            t_data.txtSize = XMFLOAT2(1.0f, 1.0f);

            // Skapa en buttonmaterial struct. Denna h�ller 2 buildmaterialinfos f�r att g�ra kortare parameterlistor
            Doremi::Core::ButtonMaterials t_buttonMaterialsMiddle;

            // Load materials for the actual square to show resoluton
            t_buttonMaterialsMiddle.m_vanillaMaterial = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Inactive.dds");
            t_buttonMaterialsMiddle.m_highLightedMaterial = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Highlight.dds");
            t_buttonMaterialsMiddle.m_selectedLightedMaterial = nullptr;

            DoremiEngine::Graphic::SpriteInfo* t_spriteInfoMiddleButton = t_meshManager.BuildSpriteInfo(t_data);

            m_monitorItem.button = Button(t_buttonMaterialsMiddle, t_spriteInfoMiddleButton, DoremiButtonActions::MONITOR_PRESS, XMFLOAT2(0, 0.0065f));


            // Create the overlaying text
            DoremiEngine::Graphic::MaterialInfo* t_matinfoText = t_meshManager.BuildMaterialInfo("FontNormal.dds");

            XMFLOAT2 t_tableCharSize = XMFLOAT2(0.0625f, 0.0714285714285714f);
            m_monitorItem.text =
                Text(t_matinfoText, XMFLOAT2(0.006f, 0.009f), XMFLOAT2(0.75f, 0.384f), XMFLOAT2(0.0f, 0.0f), t_tableCharSize, XMFLOAT2(0.0f, 0.0f));
            m_monitorItem.text.SetText(p_sharedContext, std::to_string(m_currentMonitor));

            m_basicItems.push_back(&m_monitorItem);
        }

        void OptionsHandler::CreateSliders(const DoremiEngine::Core::SharedContext& p_sharedContext)
        {
            DoremiEngine::Graphic::MeshManager& t_meshManager = p_sharedContext.GetGraphicModule().GetSubModuleManager().GetMeshManager();
            DoremiEngine::Configuration::ConfiguartionInfo configInfo = p_sharedContext.GetConfigurationModule().GetAllConfigurationValues();

            // Basic position
            DoremiEngine::Graphic::SpriteData t_dataBack;
            DoremiEngine::Graphic::SpriteData t_dataCircle;

            // 0.046 is good number
            t_dataBack.halfsize = XMFLOAT2(0.08f, 0.02f);
            t_dataBack.origo = XMFLOAT2(0.0f, 0.0f);
            t_dataBack.position = XMFLOAT2(0.75f, 0.43f);
            t_dataBack.txtPos = XMFLOAT2(0.0f, 0.0f);
            t_dataBack.txtSize = XMFLOAT2(1.0f, 1.0f);

            t_dataCircle.halfsize = XMFLOAT2(0.006f, 0.0105f);
            t_dataCircle.origo = XMFLOAT2(0.0f, 0.0f);
            t_dataCircle.position = XMFLOAT2(0.75f, 0.43f);
            t_dataCircle.txtPos = XMFLOAT2(0.0f, 0.0f);
            t_dataCircle.txtSize = XMFLOAT2(1.0f, 1.0f);

            DoremiEngine::Graphic::MaterialInfo* t_matInfoSliderBack = t_meshManager.BuildMaterialInfo("ANB_Menu_SliderBar.dds");
            DoremiEngine::Graphic::MaterialInfo* t_matInfoSliderCircle = t_meshManager.BuildMaterialInfo("ANB_Menu_SliderCircle.dds");
            DoremiEngine::Graphic::SpriteInfo* t_spriteInfoSliderBack = t_meshManager.BuildSpriteInfo(t_dataBack);
            DoremiEngine::Graphic::SpriteInfo* t_spriteInfoSliderCircle = t_meshManager.BuildSpriteInfo(t_dataCircle);


            // TODO get Fov here
            float t_fov = static_cast<float>(configInfo.CameraFieldOfView - 60) / (90.0f - 60.0f);
            t_fov = std::max(t_fov, 0.0f);
            t_fov = std::min(t_fov, 1.0f);
            Slider* t_newSlider =
                new Slider(SliderEffect::FIELD_OF_VIEW, t_matInfoSliderBack, t_spriteInfoSliderBack, t_matInfoSliderCircle, t_spriteInfoSliderCircle);
            t_newSlider->UpdateSlider(t_fov);
            m_sliders.push_back(t_newSlider);

            // Audio
            t_dataBack.position.y = 0.575f;
            t_dataCircle.position.y = 0.575f;
            t_spriteInfoSliderBack = t_meshManager.BuildSpriteInfo(t_dataBack);
            t_spriteInfoSliderCircle = t_meshManager.BuildSpriteInfo(t_dataCircle);
            float t_audioMaster = configInfo.MasterVolume;
            t_newSlider = new Slider(SliderEffect::SOUND_MASTER, t_matInfoSliderBack, t_spriteInfoSliderBack, t_matInfoSliderCircle, t_spriteInfoSliderCircle);
            t_newSlider->UpdateSlider(t_audioMaster);
            m_sliders.push_back(t_newSlider);

            // Audio
            t_dataBack.position.y = 0.625f;
            t_dataCircle.position.y = 0.625f;
            t_spriteInfoSliderBack = t_meshManager.BuildSpriteInfo(t_dataBack);
            t_spriteInfoSliderCircle = t_meshManager.BuildSpriteInfo(t_dataCircle);
            float t_audioEffect = configInfo.EffectVolume;
            t_newSlider = new Slider(SliderEffect::SOUND_EFFECTS, t_matInfoSliderBack, t_spriteInfoSliderBack, t_matInfoSliderCircle, t_spriteInfoSliderCircle);
            t_newSlider->UpdateSlider(t_audioEffect);
            m_sliders.push_back(t_newSlider);


            // Audio
            t_dataBack.position.y = 0.674f;
            t_dataCircle.position.y = 0.674f;
            t_spriteInfoSliderBack = t_meshManager.BuildSpriteInfo(t_dataBack);
            t_spriteInfoSliderCircle = t_meshManager.BuildSpriteInfo(t_dataCircle);
            float t_audioMusic = configInfo.MusicVolume;
            t_newSlider = new Slider(SliderEffect::SOUND_MUSIC, t_matInfoSliderBack, t_spriteInfoSliderBack, t_matInfoSliderCircle, t_spriteInfoSliderCircle);
            t_newSlider->UpdateSlider(t_audioMusic);
            m_sliders.push_back(t_newSlider);

            // Audio, cut off amp
            t_dataBack.position.y = 0.725f;
            t_dataCircle.position.y = 0.725f;
            t_spriteInfoSliderBack = t_meshManager.BuildSpriteInfo(t_dataBack);
            t_spriteInfoSliderCircle = t_meshManager.BuildSpriteInfo(t_dataCircle);
            float t_cutOff = configInfo.AmplitudeCutOff;
            t_newSlider = new Slider(SliderEffect::RECORDING_CUTOFFAMPLITUDE, t_matInfoSliderBack, t_spriteInfoSliderBack, t_matInfoSliderCircle,
                                     t_spriteInfoSliderCircle);
            t_newSlider->UpdateSlider(t_cutOff);
            m_sliders.push_back(t_newSlider);

            // Mouse sence
            t_dataBack.position.y = 0.770f;
            t_dataCircle.position.y = 0.770f;
            t_spriteInfoSliderBack = t_meshManager.BuildSpriteInfo(t_dataBack);
            t_spriteInfoSliderCircle = t_meshManager.BuildSpriteInfo(t_dataCircle);
            float t_sense = (configInfo.TurnSpeed - 0.001f) / (0.019f);
            t_newSlider = new Slider(SliderEffect::MOUSE_SENSE, t_matInfoSliderBack, t_spriteInfoSliderBack, t_matInfoSliderCircle, t_spriteInfoSliderCircle);
            t_newSlider->UpdateSlider(t_sense);
            m_sliders.push_back(t_newSlider);
        }

        void OptionsHandler::Update(double p_dt)
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
            m_highlightedButton = nullptr;


            // Check dropdowns first
            size_t length = m_dropdownResolution.size();
            for(size_t i = 0; i < length; i++)
            {
                if(m_dropdownResolution[i].button.CheckIfInside(t_mouseScreenPosX, t_mouseScreenPosY))
                {
                    // Since stupied design of check inside also update texture we check if nothing is highlighted here
                    if(m_highlightedButton == nullptr)
                    {
                        m_highlightedButton = &m_dropdownResolution[i].button;
                        m_highlightedWidth = m_dropdownResolution[i].width;
                        m_highlightedHegiht = m_dropdownResolution[i].height;
                    }
                }
            }

            // Check dropdowns first
            length = m_dropdownRefresh.size();
            for(size_t i = 0; i < length; i++)
            {
                if(m_dropdownRefresh[i].button.CheckIfInside(t_mouseScreenPosX, t_mouseScreenPosY))
                {
                    // Since stupied design of check inside also update texture we check if nothing is highlighted here
                    if(m_highlightedButton == nullptr)
                    {
                        m_highlightedButton = &m_dropdownRefresh[i].button;
                        m_highlightedRefreshrate = m_dropdownRefresh[i].refreshRate;
                    }
                }
            }

            // Check dropdowns first
            length = m_dropdownMonitors.size();
            for(size_t i = 0; i < length; i++)
            {
                if(m_dropdownMonitors[i].button.CheckIfInside(t_mouseScreenPosX, t_mouseScreenPosY))
                {
                    // Since stupied design of check inside also update texture we check if nothing is highlighted here
                    if(m_highlightedButton == nullptr)
                    {
                        m_highlightedButton = &m_dropdownMonitors[i].button;
                        m_highlightedMonitorindex = m_dropdownMonitors[i].monitorIndex;
                    }
                }
            }


            // Check normal buttons later
            length = m_optionsButtons.size();
            for(size_t i = 0; i < length; i++)
            {
                if(m_optionsButtons[i]->CheckIfInside(t_mouseScreenPosX, t_mouseScreenPosY))
                {
                    // Since stupied design of check inside also update texture we check if nothing is highlighted here
                    if(m_highlightedButton == nullptr)
                    {
                        m_highlightedButton = m_optionsButtons[i];
                    }
                }
            }

            // Check checkboxes

            // Since stupied design of check inside also update texture we check if nothing is highlighted here
            m_fullscreenCheckbox.CheckIfInside(t_mouseScreenPosX, t_mouseScreenPosY);


            length = m_basicItems.size();
            for(size_t i = 0; i < length; i++)
            {
                if(m_basicItems[i]->button.CheckIfInside(t_mouseScreenPosX, t_mouseScreenPosY))
                {
                    // Since stupied design of check inside also update texture we check if nothing is highlighted here
                    if(m_highlightedButton == nullptr)
                    {
                        m_highlightedButton = &m_basicItems[i]->button;
                    }
                }
            }


            // Check if we're pressing
            if(t_inputHandler->CheckForOnePress((int)UserCommandPlaying::LeftClick))
            {
                if(m_highlightedButton != nullptr)
                {
                    switch(m_highlightedButton->m_buttonAction)
                    {
                        case DoremiButtonActions::SET_FULLSCREEN:
                        {

                            break;
                        }
                        case DoremiButtonActions::RESOLUTION_PRESS:
                        {
                            // Swap resolution dropdown
                            t_resolutionDropdownIsActive = !t_resolutionDropdownIsActive;
                            t_refreshDropdownIsActive = false;
                            t_monitorDropdownIsActive = false;
                            ClearRefreshDropDown();
                            ClearMonitorDropDown();

                            // Set all other dropdowns false

                            // If we should show them now, create them
                            if(t_resolutionDropdownIsActive)
                            {
                                CreateResolutionDropDown();
                            }
                            else // else remove them
                            {
                                ClearResolutionDropDown();
                            }

                            break;
                        }
                        case DoremiButtonActions::RESOLUTION_SELECT:
                        {
                            m_currentResolutionHeight = m_highlightedHegiht;
                            m_currentResolutionWidth = m_highlightedWidth;
                            m_resolutionItem.text.UpdateText(m_sharedContext, std::to_string(m_highlightedWidth) + "x" + std::to_string(m_highlightedHegiht));

                            t_resolutionDropdownIsActive = false;
                            ClearResolutionDropDown();
                            UpdateRefreshRate();
                            m_highlightedButton = nullptr;
                            break;
                        }
                        case DoremiButtonActions::REFRESH_PRESS:
                        {
                            // Swap resolution dropdown
                            t_refreshDropdownIsActive = !t_refreshDropdownIsActive;
                            t_resolutionDropdownIsActive = false;
                            t_monitorDropdownIsActive = false;
                            ClearResolutionDropDown();
                            ClearMonitorDropDown();

                            // Set all other dropdowns false

                            // If we should show them now, create them
                            if(t_refreshDropdownIsActive)
                            {
                                CreateRefreshRateDropDown();
                            }
                            else // else remove them
                            {
                                ClearRefreshDropDown();
                            }

                            break;
                        }
                        case DoremiButtonActions::REFRESH_SELECT:
                        {
                            m_currentRefreshRate = m_highlightedRefreshrate;
                            m_refreshItem.text.UpdateText(m_sharedContext, std::to_string(m_highlightedRefreshrate));

                            t_refreshDropdownIsActive = false;
                            ClearRefreshDropDown();
                            m_highlightedButton = nullptr;
                            break;
                        }
                        case DoremiButtonActions::MONITOR_PRESS:
                        {
                            // Swap
                            t_monitorDropdownIsActive = !t_monitorDropdownIsActive;
                            t_resolutionDropdownIsActive = false;
                            t_refreshDropdownIsActive = false;
                            ClearResolutionDropDown();
                            ClearResolutionDropDown();

                            // If we should show them now, create them
                            if(t_monitorDropdownIsActive)
                            {
                                CreateMonitorDropDown();
                            }
                            else // else remove them
                            {
                                ClearMonitorDropDown();
                            }

                            break;
                        }
                        case DoremiButtonActions::MONITOR_SELECT:
                        {
                            m_currentMonitor = m_highlightedMonitorindex;
                            m_monitorItem.text.UpdateText(m_sharedContext, std::to_string(m_currentMonitor));

                            t_monitorDropdownIsActive = false;
                            ClearMonitorDropDown();
                            UpdateResolution();
                            UpdateRefreshRate();
                            m_highlightedButton = nullptr;
                            break;
                        }
                        case DoremiButtonActions::OPTIONS_APPLY:
                        {
                            ChangeMenuState* t_newEvent = new ChangeMenuState();
                            t_newEvent->state = DoremiGameStates::MAINMENU;


                            EventHandler::GetInstance()->BroadcastEvent(t_newEvent);

                            ClearMonitorDropDown();
                            ClearRefreshDropDown();
                            ClearResolutionDropDown();
                            t_monitorDropdownIsActive = false;
                            t_refreshDropdownIsActive = false;
                            t_resolutionDropdownIsActive = false;

                            std::pair<uint32_t, uint32_t> t_resolution;
                            t_resolution.first = m_currentResolutionWidth;
                            t_resolution.second = m_currentResolutionHeight;

                            m_sharedContext.GetGraphicModule().GetSubModuleManager().GetDirectXManager().SetMonitor(m_currentMonitor);
                            m_sharedContext.GetGraphicModule().GetSubModuleManager().GetDirectXManager().SetResolution(t_resolution);
                            m_sharedContext.GetGraphicModule().GetSubModuleManager().GetDirectXManager().SetRefreshRate(m_currentRefreshRate);
                            m_sharedContext.GetGraphicModule().GetSubModuleManager().GetDirectXManager().SetFullscreen(m_fullscreenCheckbox.m_isPressed);


                            break;
                        }
                        case DoremiButtonActions::OPTIONS_CANCEL:
                        {
                            ChangeMenuState* t_newEvent = new ChangeMenuState();
                            t_newEvent->state = DoremiGameStates::MAINMENU;


                            EventHandler::GetInstance()->BroadcastEvent(t_newEvent);

                            ClearMonitorDropDown();
                            ClearRefreshDropDown();
                            ClearResolutionDropDown();
                            t_monitorDropdownIsActive = false;
                            t_refreshDropdownIsActive = false;
                            t_resolutionDropdownIsActive = false;

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
                    // check sliders
                    length = m_sliders.size();
                    for(size_t i = 0; i < length; i++)
                    {
                        if(m_sliders[i]->CheckIfInside(t_mouseScreenPosX, t_mouseScreenPosY))
                        {
                            m_sliders[i]->UpdateSliderByMousePos(t_mouseScreenPosX);
                            UpdateSliderEffect(m_sliders[i]->m_slideEffect, m_sliders[i]->m_percent);
                        }
                    }

                    // Check checkboxes
                    if(m_fullscreenCheckbox.CheckIfInside(t_mouseScreenPosX, t_mouseScreenPosY))
                    {
                        m_fullscreenCheckbox.TogglePress();
                    }


                    // Deselect stuff
                    ClearMonitorDropDown();
                    ClearRefreshDropDown();
                    ClearResolutionDropDown();
                    t_monitorDropdownIsActive = false;
                    t_refreshDropdownIsActive = false;
                    t_resolutionDropdownIsActive = false;
                }
            }
            else
            {
                // Nothing
            }


            if(t_inputHandler->CheckBitMaskInputFromGame((int)UserCommandPlaying::LeftClick))
            {
                if(m_highlightedButton == nullptr)
                {
                    // check sliders
                    length = m_sliders.size();
                    for(size_t i = 0; i < length; i++)
                    {
                        if(m_sliders[i]->m_isActive)
                        {
                            if(m_sliders[i]->CheckIfInside(t_mouseScreenPosX, t_mouseScreenPosY))
                            {
                                m_sliders[i]->UpdateSliderByMousePos(t_mouseScreenPosX);
                                UpdateSliderEffect(m_sliders[i]->m_slideEffect, m_sliders[i]->m_percent);
                            }
                        }
                    }
                }
            }

            // Check if we currently have a frequency to determine if the player is effecting
            float amplitudeCutOff = m_sharedContext.GetConfigurationModule().GetAllConfigurationValues().AmplitudeCutOff;
            if(AudioHandler::GetInstance()->GetAmplitudeOfRecording() > amplitudeCutOff)
            {
                m_recordingHearing.SetCurrentObject(0);
            }
            else
            {
                m_recordingHearing.SetCurrentObject(1);
            }
            m_soundBar.UpdateProgress(AudioHandler::GetInstance()->GetAmplitudeOfRecording());
            // Check to exit TODO move place of code
            if(t_inputHandler->CheckForOnePress(static_cast<uint32_t>(UserCommandPlaying::ExitGame)))
            {
                ChangeMenuState* t_newEvent = new ChangeMenuState();
                t_newEvent->state = DoremiGameStates::MAINMENU;


                EventHandler::GetInstance()->BroadcastEvent(t_newEvent);

                ClearMonitorDropDown();
                ClearRefreshDropDown();
                ClearResolutionDropDown();
                t_monitorDropdownIsActive = false;
                t_refreshDropdownIsActive = false;
                t_resolutionDropdownIsActive = false;
            }
        }

        void OptionsHandler::CreateResolutionDropDown()
        {
            DoremiEngine::Graphic::DirectXManager& t_directxManager = m_sharedContext.GetGraphicModule().GetSubModuleManager().GetDirectXManager();
            DoremiEngine::Graphic::MeshManager& t_meshManager = m_sharedContext.GetGraphicModule().GetSubModuleManager().GetMeshManager();

            // Get resolutions
            std::vector<std::pair<uint32_t, uint32_t>> t_resolutions = t_directxManager.GetResolutions(m_currentMonitor);

            // Basic position
            DoremiEngine::Graphic::SpriteData t_data;

            t_data.halfsize = XMFLOAT2(0.05f, 0.015f);
            t_data.origo = XMFLOAT2(0.0f, 0.0f);
            t_data.position = XMFLOAT2(0.75f, 0.289f);
            t_data.txtPos = XMFLOAT2(0.0f, 0.0f);
            t_data.txtSize = XMFLOAT2(1.0f, 1.0f);

            DoremiEngine::Graphic::MaterialInfo* t_materialNormalSpecial = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Inactive.dds");
            DoremiEngine::Graphic::MaterialInfo* t_materialHoverSpecial = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Highlight.dds");

            DoremiEngine::Graphic::MaterialInfo* t_materialNormalTop = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Top.dds");
            DoremiEngine::Graphic::MaterialInfo* t_materialHoverTop = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Top_Highlight.dds");

            DoremiEngine::Graphic::MaterialInfo* t_materialNormalMiddle = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Middle.dds");
            DoremiEngine::Graphic::MaterialInfo* t_materialHoverMiddle = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Middle_Highlight.dds");

            DoremiEngine::Graphic::MaterialInfo* t_materialNormalBot = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Bottom.dds");
            DoremiEngine::Graphic::MaterialInfo* t_materialHoverBot = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Bottom_Highlight.dds");

            DoremiEngine::Graphic::MaterialInfo* t_matinfoText = t_meshManager.BuildMaterialInfo("FontNormal.dds");

            XMFLOAT2 t_startOffset = XMFLOAT2(0.75f, 0.289f);
            float t_offset = 0.0f;
            auto t_reverseRes = t_resolutions.rbegin();
            for(; t_reverseRes != t_resolutions.rend(); ++t_reverseRes)
            {
                t_offset += 1.0f;
                t_data.position = XMFLOAT2(t_startOffset.x, t_startOffset.y + 0.009f * 2.0f * t_offset);

                // Skapa en buttonmaterial struct. Denna h�ller 2 buildmaterialinfos f�r att g�ra kortare parameterlistor
                Doremi::Core::ButtonMaterials t_buttonMaterialsMiddle;

                // Load materials for the actual square to show resoluton
                t_buttonMaterialsMiddle.m_vanillaMaterial = t_materialNormalMiddle;
                t_buttonMaterialsMiddle.m_highLightedMaterial = t_materialHoverMiddle;
                t_buttonMaterialsMiddle.m_selectedLightedMaterial = nullptr;

                DoremiEngine::Graphic::SpriteInfo* t_spriteInfoMiddleButton = t_meshManager.BuildSpriteInfo(t_data);

                Button m_resolutionButton =
                    Button(t_buttonMaterialsMiddle, t_spriteInfoMiddleButton, DoremiButtonActions::RESOLUTION_SELECT, XMFLOAT2(0, 0.0065f));


                XMFLOAT2 t_tableCharSize = XMFLOAT2(0.0625f, 0.0714285714285714f);
                Text m_resolutionText = Text(t_matinfoText, XMFLOAT2(0.006f, 0.009f), XMFLOAT2(t_startOffset.x, t_startOffset.y + 0.009f * 2.0f * t_offset),
                                             XMFLOAT2(0.0f, 0.0f), t_tableCharSize, XMFLOAT2(0.0f, 0.0f));

                m_resolutionText.SetText(m_sharedContext, std::to_string(t_reverseRes->first) + "x" + std::to_string(t_reverseRes->second));

                DropDownResolution t_newDropDownItem;
                t_newDropDownItem.button = m_resolutionButton;
                t_newDropDownItem.text = m_resolutionText;
                t_newDropDownItem.width = t_reverseRes->first;
                t_newDropDownItem.height = t_reverseRes->second;

                m_dropdownResolution.push_back(t_newDropDownItem);
            }

            if(m_dropdownResolution.size())
            {
                if(m_dropdownResolution.size() == 1)
                {
                    m_dropdownResolution[0].button.m_buttonMaterials.m_vanillaMaterial = t_materialNormalSpecial;
                    m_dropdownResolution[0].button.m_buttonMaterials.m_highLightedMaterial = t_materialHoverSpecial;
                }
                else
                {
                    m_dropdownResolution.begin()->button.m_buttonMaterials.m_vanillaMaterial = t_materialNormalTop;
                    m_dropdownResolution.begin()->button.m_buttonMaterials.m_highLightedMaterial = t_materialHoverTop;

                    m_dropdownResolution.rbegin()->button.m_buttonMaterials.m_vanillaMaterial = t_materialNormalBot;
                    m_dropdownResolution.rbegin()->button.m_buttonMaterials.m_highLightedMaterial = t_materialHoverBot;
                }
            }
        }

        void OptionsHandler::CreateRefreshRateDropDown()
        {
            DoremiEngine::Graphic::DirectXManager& t_directxManager = m_sharedContext.GetGraphicModule().GetSubModuleManager().GetDirectXManager();
            DoremiEngine::Graphic::MeshManager& t_meshManager = m_sharedContext.GetGraphicModule().GetSubModuleManager().GetMeshManager();

            // Create resolution pair
            std::pair<uint32_t, uint32_t> t_resolution;
            t_resolution.first = m_currentResolutionWidth;
            t_resolution.second = m_currentResolutionHeight;

            // Get resolutions
            std::vector<uint32_t> t_refreshRates = t_directxManager.GetRefreshRates(m_currentMonitor, t_resolution);

            // Basic position
            DoremiEngine::Graphic::SpriteData t_data;

            t_data.halfsize = XMFLOAT2(0.05f, 0.015f);
            t_data.origo = XMFLOAT2(0.0f, 0.0f);
            t_data.position = XMFLOAT2(0.75f, 0.335f);
            t_data.txtPos = XMFLOAT2(0.0f, 0.0f);
            t_data.txtSize = XMFLOAT2(1.0f, 1.0f);


            DoremiEngine::Graphic::MaterialInfo* t_materialNormalSpecial = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Inactive.dds");
            DoremiEngine::Graphic::MaterialInfo* t_materialHoverSpecial = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Highlight.dds");

            DoremiEngine::Graphic::MaterialInfo* t_materialNormalTop = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Top.dds");
            DoremiEngine::Graphic::MaterialInfo* t_materialHoverTop = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Top_Highlight.dds");

            DoremiEngine::Graphic::MaterialInfo* t_materialNormalMiddle = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Middle.dds");
            DoremiEngine::Graphic::MaterialInfo* t_materialHoverMiddle = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Middle_Highlight.dds");

            DoremiEngine::Graphic::MaterialInfo* t_materialNormalBot = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Bottom.dds");
            DoremiEngine::Graphic::MaterialInfo* t_materialHoverBot = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Bottom_Highlight.dds");

            DoremiEngine::Graphic::MaterialInfo* t_matinfoText = t_meshManager.BuildMaterialInfo("FontNormal.dds");

            XMFLOAT2 t_startOffset = XMFLOAT2(0.75f, 0.335f);

            float t_offset = 0.0f;
            auto t_reverseRefresh = t_refreshRates.begin();
            for(; t_reverseRefresh != t_refreshRates.end(); ++t_reverseRefresh)
            {
                t_offset += 1.0f;
                t_data.position = XMFLOAT2(t_startOffset.x, t_startOffset.y + 0.009f * 2.0f * t_offset);

                // Skapa en buttonmaterial struct. Denna h�ller 2 buildmaterialinfos f�r att g�ra kortare parameterlistor
                Doremi::Core::ButtonMaterials t_buttonMaterialsMiddle;

                // Load materials for the actual square to show resoluton
                t_buttonMaterialsMiddle.m_vanillaMaterial = t_materialNormalMiddle;
                t_buttonMaterialsMiddle.m_highLightedMaterial = t_materialHoverMiddle;
                t_buttonMaterialsMiddle.m_selectedLightedMaterial = nullptr;

                DoremiEngine::Graphic::SpriteInfo* t_spriteInfoMiddleButton = t_meshManager.BuildSpriteInfo(t_data);

                Button m_resolutionButton =
                    Button(t_buttonMaterialsMiddle, t_spriteInfoMiddleButton, DoremiButtonActions::REFRESH_SELECT, XMFLOAT2(0, 0.0065f));


                XMFLOAT2 t_tableCharSize = XMFLOAT2(0.0625f, 0.0714285714285714f);
                Text m_resolutionText = Text(t_matinfoText, XMFLOAT2(0.006f, 0.009f), XMFLOAT2(t_startOffset.x, t_startOffset.y + 0.009f * 2.0f * t_offset),
                                             XMFLOAT2(0.0f, 0.0f), t_tableCharSize, XMFLOAT2(0.0f, 0.0f));

                m_resolutionText.SetText(m_sharedContext, std::to_string(*t_reverseRefresh));

                DropDownRefreshRate t_newDropDownItem;
                t_newDropDownItem.button = m_resolutionButton;
                t_newDropDownItem.text = m_resolutionText;
                t_newDropDownItem.refreshRate = *t_reverseRefresh;


                m_dropdownRefresh.push_back(t_newDropDownItem);
            }

            if(m_dropdownRefresh.size())
            {
                if(m_dropdownRefresh.size() == 1)
                {
                    m_dropdownRefresh[0].button.m_buttonMaterials.m_vanillaMaterial = t_materialNormalSpecial;
                    m_dropdownRefresh[0].button.m_buttonMaterials.m_highLightedMaterial = t_materialHoverSpecial;
                }
                else
                {
                    m_dropdownRefresh.begin()->button.m_buttonMaterials.m_vanillaMaterial = t_materialNormalTop;
                    m_dropdownRefresh.begin()->button.m_buttonMaterials.m_highLightedMaterial = t_materialHoverTop;

                    m_dropdownRefresh.rbegin()->button.m_buttonMaterials.m_vanillaMaterial = t_materialNormalBot;
                    m_dropdownRefresh.rbegin()->button.m_buttonMaterials.m_highLightedMaterial = t_materialHoverBot;
                }
            }
        }

        void OptionsHandler::ClearResolutionDropDown()
        {
            for(auto& t_dropdownItem : m_dropdownResolution)
            {
                delete t_dropdownItem.button.m_spriteInfo;
                t_dropdownItem.text.DeleteText();
            }

            m_dropdownResolution.clear();
        }

        void OptionsHandler::UpdateResolution()
        {
            DoremiEngine::Graphic::DirectXManager& t_directxManager = m_sharedContext.GetGraphicModule().GetSubModuleManager().GetDirectXManager();

            std::vector<std::pair<uint32_t, uint32_t>> t_resolutions = t_directxManager.GetResolutions(m_currentMonitor);

            std::pair<uint32_t, uint32_t> t_resolution;
            t_resolution.first = m_currentResolutionWidth;
            t_resolution.second = m_currentResolutionHeight;

            auto iterator = std::find(t_resolutions.begin(), t_resolutions.end(), t_resolution);
            if(iterator == t_resolutions.end())
            {
                if(t_resolutions.size())
                {
                    m_currentResolutionWidth = (*t_resolutions.rbegin()).first;
                    m_currentResolutionHeight = (*t_resolutions.rbegin()).second;
                    m_resolutionItem.text.UpdateText(m_sharedContext, std::to_string(m_currentResolutionWidth) + "x" + std::to_string(m_currentResolutionHeight));
                }
            }
        }

        void OptionsHandler::UpdateRefreshRate()
        {
            DoremiEngine::Graphic::DirectXManager& t_directxManager = m_sharedContext.GetGraphicModule().GetSubModuleManager().GetDirectXManager();

            std::pair<uint32_t, uint32_t> t_resolution;
            t_resolution.first = m_currentResolutionWidth;
            t_resolution.second = m_currentResolutionHeight;

            std::vector<uint32_t> t_refreshRates = t_directxManager.GetRefreshRates(m_currentMonitor, t_resolution);

            auto iterator = std::find(t_refreshRates.begin(), t_refreshRates.end(), m_currentRefreshRate);
            if(iterator == t_refreshRates.end())
            {
                if(t_refreshRates.size())
                {
                    m_currentRefreshRate = *(t_refreshRates.begin());
                    m_refreshItem.text.UpdateText(m_sharedContext, std::to_string(m_currentRefreshRate));
                }
            }
        }

        void OptionsHandler::ClearRefreshDropDown()
        {
            for(auto& t_dropdownItem : m_dropdownRefresh)
            {
                delete t_dropdownItem.button.m_spriteInfo;
                t_dropdownItem.text.DeleteText();
            }

            m_dropdownRefresh.clear();
        }

        void OptionsHandler::CreateMonitorDropDown()
        {
            DoremiEngine::Graphic::DirectXManager& t_directxManager = m_sharedContext.GetGraphicModule().GetSubModuleManager().GetDirectXManager();
            DoremiEngine::Graphic::MeshManager& t_meshManager = m_sharedContext.GetGraphicModule().GetSubModuleManager().GetMeshManager();

            // Create resolution pair
            std::pair<uint32_t, uint32_t> t_resolution;
            t_resolution.first = m_currentResolutionWidth;
            t_resolution.second = m_currentResolutionHeight;

            // Get resolutions
            uint32_t t_numOfMonitors = t_directxManager.GetNumberOfMonitors();

            // Basic position
            DoremiEngine::Graphic::SpriteData t_data;

            t_data.halfsize = XMFLOAT2(0.05f, 0.015f);
            t_data.origo = XMFLOAT2(0.0f, 0.0f);
            t_data.position = XMFLOAT2(0.75f, 0.384f);
            t_data.txtPos = XMFLOAT2(0.0f, 0.0f);
            t_data.txtSize = XMFLOAT2(1.0f, 1.0f);


            DoremiEngine::Graphic::MaterialInfo* t_materialNormalSpecial = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Inactive.dds");
            DoremiEngine::Graphic::MaterialInfo* t_materialHoverSpecial = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Highlight.dds");

            DoremiEngine::Graphic::MaterialInfo* t_materialNormalTop = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Top.dds");
            DoremiEngine::Graphic::MaterialInfo* t_materialHoverTop = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Top_Highlight.dds");

            DoremiEngine::Graphic::MaterialInfo* t_materialNormalMiddle = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Middle.dds");
            DoremiEngine::Graphic::MaterialInfo* t_materialHoverMiddle = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Middle_Highlight.dds");

            DoremiEngine::Graphic::MaterialInfo* t_materialNormalBot = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Bottom.dds");
            DoremiEngine::Graphic::MaterialInfo* t_materialHoverBot = t_meshManager.BuildMaterialInfo("ANB_Menu_DROPDOWN_Bottom_Highlight.dds");

            DoremiEngine::Graphic::MaterialInfo* t_matinfoText = t_meshManager.BuildMaterialInfo("FontNormal.dds");

            XMFLOAT2 t_startOffset = XMFLOAT2(0.75f, 0.384f);

            float t_offset = 0.0f;

            for(int i = 0; i < t_numOfMonitors; ++i)
            {
                t_offset += 1.0f;
                t_data.position = XMFLOAT2(t_startOffset.x, t_startOffset.y + 0.009f * 2.0f * t_offset);

                // Skapa en buttonmaterial struct. Denna h�ller 2 buildmaterialinfos f�r att g�ra kortare parameterlistor
                Doremi::Core::ButtonMaterials t_buttonMaterialsMiddle;

                // Load materials for the actual square to show resoluton
                t_buttonMaterialsMiddle.m_vanillaMaterial = t_materialNormalMiddle;
                t_buttonMaterialsMiddle.m_highLightedMaterial = t_materialHoverMiddle;
                t_buttonMaterialsMiddle.m_selectedLightedMaterial = nullptr;

                DoremiEngine::Graphic::SpriteInfo* t_spriteInfoMiddleButton = t_meshManager.BuildSpriteInfo(t_data);

                Button m_resolutionButton =
                    Button(t_buttonMaterialsMiddle, t_spriteInfoMiddleButton, DoremiButtonActions::MONITOR_SELECT, XMFLOAT2(0, 0.0065f));


                XMFLOAT2 t_tableCharSize = XMFLOAT2(0.0625f, 0.0714285714285714f);
                Text m_resolutionText = Text(t_matinfoText, XMFLOAT2(0.006f, 0.009f), XMFLOAT2(t_startOffset.x, t_startOffset.y + 0.009f * 2.0f * t_offset),
                                             XMFLOAT2(0.0f, 0.0f), t_tableCharSize, XMFLOAT2(0.0f, 0.0f));

                m_resolutionText.SetText(m_sharedContext, std::to_string(i));

                DropDownMonitor t_newDropDownItem;
                t_newDropDownItem.button = m_resolutionButton;
                t_newDropDownItem.text = m_resolutionText;
                t_newDropDownItem.monitorIndex = i;


                m_dropdownMonitors.push_back(t_newDropDownItem);
            }

            if(m_dropdownMonitors.size())
            {
                if(m_dropdownMonitors.size() == 1)
                {
                    m_dropdownMonitors[0].button.m_buttonMaterials.m_vanillaMaterial = t_materialNormalSpecial;
                    m_dropdownMonitors[0].button.m_buttonMaterials.m_highLightedMaterial = t_materialHoverSpecial;
                }
                else
                {
                    m_dropdownMonitors.begin()->button.m_buttonMaterials.m_vanillaMaterial = t_materialNormalTop;
                    m_dropdownMonitors.begin()->button.m_buttonMaterials.m_highLightedMaterial = t_materialHoverTop;

                    m_dropdownMonitors.rbegin()->button.m_buttonMaterials.m_vanillaMaterial = t_materialNormalBot;
                    m_dropdownMonitors.rbegin()->button.m_buttonMaterials.m_highLightedMaterial = t_materialHoverBot;
                }
            }
        }

        void OptionsHandler::ClearMonitorDropDown()
        {
            for(auto& t_dropdownItem : m_dropdownMonitors)
            {
                delete t_dropdownItem.button.m_spriteInfo;
                t_dropdownItem.text.DeleteText();
            }

            m_dropdownMonitors.clear();
        }

        void OptionsHandler::UpdateSliderEffect(const SliderEffect& p_sliderEffect, const float& percent)
        {
            DoremiEngine::Configuration::ConfiguartionInfo& configInfo = m_sharedContext.GetConfigurationModule().GetModifiableConfigurationInfo();

            switch(p_sliderEffect)
            {
                case Doremi::Core::SliderEffect::FIELD_OF_VIEW:
                {
                    configInfo.CameraFieldOfView = static_cast<uint32_t>(60.0f + percent * (90.0f - 60.0f));
                    CameraHandler::GetInstance()->UpdateCameraProjection();
                    break;
                }
                case Doremi::Core::SliderEffect::SOUND_MASTER:
                {
                    configInfo.MasterVolume = percent;
                    AudioHandler::GetInstance()->SetMasterVolume(percent);
                    break;
                }
                case Doremi::Core::SliderEffect::SOUND_EFFECTS:
                {
                    configInfo.EffectVolume = percent;
                    AudioHandler::GetInstance()->SetEffectVolume(percent);
                    break;
                }
                case Doremi::Core::SliderEffect::SOUND_MUSIC:
                {
                    configInfo.MusicVolume = percent;
                    AudioHandler::GetInstance()->SetMusicVolume(percent);
                    break;
                }
                case Doremi::Core::SliderEffect::RECORDING_CUTOFFAMPLITUDE:
                {
                    configInfo.AmplitudeCutOff = percent;
                    AudioHandler::GetInstance()->SetAmplitudeCutOff(percent);
                    break;
                }
                case Doremi::Core::SliderEffect::MOUSE_SENSE:
                {
                    configInfo.TurnSpeed = 0.001f + percent * 0.019f;
                    static_cast<PlayerHandlerClient*>(PlayerHandler::GetInstance())->UpdateTurnSpeed();
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }
}