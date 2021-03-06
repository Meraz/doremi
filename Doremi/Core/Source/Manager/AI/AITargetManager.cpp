/// Project
#include <Doremi/Core/Include/Manager/AI/AITargetManager.hpp>
#include <PlayerHandlerServer.hpp>

// Components
#include <EntityComponent/EntityHandler.hpp>
#include <EntityComponent/Components/TransformComponent.hpp>
#include <EntityComponent/Components/RangeComponent.hpp>
#include <EntityComponent/Components/EntityTypeComponent.hpp>
#include <EntityComponent/Components/RigidBodyComponent.hpp>
#include <EntityComponent/Components/PhysicsMaterialComponent.hpp>
#include <EntityComponent/Components/AiAgentComponent.hpp>
#include <EntityComponent/Components/PotentialFieldComponent.hpp>
#include <EntityComponent/Components/MovementComponent.hpp>

// Events
#include <Doremi/Core/Include/EventHandler/EventHandler.hpp>
#include <Doremi/Core/Include/EventHandler/Events/AnimationTransitionEvent.hpp>
#include <Doremi/Core/Include/EventHandler/Events/DamageTakenEvent.hpp>

// Helper
#include <Helper/ProximityChecker.hpp>

/// Engine
// Physics
#include <DoremiEngine/Physics/Include/PhysicsModule.hpp>
#include <DoremiEngine/Physics/Include/RayCastManager.hpp>
#include <DoremiEngine/Physics/Include/RigidBodyManager.hpp>

// Config
#include <DoremiEngine/Configuration/Include/ConfigurationModule.hpp>
// Third party
#include <DirectXMath.h>
#include <iostream>
namespace Doremi
{
    namespace Core
    {
        AITargetManager::AITargetManager(const DoremiEngine::Core::SharedContext& p_sharedContext) : Manager(p_sharedContext, "AITargetManager")
        {
            m_playerMovementImpact = m_sharedContext.GetConfigurationModule().GetAllConfigurationValues().AIAimOffset;
            m_maxActorsUpdated = 1; // We may only update one AI per update... TODOCONFIG
            m_actorToUpdate = 0;
        }

        AITargetManager::~AITargetManager() {}

        void AITargetManager::Update(double p_dt)
        {
            using namespace DirectX;

            int updatedActors = 0;
            int lastUpdatedActor = 0;
            // gets all the players in the world, used to see if we can see anyone of them
            std::map<uint32_t, PlayerServer*>& t_players = static_cast<PlayerHandlerServer*>(PlayerHandler::GetInstance())->GetPlayerMap();
            size_t length = EntityHandler::GetInstance().GetLastEntityIndex();
            EntityHandler& t_entityHandler = EntityHandler::GetInstance();

            // TODOXX this have very bad coupling and if possible should be done in the damage manager
            std::map<int, float> damageToPlayer;

            for(size_t i = m_actorToUpdate + 1; i != m_actorToUpdate; i++)
            {
                if(i > length)
                {
                    i = 0;
                    if(i == m_actorToUpdate)
                    {
                        break;
                    }
                }
                // Check and update the attack timer
                bool shouldFire = false;
                if(t_entityHandler.HasComponents(i, (int)ComponentType::AIAgent))
                {
                    AIAgentComponent* timer = t_entityHandler.GetComponentFromStorage<AIAgentComponent>(i);
                    timer->attackTimer += static_cast<float>(p_dt);
                    // If above attack freq we should attack
                    if(timer->attackTimer > timer->attackFrequency)
                    {
                        shouldFire = true;
                    }
                    else
                    {
                        // else continue to next for itteration
                        shouldFire = false;
                    }
                }
                else
                {
                    shouldFire = false; // No timer? TODOKO log error and dont let it fire!
                }
                if(t_entityHandler.HasComponents(i, (int)ComponentType::Range | (int)ComponentType::AIAgent | (int)ComponentType::Transform) &&
                   updatedActors < m_maxActorsUpdated)
                {

                    // They have a range component and are AI agents, let's see if a player is in range!
                    // I use proximitychecker here because i'm guessing it's faster than raycast
                    TransformComponent* AITransform = t_entityHandler.GetComponentFromStorage<TransformComponent>(i);
                    RangeComponent* aiRange = t_entityHandler.GetComponentFromStorage<RangeComponent>(i);
                    int closestVisiblePlayer = -1;

                    float checkRange = 1400;
                    float closestDistance = checkRange; // Hardcoded range, not intreseted if player is outside this
                    int onlyOnePlayerCounts = 1;
                    // Check waht player is close and visible
                    for(auto pairs : t_players)
                    {
                        int playerID = pairs.second->m_playerEntityID;
                        float distanceToPlayer = ProximityChecker::GetInstance().GetDistanceBetweenEntities(pairs.second->m_playerEntityID, i);
                        if(distanceToPlayer < closestDistance)
                        {
                            // It's after this we start whit heavy computation so this is counted as a updated actor
                            lastUpdatedActor = i;
                            updatedActors += onlyOnePlayerCounts;
                            onlyOnePlayerCounts = 0;

                            // Potential player found, check if we see him
                            // We are in range!! Let's raycast in a appropirate direction!! so start with finding that direction!
                            if(!EntityHandler::GetInstance().HasComponents(playerID, (int)ComponentType::Transform)) // Just for saftey :D
                            {
                                std::cout << "Player missing transformcomponent?" << std::endl;
                                return;
                            }
                            // Fetch some components
                            TransformComponent* playerTransform = t_entityHandler.GetComponentFromStorage<TransformComponent>(playerID);

                            // Get things in to vectors
                            XMVECTOR playerPos = XMLoadFloat3(&playerTransform->position);
                            XMVECTOR AIPos = XMLoadFloat3(&AITransform->position);

                            // Calculate direction
                            XMVECTOR direction = playerPos - AIPos; // Might be the wrong way
                            direction = XMVector3Normalize(direction);
                            XMFLOAT3 directionFloat;
                            XMStoreFloat3(&directionFloat, direction);

                            // Offset origin of ray so we dont hit ourself
                            XMVECTOR rayOrigin = AIPos + direction * 5.0f; // TODOCONFIG x.xf is offset from the units body, might need to increase if
                            // the bodies radius is larger than x.x
                            XMFLOAT3 rayOriginFloat;
                            XMStoreFloat3(&rayOriginFloat, rayOrigin);

                            // Send it to physx for raycast calculation
                            int bodyHit = m_sharedContext.GetPhysicsModule().GetRayCastManager().CastRay(rayOriginFloat, directionFloat, checkRange);
                            if(bodyHit == -1)
                            {
                                continue;
                            }
                            // we check if it was a bullet we did hit, in this case we probably did hit our own bullet and should take this as a
                            // player hit
                            // This is a bit of a wild guess and we should probably do a new raycast from the hit location but screw that!
                            bool wasBullet = false;
                            if(EntityHandler::GetInstance().HasComponents(bodyHit, (int)ComponentType::EntityType))
                            {
                                EntityTypeComponent* typeComp = EntityHandler::GetInstance().GetComponentFromStorage<EntityTypeComponent>(bodyHit);
                                if(((int)typeComp->type & (int)EntityType::EnemyBullet) == (int)EntityType::EnemyBullet) // if first entity is bullet
                                {
                                    wasBullet = true;
                                }
                            }
                            bool wasNonRenderObject = false;
                            // object which was hit doesn't have render component - don't collide with it
                            if(!EntityHandler::GetInstance().HasComponents(bodyHit, (int)ComponentType::Render))
                            {
                                wasNonRenderObject = true;
                            }

                            if(bodyHit == playerID || wasBullet || wasNonRenderObject)
                            {
                                closestDistance = distanceToPlayer;
                                closestVisiblePlayer = pairs.second->m_playerEntityID;

                                // Rotate the enemy to face the player
                                XMMATRIX mat = XMMatrixInverse(nullptr, XMMatrixLookAtLH(AIPos, AIPos + direction, XMLoadFloat3(&XMFLOAT3(0, 1, 0))));
                                XMVECTOR rotation = XMQuaternionRotationMatrix(mat);
                                XMFLOAT4 quater;
                                XMStoreFloat4(&quater, rotation);
                                AITransform->rotation = quater;
                            }
                        }
                    }
                    if(closestVisiblePlayer != -1)
                    {
                        // We now know what player is closest and visible
                        if(shouldFire)
                        {
                            if(closestDistance <= aiRange->range)
                            {
                                AIAgentComponent* agent = t_entityHandler.GetComponentFromStorage<AIAgentComponent>(i);
                                agent->attackTimer = 0;
                                if(agent->type == AIType::Melee)
                                {
                                    // melee attack logic, just deal the damage :P
                                    if(damageToPlayer.count(closestVisiblePlayer) == 0)
                                    {
                                        damageToPlayer[closestVisiblePlayer] = 0;
                                    }
                                    damageToPlayer[closestVisiblePlayer] += 10; // TODOCONFIG Melee enemy damage
                                }
                                else if(agent->type == AIType::SmallRanged)
                                {
                                    FireAtEntity(closestVisiblePlayer, i, closestDistance);
                                }
                                AnimationTransitionEvent* t_animationTransition = new AnimationTransitionEvent(i, Animation::ATTACK);
                                EventHandler::GetInstance()->BroadcastEvent(t_animationTransition);
                            }
                        }
                        // If we see a player turn off the phermonetrail
                        if(t_entityHandler.HasComponents(i, (int)ComponentType::PotentialField))
                        {
                            PotentialFieldComponent* pfComp = t_entityHandler.GetComponentFromStorage<PotentialFieldComponent>(i);
                            pfComp->ChargedActor->SetUsePhermonetrail(false);
                            pfComp->ChargedActor->SetActivePotentialVsType(DoremiEngine::AI::AIActorType::Player, true);
                        }
                    }
                    else if(t_entityHandler.HasComponents(i, (int)ComponentType::PotentialField))
                    {
                        // if we dont see a player set phemonetrail and shit
                        PotentialFieldComponent* pfComp = t_entityHandler.GetComponentFromStorage<PotentialFieldComponent>(i);
                        pfComp->ChargedActor->SetUsePhermonetrail(true);
                    }
                }
            }
            m_actorToUpdate = lastUpdatedActor;

            // Send events for all the players that were immediatly damage by enemies
            for(auto pairs : damageToPlayer)
            {
                DamageTakenEvent* t_damageTakenEvent = new DamageTakenEvent(pairs.second, pairs.first);
                EventHandler::GetInstance()->BroadcastEvent(t_damageTakenEvent);
            }
            damageToPlayer.clear();
        }

        void AITargetManager::FireAtEntity(const size_t& p_entityID, const size_t& p_enemyID, const float& p_distance)
        {
            EntityHandler& t_entityHandler = EntityHandler::GetInstance();
            TransformComponent* playerTransform = t_entityHandler.GetComponentFromStorage<TransformComponent>(p_entityID);
            MovementComponent* playerMovement = t_entityHandler.GetComponentFromStorage<MovementComponent>(p_entityID);
            TransformComponent* AITransform = t_entityHandler.GetComponentFromStorage<TransformComponent>(p_enemyID);
            // Get things in to vectors
            XMVECTOR playerPos = XMLoadFloat3(&playerTransform->position);
            XMVECTOR AIPos = XMLoadFloat3(&AITransform->position);

            // calculate direction again...
            XMVECTOR direction = playerPos - AIPos; // Might be the wrong way
            direction = XMVector3Normalize(direction);
            direction += XMVector3Normalize(XMLoadFloat3(&playerMovement->movement) * m_playerMovementImpact);
            direction = XMVector3Normalize(direction);
            XMFLOAT3 directionFloat;
            XMStoreFloat3(&directionFloat, direction);

            XMVECTOR bulletOrigin = AIPos + direction * 5.0f; // TODOCONFIG x.xf is offset from the units body, might need to increase if
            // the bodies radius is larger than x.x
            XMFLOAT3 bulletOriginFloat;
            XMStoreFloat3(&bulletOriginFloat, bulletOrigin);
            int id = t_entityHandler.CreateEntity(Blueprints::BulletEntity, bulletOriginFloat);
            m_sharedContext.GetPhysicsModule().GetRigidBodyManager().SetCallbackFiltering(id, 3, 1, 8, 2);

            // Add a force to the body TODOCONFIG should not be hard coded the force amount
            direction *= 1500.0f;
            XMFLOAT3 force;
            XMStoreFloat3(&force, direction);
            m_sharedContext.GetPhysicsModule().GetRigidBodyManager().AddForceToBody(id, force);
        }
    }
}