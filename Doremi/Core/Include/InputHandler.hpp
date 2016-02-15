#pragma once
// Project specific
#include <Doremi/Core/Include/EntityComponent/Constants.hpp>

// Standard Libraries
#include <unordered_map>
#include <vector>

namespace DoremiEngine
{
    namespace Core
    {
        class SharedContext;
    }
}

namespace Doremi
{
    namespace Core
    {
        // TODOEA L�gga in detta i en textfil.
        /**
            Check the class for more info. More detailed
            This class contains the different enums for the actions in the game.
            Please fill out what key it is.
        */
        enum class UserCommandPlaying
        { // Key                  Code for it
            Jump = 1, // Space     32
            Forward = 2, // W         119
            Backward = 4, // S         115
            Left = 8, // A         97
            Right = 16, // D         100
            Fire = 32, // LeftMouseClick   1
            ScrollWpnUp = 64, // MWheelUp    NULL Handled differently
            ScrollWpnDown = 128, // MWheelDown NULL Handled differently
            DebugForward = 256, // 1073741906
            DebugBackward = 512, // 1073741905
            DebugLeft = 1024, // 1073741904
            DebugRight = 2048, // 1073741903
            DebugButton = 4096,
            StartRepeatableAudioRecording = 8192, //�
            PlayRepeatableAudioRecording = 16384, //�
            ExitGame = 32768,
            LeftClick = 65536,
            SetFrequency0 = 131072, // 7 55
            SetFrequency500 = 262144, // 8 56
            SetFrequency1000 = 524288, // 9 57
            RightClick = 1048576,

            All = Jump | Forward | Backward | Left | Right | Fire | ScrollWpnUp | ScrollWpnDown | DebugForward | DebugBackward | DebugLeft | DebugRight | DebugButton |
                  StartRepeatableAudioRecording |
                  PlayRepeatableAudioRecording |
                  ExitGame |
                  LeftClick |
                  SetFrequency0 |
                  SetFrequency500 |
                  SetFrequency1000 |
                  RightClick,
        };

        // TODOEA L�gga in detta i en textfil.
        /**
            Class that is not being used so far. Might get relevant if we navigate the menu with buttons.
        */
        enum class UserCommandMeny
        { // Code
            LeftClick = 1, // 1
            RightClick = 2, // 3
            Enter = 4, // 13
            Up = 8, // 1073741906
            Down = 16, // 1073741905
            Left = 32, // 1073741904
            Right = 64, // 1073741903
        };

        /**
            There is documentation on how to recieve input on googledrive!
        */
        class InputHandler
        {
        public:
            /**
                Will make you able to recieve input.
            */
            InputHandler(const DoremiEngine::Core::SharedContext& p_sharedContext);

            /**
                Empty deconstructor
            */
            ~InputHandler();

            /**
                Checks if the desired key is pushed.
            */
            bool CheckBitMaskInputFromGame(int p_bitMask);

            /**
                Check if one key is pressed and returns true when the key is first pressed. The next check if the key still is pressed will return a
               false.
            */
            bool CheckForOnePress(int p_bitMask);

            bool CheckForRelease(int p_bitMask);

            /*
                Returns the keys being pressed.
            */
            uint32_t GetInputBitMask();

            // TODOEA void ChangeKeyConfig();//F�r se hur vi g�r h�r kan g�ra p� flera s�tt.
            // Kan g�ra att jag har massa funktioner h�r eller att den menyn skickar in ett ID som
            // p� vad som ska bytas s� kan vi koppla det p� n�got SK�NT s�tt
            // Beh�ver nog ta bort old entries eller �ndra dem p� n�got s�tt.

        protected:
            /**
                Fixar shared context
            */
            const DoremiEngine::Core::SharedContext& m_sharedContext;


            // TODOXX Could be a problem with meny and game inputs I DUNNO To only hav one bitmask.
            /**
                Masken som byggs upp av inputhandlern som sedans anv�nds f�r att checka vad som tryckts.
            */
            uint32_t m_maskWithInput = 0;

            /**
                Masken som anv�nds f�r att kolla vad som tycktes ner senaste updaten. Anv�nds f�r om man ska kolla om man tryckt ner n�got en g�ng och
               inte kontinuerligt.
            */
            uint32_t m_lastUpdateMaskWithInput = 0;
        };
    }
}
