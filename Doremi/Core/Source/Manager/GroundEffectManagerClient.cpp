/// Project specific
// This class
#include <Manager/GroundEffectManagerClient.hpp>
// Handles
#include <EntityComponent/EntityHandler.hpp>
// Components
#include <EntityComponent\Components\PressureParticleComponent.hpp>

/// Engine
// Physics
#include <DoremiEngine/Physics/Include/PhysicsModule.hpp>
#include <DoremiEngine/Physics/Include/FluidManager.hpp>
#include <DoremiEngine/Physics/Include/RigidBodyManager.hpp>

using namespace DirectX;
namespace Doremi
{
    namespace Core
    {

        GroundEffectManagerClient::GroundEffectManagerClient(const DoremiEngine::Core::SharedContext& p_sharedContext)
            : Manager(p_sharedContext, "GroundEffectManagerClient")
        {
            // EXPERIMENTAL PHYSICS. Hard-coded ID works since I thought ahead and made it signed. Tru story
            m_sharedContext.GetPhysicsModule().GetRigidBodyManager().CreateArbitraryBody(-15);
        }

        GroundEffectManagerClient::~GroundEffectManagerClient() {}

        void GroundEffectManagerClient::Update(double p_dt)
        {
            EntityHandler& entityHandler = EntityHandler::GetInstance();
            const size_t length = entityHandler.GetLastEntityIndex();
            int mask = (int)ComponentType::PressureParticleSystem;
            for(size_t i = 0; i < length; i++)
            {
                // Check if we have a pressure particle system. TODOXX This will be really funny if we have just ambient particle systems
                if(entityHandler.HasComponents(i, mask))
                {
                    // Merge new positions into already existing positions
                    const vector<XMFLOAT3>& newPositions = m_sharedContext.GetPhysicsModule().GetFluidManager().GetRemovedParticlesPositions(i);
                    // m_groundEffectPoints.reserve(m_groundEffectPoints.size() + newPositions.size());
                    // m_groundEffectPoints.insert(m_groundEffectPoints.end(), newPositions.begin(), newPositions.end());

                    // Loop through new positions
                    size_t size = newPositions.size();
                    if(size > 0)
                    {
                        for(size_t j = 0; j < size; j++)
                        {
                            m_sharedContext.GetPhysicsModule().GetRigidBodyManager().AddShapeToBody(-15, newPositions[j]);
                        }
                    }
                }
            }
        }

        void GroundEffectManagerClient::OnEvent(Event* p_event) {}
    }
}