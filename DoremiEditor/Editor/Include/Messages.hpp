#pragma once
#include <ObjectData.hpp>
namespace DoremiEditor
{
    struct MessageHeader
    {
        MessageHeader()
        {
            nodeType = 0;
            messageType = 0;
            msgConfig = 0;
            byteTotal = 0;
            byteSize = 0;
            bytePadding = 0;
            // nameElementSize = 0;
        }

        int nodeType;
        int messageType;
        int msgConfig;
        size_t byteTotal;
        size_t byteSize;
        size_t bytePadding;
        // int nameElementSize; //hur stort �r namnet i karakt�rer
        // char objectName[100]; //dynamisk eller inte, char compare funktionen vad g�ra?
    };
    MessageHeader messageHeader;
    bool headerDidFit; // true om den fick plats p� denna sidan av filemapen, annars �r den p� andra, anv�nds vid l�sning av meddelanden


    // har kvar vissa av dessa structsen �ven om de �r lite on�diga d� alla v�rden i vissa redan ligger i en full struct, men vill h�lla det cleant
    // med namngivning :-)
    struct TransformMessage
    {
        char objectName[MAX_NAME_SIZE]; // ifall dessa ska �ndras till dynamiskt allokerade s� kolla FileHandler->CorrectName!!
        char parentName[MAX_NAME_SIZE];
        TransformData transformData;
    };
    struct CameraMessage
    {
        char objectName[MAX_NAME_SIZE];
        char transformName[MAX_NAME_SIZE];
        CameraData cameraData;
    };
    struct MeshMessage
    {
        char objectName[MAX_NAME_SIZE];
        int nrOfTransforms;
        NameMaxNameStruct* transformNames; // skriver man s�h�r? en vector av char arrayer som �r MAX_NAME_SIZE stora
        // char transformName[MAX_NAME_SIZE];
        char materialName[MAX_NAME_SIZE];
        int meshID; // kolla ifall denna meshen ska instance draw:as!
        int materialID;
        MeshData* meshData;
    };
    struct MaterialMessage
    { // namnet p� den ligger i headern sen
        char objectName[MAX_NAME_SIZE];

        char textureName[MAX_NAME_SIZE];
        char glowMapName[MAX_NAME_SIZE];
        char specularMapName[MAX_NAME_SIZE];
        char bumpMapName[MAX_NAME_SIZE];

        int materialType;
        MaterialData materialData;
    };
    struct LightMessage
    {
        // ljusv�rden
        char objectName[MAX_NAME_SIZE];
        char transformName[MAX_NAME_SIZE];
        LightData lightdata;
    };
}