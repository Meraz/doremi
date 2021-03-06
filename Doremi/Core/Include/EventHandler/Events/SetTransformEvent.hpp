#pragma once
#include <Doremi/Core/Include/EventHandler/Events/Event.hpp>
#include <Doremi/Core/Include/EntityComponent/Constants.hpp>
#include <DirectXMath.h>
#include <stdint.h>


namespace Doremi
{
    namespace Core
    {
        struct SetTransformEvent : public Event
        {
            SetTransformEvent()
                : Event(EventType::SetTransform), entityID(0), position(DirectX::XMFLOAT3(0, 0, 0)), orientation(DirectX::XMFLOAT4(0, 0, 0, 0))
            {
            }
            SetTransformEvent(const uint32_t& p_entityID, const DirectX::XMFLOAT3& p_position, const DirectX::XMFLOAT4& p_orientation)
                : Event(EventType::SetTransform), entityID(p_entityID), position(p_position), orientation(p_orientation)
            {
            }

            /**
            Write object to stream
            */
            void Write(Streamer* p_streamer, uint32_t& op_bitsWritten) override
            {
                p_streamer->WriteUnsignedInt32(entityID);
                p_streamer->WriteFloat3(position);
                p_streamer->WriteRotationQuaternion(orientation);
                op_bitsWritten += (sizeof(uint32_t) + sizeof(float) * 3 + sizeof(float) * 4) * 8;
            }

            /**
            Read object from stream
            */
            void Read(Streamer* p_streamer, uint32_t& op_bitsRead) override
            {
                entityID = p_streamer->ReadUnsignedInt32();
                position = p_streamer->ReadFloat3();
                orientation = p_streamer->ReadRotationQuaternion();
                op_bitsRead += (sizeof(uint32_t) + sizeof(float) * 3 + sizeof(float) * 4) * 8;
            }

            EntityID entityID;
            DirectX::XMFLOAT3 position;
            DirectX::XMFLOAT4 orientation;
        };
    }
}