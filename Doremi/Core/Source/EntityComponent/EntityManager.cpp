// Project specific
#include <EntityComponent/EntityManager.hpp>
#include <EntityComponent/ComponentTable.hpp>

namespace Doremi
{
    namespace Core
    {

        EntityManager* EntityManager::GetInstance()
        {
            static EntityManager singleton;
            return &singleton;
        }


        EntityManager::EntityManager() { mNextSlot = 0; }


        EntityManager::~EntityManager() {}

        EntityID EntityManager::AddEntity()
        {
            EntityID tIDToUse;

            // if any free slots we use them
            if(mFreeEntitySlots.size() != 0)
            {
                tIDToUse = mFreeEntitySlots.back();
                mFreeEntitySlots.pop_back();
            }
            else
            {
                tIDToUse = mNextSlot;
                mNextSlot++;
            }
            return tIDToUse;
        }

        void EntityManager::RemoveEntity(int pEntityID)
        {
            ComponentTable* tCompTable = ComponentTable::GetInstance();
            // Remove bitmask
            tCompTable->RemoveEntity(pEntityID);

            // Checks if the entity id have been put in the free entity slots list
            std::list<EntityID>::iterator findIter = std::find(mFreeEntitySlots.begin(), mFreeEntitySlots.end(), pEntityID);

            if(findIter == mFreeEntitySlots.end())
            {
                mFreeEntitySlots.push_back(pEntityID);
            }
        }

        void EntityManager::Reset()
        {
            mNextSlot = 0;
            mFreeEntitySlots.clear();
        }
    }
}