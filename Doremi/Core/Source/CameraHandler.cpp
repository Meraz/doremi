#include <CameraHandler.hpp>
#include <InputHandler.hpp>
#include <CameraClasses/ThirdPersonCamera.hpp>
#include <CameraClasses/FreeLookCamera.hpp>
#include <PlayerHandlerClient.hpp>
#include <InputHandlerClient.hpp>
// Engine
#include <DoremiEngine/Graphic/Include/Interface/Camera/Camera.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Manager/CameraManager.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Manager/SubModuleManager.hpp>
#include <DoremiEngine/Graphic/Include/GraphicModule.hpp>

#include <DoremiEngine/Configuration/Include/ConfigurationModule.hpp>
// Third party


namespace Doremi
{
    namespace Core
    {
        CameraHandler* CameraHandler::m_singleton = nullptr;

        void CameraHandler::StartCameraHandler(const DoremiEngine::Core::SharedContext& p_sharedContext)
        {
            if(m_singleton == nullptr)
            {
                m_singleton = new CameraHandler(p_sharedContext);
            }
        }
        CameraHandler* CameraHandler::GetInstance()
        {
            if(m_singleton == nullptr)
            {
                // TODOKO error message, not started
            }
            return m_singleton;
        }

        CameraHandler::CameraHandler(const DoremiEngine::Core::SharedContext& p_sharedContext) : m_sharedContext(p_sharedContext)
        {
            using namespace DirectX;
            DoremiEngine::Graphic::CameraManager& t_graphicModuleCameraManager = m_sharedContext.GetGraphicModule().GetSubModuleManager().GetCameraManager();
            XMFLOAT4X4 projection;

            DoremiEngine::Configuration::ConfiguartionInfo configInfo = m_sharedContext.GetConfigurationModule().GetAllConfigurationValues();
            XMMATRIX mat = XMMatrixTranspose(XMMatrixPerspectiveFovLH(configInfo.CameraFieldOfView * XM_PI / 180.0f,
                                                                      configInfo.ScreenWidth / configInfo.ScreenHeight, 0.1f, configInfo.CameraViewDistance));


            XMStoreFloat4x4(&projection, mat);

            DoremiEngine::Graphic::Camera* freecamera = t_graphicModuleCameraManager.BuildNewCamera(projection);
            DoremiEngine::Graphic::Camera* thirdPersonCamera = t_graphicModuleCameraManager.BuildNewCamera(projection);
            m_thirdPersonCamera = new ThirdPersonCamera(thirdPersonCamera, configInfo.CameraDistanceFromPlayer);
            m_freeLookCamera = new FreeLookCamera(freecamera);
            m_currentCamera = CameraType::THIRDPERSON;
        }

        CameraHandler::~CameraHandler() {}

        void CameraHandler::UpdateInput(double p_dt)
        {
            PlayerHandlerClient* t_playerHandler = static_cast<PlayerHandlerClient*>(PlayerHandler::GetInstance());
            InputHandlerClient* inputHandler = t_playerHandler->GetInputHandler();

            if(t_playerHandler->PlayerExists())
            {
                if(inputHandler->CheckForOnePress((int)UserCommandPlaying::DebugButton))
                {
                    switch(m_currentCamera)
                    {
                        case CameraType::FREELOOK:
                            ChangeCamera(CameraType::THIRDPERSON);
                            break;
                        case CameraType::THIRDPERSON:
                            ChangeCamera(CameraType::FREELOOK);
                            break;
                        default:
                            break;
                    }
                }
                switch(m_currentCamera)
                {
                    case CameraType::FREELOOK:
                        inputHandler->SetCursorInvisibleAndMiddle(false);
                        m_freeLookCamera->UpdateInput(p_dt);

                        break;
                    case CameraType::THIRDPERSON:
                        inputHandler->SetCursorInvisibleAndMiddle(true);
                        m_thirdPersonCamera->UpdatePositions();
                        break;
                    default:
                        break;
                }
            }
        }

        void CameraHandler::UpdateDraw(const double& p_alpha)
        {
            DoremiEngine::Graphic::CameraManager& t_graphicModuleCameraManager = m_sharedContext.GetGraphicModule().GetSubModuleManager().GetCameraManager();
            PlayerHandlerClient* t_playerHandler = static_cast<PlayerHandlerClient*>(PlayerHandler::GetInstance());

            if(t_playerHandler->PlayerExists())
            {
                // m_thirdPersonCamera->Update();
                switch(m_currentCamera)
                {
                    case CameraType::FREELOOK:
                        m_freeLookCamera->Update();
                        t_graphicModuleCameraManager.PushCameraToDevice(m_freeLookCamera->GetCamera());
                        break;
                    case CameraType::THIRDPERSON:
                        m_thirdPersonCamera->UpdateInterpolation(p_alpha);
                        t_graphicModuleCameraManager.PushCameraToDevice(m_thirdPersonCamera->GetCamera());
                        break;
                    default:
                        break;
                }
            }
        }

        void CameraHandler::ChangeCamera(CameraType p_type) { m_currentCamera = p_type; }
        DirectX::XMFLOAT3 CameraHandler::GetActiveCameraPosition()
        {
            switch(m_currentCamera)
            {
                case CameraType::FREELOOK:
                    return m_freeLookCamera->GetCamera().GetCameraPosition();
                    break;
                case CameraType::THIRDPERSON:
                    return m_thirdPersonCamera->GetCamera().GetCameraPosition();
                    break;
                default:
                    break;
            }
            // TODO Returnpath
        }

        void CameraHandler::UpdateCameraProjection()
        {
            using namespace DirectX;
            XMFLOAT4X4 projection;

            DoremiEngine::Configuration::ConfiguartionInfo configInfo = m_sharedContext.GetConfigurationModule().GetAllConfigurationValues();
            XMMATRIX mat = XMMatrixTranspose(XMMatrixPerspectiveFovLH(configInfo.CameraFieldOfView * XM_PI / 180.0f,
                                                                      configInfo.ScreenWidth / configInfo.ScreenHeight, 0.1f, configInfo.CameraViewDistance));


            XMStoreFloat4x4(&projection, mat);

            m_freeLookCamera->GetCamera().SetProjectionMatrix(projection);
            m_thirdPersonCamera->GetCamera().SetProjectionMatrix(projection);
        }
        const DoremiEngine::Graphic::Camera& CameraHandler::GetActiveCamera()
        {
            switch(m_currentCamera)
            {
                case Doremi::Core::CameraType::FREELOOK:
                    return m_freeLookCamera->GetCamera();
                    break;
                case Doremi::Core::CameraType::THIRDPERSON:
                    return m_thirdPersonCamera->GetCamera();
                    break;
                default:
                    break;
            }
        }
        const DoremiEngine::Graphic::Camera& CameraHandler::GetThirdPersonCamera() { return m_thirdPersonCamera->GetCamera(); }
    }
}