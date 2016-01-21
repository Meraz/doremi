// Project specific
#include <Manager/ExampleManager.hpp>
#include <DoremiEngine/Physics/Include/PhysicsModule.hpp>
#include <DoremiEngine/Graphic/Include/GraphicModule.hpp>
#include <DoremiEngine/Audio/Include/AudioModule.hpp>
#include <DoremiEngine/Input/Include/InputModule.hpp>
#include <EntityComponent/EntityHandler.hpp>
#include <EntityComponent/Components/ExampleComponent.hpp>
#include <EntityComponent/Components/Example2Component.hpp>
#include <EventHandler/EventHandler.hpp>
#include <EventHandler/Events/ExampleEvent.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Manager/SubModuleManager.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Manager/MeshManager.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Manager/DirectXManager.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Mesh/MeshInfo.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Mesh/MaterialInfo.hpp>


// Third party

// Standard
#include <iostream>

using namespace std;

namespace Doremi
{
    namespace Core
    {
        ExampleManager::ExampleManager(const DoremiEngine::Core::SharedContext& p_sharedContext) : Manager(p_sharedContext, "ExampleManager")
        {
            EventHandler::GetInstance()->Subscribe(EventType::Example, this);
        }

        ExampleManager::~ExampleManager() {}


        void ExampleManager::Update(double p_dt)
        {
            // Example on how to create and Broadcast a event
            ExampleEvent* myEvent = new ExampleEvent();
            myEvent->eventType = EventType::Example;
            myEvent->myInt = 42;
            EventHandler::GetInstance()->BroadcastEvent(myEvent);

            // Loop through all entities
            const size_t length = EntityHandler::GetInstance().GetLastEntityIndex();
            for(size_t i = 0; i < length; i++)
            {
                // Check that the current entity has the relevant components
                if(EntityHandler::GetInstance().HasComponents(i, (int)ComponentType::Example) | (int)ComponentType::Example2)
                {
                    // Get component
                    ExampleComponent* t_example = EntityHandler::GetInstance().GetComponentFromStorage<ExampleComponent>(i);
                    Example2Component* t_example2 = EntityHandler::GetInstance().GetComponentFromStorage<Example2Component>(i);

                    // Perform desired operation
                    t_example->posX++;

                    // Test run GraphicsModule

                    m_sharedContext.GetInputModule().Update(); // TODOEA
                }
            }
        }
        void ExampleManager::OnEvent(Event* p_event)
        {
            // Check to see what event was received and do something with it (Might be changed to callback functions instead)
            switch(p_event->eventType)
            {
                case EventType::Example:
                {
                    // Cast the event to the correct format
                    ExampleEvent* t_event = (ExampleEvent*)p_event;
                    int t_intFromEvent = t_event->myInt;
                    break;
                }
                default:
                    break;
            }
        }
    }
}