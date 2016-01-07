#pragma once
// Project specific
#include <AudioHandler.hpp>
#include <Doremi/Core/Include/InputHandler.hpp>
#include <DoremiEngine/Physics/Include/PhysicsModule.hpp>
#include <DoremiEngine/Physics/Include/RigidBodyManager.hpp>
//#include <DoremiEngine/Physics/Include/PhysicsMaterialManager.hpp>
#include <DoremiEngine/Physics/Include/PhysicsMaterialManager.hpp>
#include <EntityComponent/EntityHandler.hpp>
#include <DoremiEngine/Audio/Include/AudioModule.hpp>
#include <iostream>
namespace Doremi
{
    namespace Core
    {
        AudioHandler::AudioHandler(const DoremiEngine::Core::SharedContext& p_sharedContext) : m_sharedContext(p_sharedContext) {}
        AudioHandler::~AudioHandler() {}
        void AudioHandler::StartAudioHandler(const DoremiEngine::Core::SharedContext& p_sharedContext)
        {
            m_singleton = new AudioHandler(p_sharedContext);
        }
        void AudioHandler::Initialize()
        {
            m_currentFrequency = 0;
            m_continuousFrequencyAnalyserChannelID = 99999;
            m_continuousFrequencyAnalyserSoundID = 0;
            m_repeatableFrequencyAnalyserChannelID = 99999;
            m_outputRepeatableSoundChannelID = 99999;
            m_repeatableFrequencyAnalyserSoundID = 0;
            m_accumulatedDeltaTime = 0;
            m_analyseActive = false;
            m_repeatableAnalysisActive = false;
            m_continuousRecording = true;
            m_repeatableAnalysisComplete = false;
            m_frequencyVectorPrecision = 0.01;
        }
        AudioHandler* AudioHandler::m_singleton = nullptr;
        AudioHandler* AudioHandler::GetInstance() { return m_singleton; }
        void AudioHandler::SetContinuousFrequencyAnalyserSoundID(const size_t& p_soundID) { m_continuousFrequencyAnalyserSoundID = p_soundID; }
        void AudioHandler::SetRepeatableFrequencyAnalyserSoundID(const size_t& p_soundID) { m_repeatableFrequencyAnalyserSoundID = p_soundID; }
        void AudioHandler::SetupContinuousRecording()
        {
            DoremiEngine::Audio::AudioModule& t_audioModule = m_sharedContext.GetAudioModule();
            m_continuousFrequencyAnalyserSoundID = t_audioModule.SetupRecording(true);
        }
        void AudioHandler::SetupRepeatableRecording()
        {
            DoremiEngine::Audio::AudioModule& t_audioModule = m_sharedContext.GetAudioModule();
            m_repeatableFrequencyAnalyserSoundID = t_audioModule.SetupRecording(false);
        }
        void AudioHandler::StartContinuousRecording()
        {
            DoremiEngine::Audio::AudioModule& t_audioModule = m_sharedContext.GetAudioModule();
            t_audioModule.StopRecording();
            t_audioModule.StartRecording(m_continuousFrequencyAnalyserSoundID, true);
            m_repeatableAnalysisActive = false;
            m_continuousRecording = true;
        }
        void AudioHandler::StartRepeatableRecording()
        {
            DoremiEngine::Audio::AudioModule& t_audioModule = m_sharedContext.GetAudioModule();
            t_audioModule.StopRecording();
            t_audioModule.StartRecording(m_repeatableFrequencyAnalyserSoundID, false);
            m_repeatableAnalysisActive = false;
            m_analyseActive = false;
            m_continuousRecording = false;
            m_repeatableAnalysisComplete = false;
        }

        float AudioHandler::GetRepeatableSoundFrequency()
        {
            if (m_repeatableAnalysisComplete)
            {
                DoremiEngine::Audio::AudioModule& t_audioModule = m_sharedContext.GetAudioModule();
                double t_timeElapsed = t_audioModule.GetSoundTimePointer(m_outputRepeatableSoundChannelID);
                t_timeElapsed /= m_frequencyVectorPrecision;
                float retValue = m_frequencies[(int)t_timeElapsed];
                return retValue;
            }
            else
            {
                return 0;
            }
        }

        void AudioHandler::PlayRepeatableRecordedSound()
        {
            if (m_outputRepeatableSoundID < 1000)
            {
                DoremiEngine::Audio::AudioModule& t_audioModule = m_sharedContext.GetAudioModule();
                t_audioModule.PlayASound(m_outputRepeatableSoundID, false, m_outputRepeatableSoundChannelID);
            }
        }
        void AudioHandler::Update(double p_deltaTime)
        {
            DoremiEngine::Audio::AudioModule& t_audioModule = m_sharedContext.GetAudioModule();
            switch (SoundState)
            {
            default:
                break;
            }


            if (m_continuousRecording)
            {
                if (m_analyseActive)
                {
                    m_currentFrequency = t_audioModule.AnalyseSoundSpectrum(m_continuousFrequencyAnalyserChannelID);
                }
                else
                {
                    unsigned int recordPointer = t_audioModule.GetRecordPointer();
                    if (recordPointer > 9600)
                    {
                        m_analyseActive = true;
                        //Fulfix f�r att kunna �teranv�nda kanalen 99999 �r min -1. Default v�rde som inte kan anv�ndas. I komponenterna s�tts det i defaultkontruktorn.
                        if (m_continuousFrequencyAnalyserChannelID != 99999)
                        {
                            /**
                            TODOLH Beh�vs nog uppdatera ljudposition till listener position.
                            */
                            t_audioModule.PlaySoundOnSpecificChannel(m_continuousFrequencyAnalyserSoundID, true, m_continuousFrequencyAnalyserChannelID);
                        }
                        else
                        {
                            t_audioModule.PlayASound(m_continuousFrequencyAnalyserSoundID, true, m_continuousFrequencyAnalyserChannelID);
                        }
                        t_audioModule.SetPriority(m_continuousFrequencyAnalyserChannelID, 0); 
                        /** TODOLH os�ker p� om detta beh�vs.
                        Tydligen kan det bli s� att fmod snor kanaler. Men d� tar den kanaler som har l�g prio*/
                    }
                }
            }
            else
            {
                if (m_repeatableAnalysisActive)
                {
                    /**
                    TODOLH H�r ska vi koda in arrayen som spar ner analysdatan
                    */

                    //Kolla om ljudet �r "klart" is�fall �verg� till andra inspelingss�ttet.
                    unsigned int recordPointer = t_audioModule.GetRecordPointer();
                    //std::cout << "Analysaktiv " << recordPointer << endl;
                    if (!t_audioModule.IsRecording())
                    {
                        m_continuousRecording = true;
                        m_repeatableAnalysisActive = false;
                        m_analyseActive = false;
                        m_repeatableAnalysisComplete = true;
                        t_audioModule.StopRecording();
                        t_audioModule.StartRecording(m_continuousFrequencyAnalyserSoundID, true);
                        m_outputRepeatableSoundID = t_audioModule.TestCopy(m_repeatableFrequencyAnalyserSoundID, 1.0f);
                        t_audioModule.PlayASound(m_outputRepeatableSoundID, false, m_outputRepeatableSoundChannelID);
                        t_audioModule.SetVolumeOnChannel(m_outputRepeatableSoundChannelID, 0);
                    }
                    else
                    {
                        //Fixa in i array f�r att anv�ndas med beamen senare!
                        m_accumulatedDeltaTime += p_deltaTime;
                        if (m_accumulatedDeltaTime > m_frequencyVectorPrecision)
                        {
                            m_accumulatedDeltaTime = 0; ///Ta tidsintervallet multiplicera med 5 � ta det arrayv�rdet!!
                            m_frequencies.push_back(t_audioModule.AnalyseSoundSpectrum(m_repeatableFrequencyAnalyserChannelID));
                        }
                        else
                        {
                            //do nothing
                        }
                    }
                }
                else
                {
                    unsigned int recordPointer = t_audioModule.GetRecordPointer();
                    std::cout << "AnalysFalse " << recordPointer << endl;
                    if (recordPointer > 9600)
                    {
                        m_repeatableAnalysisActive = true;
                        m_frequencies.clear();
                        t_audioModule.PlayASound(m_repeatableFrequencyAnalyserSoundID, false, m_repeatableFrequencyAnalyserChannelID);
                        t_audioModule.SetPriority(m_repeatableFrequencyAnalyserChannelID, 1);
                    }
                }
            }

        }
    }
}
