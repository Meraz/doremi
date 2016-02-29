#include <Doremi/Core/Include/PlayerHandlerClient.hpp>

#include <Doremi/Core/Include/EntityComponent/EntityHandler.hpp>

#include <Doremi/Core/Include/NetworkEventReceiver.hpp>
#include <Doremi/Core/Include/FrequencyBufferHandler.hpp>

#include <Doremi/Core/Include/EventHandler/EventHandler.hpp>
#include <Doremi/Core/Include/EventHandler/Events/PlayerCreationEvent.hpp>
#include <Doremi/Core/Include/EventHandler/Events/GunFireToggleEvent.hpp>
#include <Doremi/Core/Include/EventHandler/Events/PlayerRespawnEvent.hpp>

#include <DoremiEngine/Input/Include/InputModule.hpp>
#include <Doremi/Core/Include/EventHandler/Events/PlaySoundEvent.hpp>
#include <Doremi/Core/Include/EntityComponent/Components/AudioComponent.hpp>

// AI
#include <DoremiEngine/AI/Include/AIModule.hpp>
#include <DoremiEngine/AI/Include/Interface/SubModule/PotentialFieldSubModule.hpp>
#include <DoremiEngine/AI/Include/Interface/PotentialField/PotentialFieldActor.hpp>

// Physics
#include <DoremiEngine/Physics/Include/PhysicsModule.hpp>
#include <DoremiEngine/Physics/Include/RigidBodyManager.hpp>
#include <DoremiEngine/Physics/Include/CharacterControlManager.hpp>
#include <DoremiEngine/Physics/Include/PhysicsMaterialManager.hpp>

// Components
#include <EntityComponent/Components/PhysicsMaterialComponent.hpp>
#include <EntityComponent/Components/PotentialFieldComponent.hpp>
#include <EntityComponent/Components/PressureParticleComponent.hpp>
#include <EntityComponent/Components/TransformComponent.hpp>
#include <EntityComponent/Components/HealthComponent.hpp>

#include <Doremi/Core/Include/InputHandlerClient.hpp>

#include <Doremi/Core/Include/Streamers/NetworkStreamer.hpp>
#include <Doremi/Core/Include/EventInterpeter.hpp>

// Timing
#include <Timing/TimerManager.hpp>

#include <iostream>

// Logging
#include <DoremiEngine/Logging/Include/LoggingModule.hpp>
#include <DoremiEngine/Logging/Include/SubmoduleManager.hpp>
#include <DoremiEngine/Logging/Include/Logger/Logger.hpp>

namespace Doremi
{
    namespace Core
    {
        PlayerHandlerClient::PlayerHandlerClient(const DoremiEngine::Core::SharedContext& p_sharedContext)
            : PlayerHandler(p_sharedContext), m_logger(nullptr), m_lastJoinEventRead(0), m_maxNumEvents(0)
        {
            EventHandler::GetInstance()->Subscribe(EventType::GunFireToggle, this);
            EventHandler::GetInstance()->Subscribe(EventType::PlayerRespawn, this);
            EventHandler::GetInstance()->Subscribe(EventType::EntityCreated, this);
            EventHandler::GetInstance()->Subscribe(EventType::SetHealth, this);
            EventHandler::GetInstance()->Subscribe(EventType::SetTransform, this);
            m_logger = &m_sharedContext.GetLoggingModule().GetSubModuleManager().GetLogger();

            InputHandlerClient* m_inputHandler = new InputHandlerClient(p_sharedContext);
            NetworkEventReceiver* newNetworkEventReceiver = new NetworkEventReceiver();
            FrequencyBufferHandler* newFrequencyHandler = new FrequencyBufferHandler();

            // Create player object thing
            m_player = new PlayerClient(0, m_inputHandler, newFrequencyHandler, newNetworkEventReceiver);
        }

        PlayerHandlerClient::~PlayerHandlerClient()
        {
            // Do not delete m_logger, internally handled by loggingmodule
        }

        void PlayerHandlerClient::StartPlayerHandlerClient(const DoremiEngine::Core::SharedContext& p_sharedContext)
        {
            if(m_singleton != nullptr)
            {
                std::runtime_error("PlayerHandler StartPlayerHandler called multiple times.");
            }
            m_singleton = new PlayerHandlerClient(p_sharedContext);
        }


        EntityID PlayerHandlerClient::GetPlayerEntityID() { return m_player->m_playerEntityID; }


        InputHandlerClient* PlayerHandlerClient::GetInputHandler() { return static_cast<InputHandlerClient*>(m_player->m_inputHandler); }

        FrequencyBufferHandler* PlayerHandlerClient::GetFrequencyBufferHandler() { return m_player->m_frequencyBufferHandler; }


        void PlayerHandlerClient::Update(double p_dt)
        {
            TIME_FUNCTION_START

            // If player is created we can update it
            if(m_player->IsCreated)
            {
                UpdatePlayerPositions(m_player);
                UpdatePlayerRotations(m_player);
                UpdateFiring(m_player);
            }

            TIME_FUNCTION_STOP
        }

        void PlayerHandlerClient::UpdatePlayerRotations(Player* p_player)
        {
            TIME_FUNCTION_START

            InputHandlerClient* inputHandler = (InputHandlerClient*)p_player->m_inputHandler;

            EntityID entityID = p_player->m_playerEntityID;

            if(EntityHandler::GetInstance().HasComponents(entityID, (int)ComponentType::CharacterController | (int)ComponentType::Transform))
            {
                /// Handle mouse input
                // Get mouse input
                int t_mouseMovementX = inputHandler->GetMouseMovementX();
                if(t_mouseMovementX)
                {
                    TransformComponent* transComp = EntityHandler::GetInstance().GetComponentFromStorage<TransformComponent>(entityID);
                    // Get direction
                    XMFLOAT4 orientation = transComp->rotation;

                    // Create rotation matrix with orientation quaternion
                    XMVECTOR orientationVec = XMLoadFloat4(&orientation);

                    // Get current angle
                    float angle;
                    XMVECTOR oldDir; // This really is only needed for the parameter below...
                    XMQuaternionToAxisAngle(&oldDir, &angle, orientationVec);
                    // Change the angle
                    angle += t_mouseMovementX * p_player->m_turnSpeed;
                    // Single quaternions don't really like angles over 2*pi, we do this
                    if(angle > 2.0f * 3.1415f)
                    {
                        angle -= 2.0f * 3.1415f;
                    }
                    else if(angle < 0)
                    {
                        angle += 2.0f * 3.1415f;
                    }
                    // Create a new quaternion with the new angle
                    orientationVec = XMQuaternionRotationAxis(XMLoadFloat3(&XMFLOAT3(0, 1, 0)), angle);
                    // Store results
                    XMStoreFloat4(&transComp->rotation, orientationVec);
                }
            }

            TIME_FUNCTION_STOP
        }

        void PlayerHandlerClient::UpdatePlayerInputs()
        {
            TIME_FUNCTION_START
            m_sharedContext.GetInputModule().Update();

            // Update the inputhandler
            InputHandlerClient* inputHandler = static_cast<InputHandlerClient*>(m_player->m_inputHandler);
            inputHandler->Update();

            // If player is created, I think we want to logg input
            if(m_player->IsCreated)
            {
                using namespace Doremi::Utilities::Logging;
                m_logger->DebugLog(LogTag::INPUT, LogLevel::MASS_DATA_PRINT, "X, %d\nY, %d\nM, %d", inputHandler->GetMouseMovementX(),
                    inputHandler->GetMouseMovementY(), inputHandler->GetInputBitMask());
            }

            TIME_FUNCTION_STOP
        }

        void PlayerHandlerClient::SetNewPlayerEntityID(const EntityID& p_entityID)
        {
            if(m_player->IsCreated)
            {
                std::runtime_error("Player created multiple times");
            }

            // Set id and set it created
            m_player->IsCreated = true;
            m_player->m_playerEntityID = p_entityID;

            // Create event
            PlayerCreationEvent* playerCreateEvent = new PlayerCreationEvent(p_entityID);

            // Broadcast event
            // TODOXX If we create this event on server and send this might be removed
            EventHandler::GetInstance()->BroadcastEvent(playerCreateEvent);
        }

        NetworkEventReceiver* PlayerHandlerClient::GetNetworkEventReceiver() { return m_player->m_networkEventReceiver; }

        void PlayerHandlerClient::ReadEventsForJoin(NetworkStreamer& p_streamer, const uint32_t& p_bufferSize, uint32_t& op_bytesRead)
        {
            // Get eventhandler for later use
            EventHandler* t_eventHandler = EventHandler::GetInstance();

            // Read the first event to be read
            uint32_t t_messageStartEvent = p_streamer.ReadUnsignedInt32();
            op_bytesRead += sizeof(uint32_t);

            // Read the number of events to be read
            uint32_t t_numOfEvents = p_streamer.ReadUnsignedInt32();
            op_bytesRead += sizeof(uint32_t);

            // Here is a thing, because we know all event exists beforehand, and because they are bunched to be sent according to acks
            // We can make the assumption that if we've already read the first one here, we've read all in the message

            uint32_t t_bitsRead = 0;
            if(t_messageStartEvent < m_lastJoinEventRead)
            {
                for(size_t i = 0; i < t_numOfEvents; i++)
                {
                    // Read but ignore all events
                    Event* t_newEvent = InterpetEvent(p_streamer, t_bitsRead);

                    // TODO better way of creating, destroying events?(not just this place)
                    // delete them right away
                    delete t_newEvent;
                }
            }
            else
            {
                for(size_t i = 0; i < t_numOfEvents; i++)
                {
                    // Read but ignore all events
                    Event* t_newEvent = InterpetEvent(p_streamer, t_bitsRead);

                    t_eventHandler->BroadcastEvent(t_newEvent);
                }
                m_lastJoinEventRead += t_numOfEvents;
            }

            // Exclude duplicates
            op_bytesRead += ceil(t_bitsRead / 8.0f);

            // Set position back
            p_streamer.SetReadWritePosition(op_bytesRead);
        }

        bool PlayerHandlerClient::PlayerExists() { return m_player->IsCreated; }

        void PlayerHandlerClient::OnEvent(Event* p_event)
        {
            if(p_event->eventType == EventType::GunFireToggle)
            {
                GunFireToggleEvent* t_gunFireToggleEvent = static_cast<GunFireToggleEvent*>(p_event);

                // If it's not ourselves we fire
                if(m_player->IsCreated && t_gunFireToggleEvent->entityID != m_player->m_playerEntityID)
                {
                    if(t_gunFireToggleEvent->isFiring)
                    {
                        m_gunController.StartFireGun(t_gunFireToggleEvent->entityID, m_sharedContext);
                    }
                    else
                    {
                        m_gunController.StopFireGun(t_gunFireToggleEvent->entityID, m_sharedContext);
                    }
                }
            }
            else if(p_event->eventType == EventType::PlayerRespawn)
            {
                PlayerRespawnEvent* t_playerSpawnerEvent = static_cast<PlayerRespawnEvent*>(p_event);

                // Reset health for show
                HealthComponent* t_healthComp = GetComponent<HealthComponent>(t_playerSpawnerEvent->entityID);
                t_healthComp->currentHealth = t_healthComp->maxHealth;

                // Play respawn sound
                EventHandler::GetInstance()->BroadcastEvent(new PlaySoundEvent(t_playerSpawnerEvent->entityID, static_cast<int32_t>(AudioCompEnum::Death)));
            }
            else if(p_event->eventType == Doremi::Core::EventType::SetHealth)
            {
                SetHealthEvent* t_setHealthEvent = static_cast<SetHealthEvent*>(p_event);

                GetComponent<HealthComponent>(t_setHealthEvent->entityID)->currentHealth = t_setHealthEvent->health;
            }
            else if(p_event->eventType == Doremi::Core::EventType::SetTransform)
            {
                SetTransformEvent* t_setTransformEvent = static_cast<SetTransformEvent*>(p_event);

                EntityHandler& t_entityHandler = EntityHandler::GetInstance();

                // If char controller or rigid body, set position to physics
                if(t_entityHandler.HasComponents(t_setTransformEvent->entityID, static_cast<uint32_t>(ComponentType::RigidBody)))
                {
                    DoremiEngine::Physics::RigidBodyManager& t_rigidBodyManager = m_sharedContext.GetPhysicsModule().GetRigidBodyManager();

                    t_rigidBodyManager.SetBodyPosition(t_setTransformEvent->entityID, t_setTransformEvent->position, t_setTransformEvent->orientation);
                }
                else if(t_entityHandler.HasComponents(t_setTransformEvent->entityID, static_cast<uint32_t>(ComponentType::CharacterController)))
                {
                    DoremiEngine::Physics::CharacterControlManager& t_characterControlManager = m_sharedContext.GetPhysicsModule().GetCharacterControlManager();

                    t_characterControlManager.SetPosition(t_setTransformEvent->entityID, t_setTransformEvent->position);
                }

                // Set transform to components
                TransformComponent* transComp = GetComponent<TransformComponent>(t_setTransformEvent->entityID);
                transComp->position = t_setTransformEvent->position;
                transComp->rotation = t_setTransformEvent->orientation;

                // Set copy data to other transform components
                memcpy(GetComponent<TransformComponentNext>(t_setTransformEvent->entityID), transComp, sizeof(TransformComponent));
                memcpy(GetComponent<TransformComponentPrevious>(t_setTransformEvent->entityID), transComp, sizeof(TransformComponent));
                *GetComponent<TransformComponentSnapshotNext>(t_setTransformEvent->entityID) =
                    TransformComponentSnapshotNext(*GetComponent<TransformComponentNext>(t_setTransformEvent->entityID));
                *GetComponent<TransformComponentSnapshotPrevious>(t_setTransformEvent->entityID) =
                    TransformComponentSnapshotPrevious(*GetComponent<TransformComponentNext>(t_setTransformEvent->entityID));
            }
        }
    }
}
