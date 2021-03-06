#pragma once
#include <CharacterControlManager.hpp>
#include <PhysX/PxPhysicsAPI.h>

#include <unordered_map>
using namespace std;
using namespace physx;
namespace DoremiEngine
{
    namespace Physics
    {
        struct InternalPhysicsUtils;
        class CharacterControlManagerImpl : public CharacterControlManager, public PxControllerBehaviorCallback
        {
        public:
            CharacterControlManagerImpl(InternalPhysicsUtils& p_utils);
            virtual ~CharacterControlManagerImpl();

            bool IsSleeping(int p_id) override;

            int AddController(int p_id, int p_matID, XMFLOAT3 p_position, XMFLOAT2 p_dimensions) override;
            bool MoveController(int p_id, XMFLOAT3 p_discplacement, float p_dt) override;
            void SetCallbackFiltering(int p_body, int p_thisIdMask, int p_notifyTouchOthersMask, int p_notifyLeaveOthersMask, int p_ignoreOthersMask) override;
            void SetCallback(int p_bodyID, int p_filterGroup, int p_filterMask) override;
            void SetDrain(int p_id, bool p_isDrain) override;

            XMFLOAT3 GetPosition(int p_id) override;
            XMFLOAT4 GetOrientation(int p_id) override;

            void SetPosition(int p_id, XMFLOAT3 p_position) override;

            /**
            Set callback method.
            Only used internally*/
            void SetCallbackClass(PxUserControllerHitReport* p_callback);


            // Implements PxControllerBehaviorCallback
            virtual PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor);
            virtual PxControllerBehaviorFlags getBehaviorFlags(const PxController& controller);
            virtual PxControllerBehaviorFlags getBehaviorFlags(const PxObstacle& obstacle);
            void RemoveCharacterController(int p_bodyID);

            /**
            */
            unordered_map<PxController*, int> GetIdsByControllers();

        private:
            InternalPhysicsUtils& m_utils;

            unordered_map<int, PxController*> m_controllers;
            unordered_map<PxController*, int> m_IDsByControllers;

            PxControllerManager* m_manager;

            PxUserControllerHitReport* m_controllerCallback;
        };
    }
}