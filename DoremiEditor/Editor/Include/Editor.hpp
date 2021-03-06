#pragma once
#include <Doremi/Core/Include/GameCore.hpp>
#include <vector>

namespace Doremi
{
    namespace Core
    {
        class Manager;
        class EntityInterface;
    }
}
namespace DoremiEditor
{

    class EditorMain : public Doremi::Core::GameCore
    {
    public:
        /**
        Constructor
        */
        EditorMain();

        /**
        Destructor
        */
        virtual ~EditorMain();

        /**
        doc
        */
        void Start();

    private:
        /**
        TODOCM doc
        */
        void Initialize();

        /**
        TODOCM doc
        */
        void Run();

        /**
        TODOCM doc
        */
        void Update(double p_deltaTime);

        /**
        doc
        */
        void Stop() override;

        /**
        TOODCM doc
        */
        std::vector<Doremi::Core::Manager*> m_managers;
    };
}