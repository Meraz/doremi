#pragma once
#include <DoremiEngine/Core/Include/Subsystem/EngineModule.hpp>
#include <DoremiEngine/Core/Include/SharedContext.hpp>

// standard
#include <DirectXMath.h>
#include <string>
#if defined(_WINDLL)
#define CONFIGURATION_DLL_EXPORT __declspec(dllexport)
#else
#define CONFIGURATION_DLL_EXPORT __declspec(dllimport)
#endif

namespace DoremiEngine
{
    namespace Configuration
    {
        /**
        A struct containing all the information gathered from config files
        */
        struct ConfiguartionInfo
        {
            // Graphic stuff
            float ScreenWidth = 800;
            float ScreenHeight = 600;

            // Audio stuff
            float Volume = 0.5f;

            // Key binds TODOKO ask how this works
            int Forward = 119;
            int Backward = 115;
            int Left = 97;
            int Right = 100;
            int LeftClick = 1;
            int RightClick = 3;
            int StartRepeatingAudioRecording = 228;
            int PlayRepeatableAudioRecording = 246;
            int ExitGame = 27;
        };
        /**
        Reads and saves configuration from file. If another module needs configuration values they can use fucntions in this class to get them.
        */
        class ConfigurationModule : public DoremiEngine::Core::EngineModule
        {
        public:
            /**
            Returns a non modifiable struct with all configuration values
            */
            virtual const ConfiguartionInfo& GetAllConfigurationValues() const = 0;

            /**
            Reads values from the specified file and saves the values recognized
            */
            virtual void ReadConfigurationValuesFromFile(const std::string p_fileName) = 0;

            /**
            Writes the configuration values to file. If the file have the config value it will be overwriten
            Every value the specified filed doesnt have will be writen at the end of the file
            */
            virtual void WriteConfigurationValuesToFile(const std::string p_fileName) = 0;

            /**
            CAUTION!!! Gives full control of the configuration information. Dont delete outside!!
            Usable when you want to change values in the configuration that later should be saved
            */
            virtual ConfiguartionInfo& GetModifiableConfigurationInfo() = 0;
        };
    }
}

extern "C" {
typedef DoremiEngine::Configuration::ConfigurationModule* (*CREATE_CONFIGURATION_MODULE)(const DoremiEngine::Core::SharedContext&);
CONFIGURATION_DLL_EXPORT DoremiEngine::Configuration::ConfigurationModule* CreateConfigurationModule(const DoremiEngine::Core::SharedContext& p_context);
}