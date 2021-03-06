#pragma once
#include <SharedContext.hpp>

#include <string>
#include <functional>

namespace DoremiEngine
{
    namespace Core
    {
        class SharedContextImplementation : public SharedContext
        {
        public:
            SharedContextImplementation()
                : m_workingDirectory(""),
                  m_audio(nullptr),
                  m_core(nullptr),
                  m_graphic(nullptr),
                  m_network(nullptr),
                  m_physics(nullptr),
                  m_script(nullptr),
                  m_input(nullptr),
                  m_ai(nullptr),
                  m_logging(nullptr),
                  m_configuration(nullptr)
            {
            }

            void SetWorkingDirectory(const std::string& p_workingDirectory) { m_workingDirectory = p_workingDirectory; }

            void SetAudioModule(Audio::AudioModule* p_audioModule) { m_audio = p_audioModule; }
            void SetCoreModule(DoremiEngine* p_coreModule) { m_core = p_coreModule; }
            void SetGraphicModule(Graphic::GraphicModule* p_graphicModule) { m_graphic = p_graphicModule; }
            void SetNetworkModule(Network::NetworkModule* p_networkModule) { m_network = p_networkModule; }
            void SetPhysicsModule(Physics::PhysicsModule* p_physicsModule) { m_physics = p_physicsModule; }
            void SetScriptModule(Script::ScriptModule* p_scriptModule) { m_script = p_scriptModule; }
            void SetInputModule(Input::InputModule* p_inputModule) { m_input = p_inputModule; }
            void SetAIModule(AI::AIModule* p_AIModule) { m_ai = p_AIModule; }
            void SetLoggingModule(Logging::LoggingModule* p_loggingModule) { m_logging = p_loggingModule; }
            void SetConfigurationModule(Configuration::ConfigurationModule* p_configurationModule) { m_configuration = p_configurationModule; }
            void SetExitFunction(std::function<void()> p_function) { m_exitFunction = p_function; }
            const std::string GetWorkingDirectory() const { return m_workingDirectory; };

            Audio::AudioModule& GetAudioModule() const override
            {
                if(m_audio != nullptr)
                {
                    return *m_audio;
                }
                throw std::runtime_error("Audiomodule has not been initialized."); // TODOXX This cannot be used across .dll borders.
            }

            DoremiEngine& GetCoreModule() const override
            {
                if(m_core != nullptr)
                {
                    return *m_core;
                }
                throw std::runtime_error("Coremodule has not been initialized."); // TODOXX This cannot be used across .dll borders.
            };

            Graphic::GraphicModule& GetGraphicModule() const override
            {
                if(m_graphic != nullptr)
                {
                    return *m_graphic;
                }
                throw std::runtime_error("Graphic module has not been initialized."); // TODOXX This cannot be used across .dll borders.
            };

            Network::NetworkModule& GetNetworkModule() const override
            {
                if(m_network != nullptr)
                {
                    return *m_network;
                }
                throw std::runtime_error("Network module has not been initialized."); // TODOXX This cannot be used across .dll borders.
            };

            Physics::PhysicsModule& GetPhysicsModule() const override
            {
                if(m_physics != nullptr)
                {
                    return *m_physics;
                }
                throw std::runtime_error("Physics module has not been initialized."); // TODOXX This cannot be used across .dll borders.
            };

            Script::ScriptModule& GetScriptModule() const override
            {
                if(m_script != nullptr)
                {
                    return *m_script;
                }
                throw std::runtime_error("Script module has not been initialized."); // TODOXX This cannot be used across .dll borders.
            };

            Input::InputModule& GetInputModule() const override
            {
                if(m_input != nullptr)
                {
                    return *m_input;
                }
                throw std::runtime_error("Input module has not been initialized."); // TODOXX This cannot be used across .dll borders.
            };

            AI::AIModule& GetAIModule() const override
            {
                if(m_ai != nullptr)
                {
                    return *m_ai;
                }
                throw std::runtime_error("AI module has not been initialized."); // TODOXX This cannot be used over .dll borders.
            }

            Logging::LoggingModule& GetLoggingModule() const override
            {
                if(m_logging != nullptr)
                {
                    return *m_logging;
                }
                throw std::runtime_error("Logging module has not been initialized."); // TODOXX This cannot be used over .dll borders.
            }

            Configuration::ConfigurationModule& GetConfigurationModule() const override
            {
                if(m_configuration != nullptr)
                {
                    return *m_configuration;
                }
                throw std::runtime_error("Configuration module has not been initialized."); // TODOXX This cannot be used over .dll borders.
            }

            void RequestApplicationExit() const override
            {
                if(m_exitFunction != nullptr)
                {
                    m_exitFunction();
                }
            }

        private:
            std::string m_workingDirectory;
            Audio::AudioModule* m_audio;
            DoremiEngine* m_core;
            Graphic::GraphicModule* m_graphic;
            Network::NetworkModule* m_network;
            Physics::PhysicsModule* m_physics;
            Script::ScriptModule* m_script;
            Input::InputModule* m_input;
            AI::AIModule* m_ai;
            Logging::LoggingModule* m_logging;
            Configuration::ConfigurationModule* m_configuration;
            std::function<void()> m_exitFunction;
        };
    }
}