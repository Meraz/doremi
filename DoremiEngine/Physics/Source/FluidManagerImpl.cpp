#include <Internal/FluidManagerImpl.hpp>
#include <Internal/PhysicsModuleImplementation.hpp>
#include <Internal/ParticleClasses/ParticleEmitter.hpp>
namespace DoremiEngine
{
    namespace Physics
    {
        FluidManagerImpl::FluidManagerImpl(InternalPhysicsUtils& p_utils) : m_utils(p_utils) {}
        FluidManagerImpl::~FluidManagerImpl() {}

        void FluidManagerImpl::CreateParticleEmitter(int p_id, ParticleEmitterData p_data)
        {
            m_emitters[p_id] = new ParticleEmitter(p_data, m_utils);
        }
        void FluidManagerImpl::GetParticlePositions(int p_id, vector<XMFLOAT3>& o_positions) { m_emitters[p_id]->GetPositions(o_positions); }

        void FluidManagerImpl::SetParticleEmitterData(int p_id, ParticleEmitterData p_data) { m_emitters[p_id]->SetData(p_data); }

        vector<int> FluidManagerImpl::GetDrainsHit(int p_id)
        {
            // Secure that the emitter exists
            if(m_emitters.find(p_id) == m_emitters.end())
            {
                throw std::runtime_error("Physics: DrainsHit fetch failed: No particle emitter exists with id: " + to_string(p_id));
            }
            return m_emitters[p_id]->GetDrainsHit();
        }

        void FluidManagerImpl::Update(float p_dt)
        {
            // Update all our emitters
            for(auto it = m_emitters.begin(); it != m_emitters.end(); ++it)
            {
                it->second->Update(p_dt);
            }
        }
    }
}