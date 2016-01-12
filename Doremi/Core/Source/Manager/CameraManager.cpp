#include <Manager/CameraManager.hpp>
#include <InputHandler.hpp>
#include <CameraClasses/ThirdPersonCamera.hpp>
#include <CameraClasses/FreeLookCamera.hpp>
#include <PlayerHandler.hpp>
#include <InputHandlerClient.hpp>
// Engine
#include <DoremiEngine/Graphic/Include/Interface/Camera/Camera.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Manager/CameraManager.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Manager/SubModuleManager.hpp>
#include <DoremiEngine/Graphic/Include/GraphicModule.hpp>
// Third party


namespace Doremi
{
    namespace Core
    {
        CameraManager::CameraManager(const DoremiEngine::Core::SharedContext& p_sharedContext)
            : Manager(p_sharedContext, "CameraManager"),
              m_graphicModuleCameraManager(m_sharedContext.GetGraphicModule().GetSubModuleManager().GetCameraManager())
        {

            // TODOKO Rename cameramanager on graphic module
            using namespace DirectX;
            XMFLOAT4X4 projection;
            XMMATRIX mat =
                XMMatrixTranspose(XMMatrixPerspectiveFovLH(90.0f * 3.14f / 180.0f, 800.0f / 600.0f, 0.1f, 1000.0f)); // TODOKO use config values
            XMStoreFloat4x4(&projection, mat);

            DoremiEngine::Graphic::Camera* freecamera = m_graphicModuleCameraManager.BuildNewCamera(projection);
            DoremiEngine::Graphic::Camera* thirdPersonCamera = m_graphicModuleCameraManager.BuildNewCamera(projection);
            m_thirdPersonCamera = new ThirdPersonCamera(thirdPersonCamera);
            m_freeLookCamera = new FreeLookCamera(freecamera);
            m_currentCamera = CameraType::FREELOOK;
        }

        CameraManager::~CameraManager() {}

        void CameraManager::Update(double p_dt)
        {
            InputHandlerClient* inputHandler = (InputHandlerClient*)PlayerHandler::GetInstance()->GetDefaultInputHandler();
            if (inputHandler != nullptr)
            {
                if (inputHandler->CheckForOnePress((int)UserCommandPlaying::DebugButton))
                {
                    switch (m_currentCamera)
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
                switch (m_currentCamera)
                {
                case CameraType::FREELOOK:
                    inputHandler->SetCursorInvisibleAndMiddle(false);
                    m_freeLookCamera->Update(p_dt);
                    m_graphicModuleCameraManager.PushCameraToDevice(m_freeLookCamera->GetCamera());
                    break;
                case CameraType::THIRDPERSON:
                    inputHandler->SetCursorInvisibleAndMiddle(true);
                    m_thirdPersonCamera->Update(p_dt);
                    m_graphicModuleCameraManager.PushCameraToDevice(m_thirdPersonCamera->GetCamera());
                    break;
                default:
                    break;
                }
            }
        }
        void CameraManager::ChangeCamera(CameraType p_type)
        {
            m_currentCamera = p_type;
        }
        void CameraManager::OnEvent(Event* p_event) {}
    }
}