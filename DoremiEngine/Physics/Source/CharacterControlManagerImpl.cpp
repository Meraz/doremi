#include <Internal/CharacterControlManagerImpl.hpp>
#include <Internal/PhysicsModuleImplementation.hpp>
#include <cstdint>

#include <iostream>
using namespace std;
namespace DoremiEngine
{
    namespace Physics
    {
        CharacterControlManagerImpl::CharacterControlManagerImpl(InternalPhysicsUtils& p_utils) : m_utils(p_utils)
        {
            m_manager = PxCreateControllerManager(*m_utils.m_worldScene);
        }
        CharacterControlManagerImpl::~CharacterControlManagerImpl() { m_manager->release(); }

        bool CharacterControlManagerImpl::IsSleeping(int p_id)
        {
            if(m_controllers.find(p_id) == m_controllers.end())
            {
                cout << "Char controller IsSleeping went wrong" << endl;
                return false;
            }
            else
            {
                return m_controllers[p_id]->getActor()->isSleeping();
            }
        }

        int CharacterControlManagerImpl::AddController(int p_id, int p_matID, XMFLOAT3 p_position, XMFLOAT2 p_dimensions)
        {
            // Set start attributes of the controller
            PxCapsuleControllerDesc desc;
            desc.setToDefault();
            desc.position = PxExtendedVec3(p_position.x, p_position.y, p_position.z);
            desc.height = p_dimensions.x;
            desc.radius = p_dimensions.y;
            desc.material = m_utils.m_physicsMaterialManager->GetMaterial(p_matID); // DANGER! Assumes it already has material
            if(desc.material == NULL)
            {
                throw std::runtime_error("No physics material exists with that ID");
            }
            desc.stepOffset = 0.1f;
            desc.reportCallback = m_controllerCallback;
            desc.behaviorCallback = this;

            // Hard coded up vector
            desc.upDirection = PxVec3(0, 1, 0);

            m_controllers[p_id] = m_manager->createController(desc);
            m_IDsByControllers[m_controllers[p_id]] = p_id;

            // SetCallback(p_id, (1 << 0), (1 << 0));
            SetCallbackFiltering(p_id, 1, 1, 0, 0);

            // Redundant return?
            return p_id;
        }

        bool CharacterControlManagerImpl::MoveController(int p_id, XMFLOAT3 p_discplacement, float p_dt)
        {
            /*
            Some variable that apperas to define when the controller stops moving
            Tweak as necessary. Possibly could be moved to some other place.*/
            // EMPTY FILTERS!
            PxControllerFilters filters;
            // Check if controller exists
            if(m_controllers.find(p_id) == m_controllers.end())
            {
                // Controller did not exist
                throw std::runtime_error("No controller exists with id: " + to_string(p_id));
            }
            PxControllerCollisionFlags flags = m_controllers[p_id]->move(PxVec3(p_discplacement.x, p_discplacement.y, p_discplacement.z), 0, p_dt, filters);
            if(flags & PxControllerCollisionFlag::eCOLLISION_DOWN)
            {
                return true;
            }
            // Redundant return?
            return false;
        }

        XMFLOAT3 CharacterControlManagerImpl::GetPosition(int p_id)
        {
            // Check if controller exists
            if(m_controllers.find(p_id) == m_controllers.end())
            {
                // Controller did not exist
                throw std::runtime_error("No controller exists with id: " + to_string(p_id));
            }
            PxExtendedVec3 p = m_controllers[p_id]->getPosition();
            return XMFLOAT3(static_cast<float>(p.x), static_cast<float>(p.y), static_cast<float>(p.z));
        }

        XMFLOAT4 CharacterControlManagerImpl::GetOrientation(int p_id)
        {
            // Check if controller exists
            if(m_controllers.find(p_id) == m_controllers.end())
            {
                // Controller did not exist
                throw std::runtime_error("No controller exists with id: " + to_string(p_id));
            }
            PxQuat q = m_controllers[p_id]->getActor()->getGlobalPose().q;
            return XMFLOAT4(q.x, q.y, q.z, q.w);
        }

        void CharacterControlManagerImpl::SetPosition(int p_id, XMFLOAT3 p_position)
        {
            // Check if controller exists
            if(m_controllers.find(p_id) == m_controllers.end())
            {
                // Controller did not exist
                throw std::runtime_error("No controller exists with id: " + to_string(p_id));
            }
            if(p_id == 502)
            {
                m_controllers[p_id]->setFootPosition(PxExtendedVec3(p_position.x, p_position.y, p_position.z));
            }
            else
            {
                m_controllers[p_id]->setPosition(PxExtendedVec3(p_position.x, p_position.y, p_position.z));
            }
        }

        void CharacterControlManagerImpl::SetCallbackFiltering(int p_bodyID, int p_thisIdMask, int p_notifyTouchOthersMask,
                                                               int p_notifyLeaveOthersMask, int p_ignoreOthersMask)
        {
            if(m_controllers.find(p_bodyID) == m_controllers.end())
            {
                cout << "Physics. Character Controllers. SetCallBackFiltering. No such controller exists with ID: " << p_bodyID << endl;
                return;
            }
            PxFilterData filterData = PxFilterData(p_thisIdMask, p_notifyTouchOthersMask, p_notifyLeaveOthersMask, p_ignoreOthersMask);
            PxShape* shape;
            m_controllers.at(p_bodyID)->getActor()->getShapes(&shape, 1);
            shape->setSimulationFilterData(filterData);
        }

        void CharacterControlManagerImpl::SetCallback(int p_bodyID, int p_filterGroup, int p_filterMask)
        {
            // Check if controller exists
            if(m_controllers.find(p_bodyID) == m_controllers.end())
            {
                // Controller did not exist
                throw std::runtime_error("No controller exists with id: " + to_string(p_bodyID));
            }
            PxFilterData filterData;
            filterData.word0 = p_filterGroup; // Own ID
            filterData.word1 = p_filterMask; // ID mask to filter pairs that trigger contact callback
            PxRigidActor* actor = m_controllers[p_bodyID]->getActor();
            uint32_t numShapes = actor->getNbShapes();
            // Magic allocation of memory (i think)
            PxShape** shapes = (PxShape**)m_utils.m_allocator.allocate(sizeof(PxShape*) * numShapes, 0, __FILE__, __LINE__);
            actor->getShapes(shapes, numShapes);
            for(uint32_t i = 0; i < numShapes; i++)
            {
                PxShape* shape = shapes[i];
                shape->setSimulationFilterData(filterData);
            }
            if(shapes)
            {
                m_utils.m_allocator.deallocate(shapes);
                shapes = NULL;
            }
        }

        void CharacterControlManagerImpl::SetDrain(int p_id, bool p_isDrain)
        {
            // Check if controller exists
            if(m_controllers.find(p_id) == m_controllers.end())
            {
                // Controller did not exist
                throw std::runtime_error("Physics error: Cannot set controller to drain: No controller exists with id: " + to_string(p_id));
            }
            PxShape* shape;
            m_controllers[p_id]->getActor()->getShapes(&shape, 1);
            shape->setFlag(PxShapeFlag::ePARTICLE_DRAIN, true);
        }

        void CharacterControlManagerImpl::SetCallbackClass(PxUserControllerHitReport* p_callback) { m_controllerCallback = p_callback; }

        unordered_map<PxController*, int> CharacterControlManagerImpl::GetIdsByControllers() { return m_IDsByControllers; }

        PxControllerBehaviorFlags CharacterControlManagerImpl::getBehaviorFlags(const PxShape& shape, const PxActor& actor)
        {
            return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
        }
        PxControllerBehaviorFlags CharacterControlManagerImpl::getBehaviorFlags(const PxController& controller)
        {
            return PxControllerBehaviorFlags(0);
        }
        PxControllerBehaviorFlags CharacterControlManagerImpl::getBehaviorFlags(const PxObstacle& obstacle) { return PxControllerBehaviorFlags(0); }

        void CharacterControlManagerImpl::RemoveCharacterController(int p_bodyID)
        {
            // Erases the character controller in both maps.
            m_controllers[p_bodyID]->release();
            m_IDsByControllers.erase(m_controllers[p_bodyID]);
            m_controllers.erase(p_bodyID);
        }
    }
}