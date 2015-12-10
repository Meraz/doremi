#pragma once
namespace Doremi
{
    namespace Core
    {
        /**
        The audio component contains the handle to the soundchannel and a handle to the sound
        */
        struct AudioComponent
        {
            size_t soundID;
            AudioComponent(size_t p_soundID) { soundID = p_soundID; }
            AudioComponent() : soundID(-1) {}
        };
    }
}