#pragma once
#ifndef MAYALOADER_H
#define MAYALOADER_H
#endif
/// New stuff
/// Engine stuff
namespace DoremiEngine
{
    namespace Core
    {
        class SharedContext;
    }
    namespace Graphic
    {
        class DepthStencilState;
        class RasterizerState;
        class PixelShader;
        class VertexShader;
    }
}

#include <RenderObject.hpp>

/// Old stuff
#include <vector>
#include <Windows.h>
#include <ostream>
#include <iostream>
#include <string>

#include "TA files/Transform.h"
#include "TA files/Material.h"
#include "TA files/Mutex.h"
#include "TA files/FileHandler.h"
#include "TA files/Cam.h"

using namespace std;
//const int MAX_NAME_SIZE = 100; la den i ObjectData s� att alla kan komma �t den
const UINT MAX_NR_LIGHTS = 256;
class MayaLoader{
	
public:	
	MayaLoader(UINT screenWidth, UINT screenHeight, const DoremiEngine::Core::SharedContext& p_sharedContext);
	MayaLoader();
	~MayaLoader();
	void DrawScene();
	void SaveScene();

	//void StartSyncing(); //startar hela processen
	void CreateFileMaps(unsigned int messageFilemapSize);
	void InitVariables();

	void TryReadAMessage();

	void ReadTransform(int i); //parameter f�r edited/added/removed
	void ReadMesh(int i);
	void ReadMeshData(size_t offSetStart); //bara s� det inte ska bli lika mkt kod
	void ReadLight(int i);
	void ReadMaterial(int i);
	void ReadCamera(int i);
	void ReadName(); //mallocen m�ste deallokeras ox�!!

private:
    /// NEW STUFF
    const DoremiEngine::Core::SharedContext& m_sharedContext;
    // Dx stuff
    void InitDX();
    DoremiEngine::Graphic::PixelShader* m_pixelShader;
    DoremiEngine::Graphic::VertexShader* m_vertexShader;
    DoremiEngine::Graphic::DepthStencilState* m_depthStencilState;
    DoremiEngine::Graphic::RasterizerState* m_rasterizerState;

    // Draw stuff
    
    /// END NEW STUFF




	//externa grejer
	UINT screenWidth, screenHeight;
	//*camera*********camera*
	bool UpdateCameraValues(); //returnerar ifall en kamera finns, om den inte finns s� vill man nog inte rendera alls
	struct CameraCBufferData
	{
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
		
	};
	CameraCBufferData defaultCameraCBufferData;
	//*camera*********camera*

	//*light**********light**
	struct LightCBufferDataArray
	{
		LightCBufferData lightDatas[MAX_NR_LIGHTS];
		UINT NumLights;
		float pad[3];
	};
	LightCBufferDataArray lightCBufferDataArray;
	void CreateLightCBufferArray();
	void UpdateLightCBufferArray();
	//*light**********light**

	FileHandler *fileHandler = nullptr;
	//lokala-isch grejer
	//data om filemapen
	struct FilemapInfo{
		size_t head_ByteOffset; //offset in bytes from beginning of the shared memory
		size_t tail_ByteOffset; //offset in bytes from beginning of the shared memory
		size_t non_accessmemoryOffset; //memory in beginning of file thats no touchy, so that head and tail won't get to each other
		//size_t totalConsumers;
		size_t messageFilemap_Size;
		char outFileName[MAX_NAME_SIZE];
		FilemapInfo(){
			head_ByteOffset = 0;
			tail_ByteOffset = 0;
			non_accessmemoryOffset = 10;
			//totalConsumers = 0;
			messageFilemap_Size = 1024; //storleken p� filemapen med meddelanden
		}

	};
	FilemapInfo fileMapInfo;

	void SetFilemapInfoValues(size_t headPlacement, size_t tailPlacement, size_t nonAccessMemoryPlacement, size_t messageFileMapTotalSize); //s�tt negativa v�rden om den inte ska �ndras!
	HANDLE hMessageFileMap = nullptr;
	LPVOID mMessageData = nullptr;
	unsigned int mSize = 1 << 13; //2^15, denna s�tts vid skapelsen av filemapen, parameter v�rdet till funktionen
	//will hold information about where heads and tails are for example, aswell as where the free memory is in the other shared memory
	HANDLE hInfoFileMap = nullptr;
	LPVOID mInfoData = nullptr;
	unsigned int mInfoSize = 1 << 6;

	size_t localTail = 0; //hur l�ngt ifr�n starten i bytes den �r p� filemapen med meddelanden
	int delay = 10; //hur l�nge den ska sleepa mellan varje l�snings f�rs�k

	struct MessageHeader{
		MessageHeader(){
			nodeType = 0;
			messageType = 0;
			msgConfig = 0;
			byteTotal = 0;
			byteSize = 0;
			bytePadding = 0;
			//nameElementSize = 0;
		}

		int nodeType;
		int messageType;
		int msgConfig;
		size_t byteTotal;
		size_t byteSize;
		size_t bytePadding;
		//int nameElementSize; //hur stort �r namnet i karakt�rer
		//char objectName[100]; //dynamisk eller inte, char compare funktionen vad g�ra?
	};
	MessageHeader messageHeader;
	bool headerDidFit; //true om den fick plats p� denna sidan av filemapen, annars �r den p� andra, anv�nds vid l�sning av meddelanden
	
	
	//har kvar vissa av dessa structsen �ven om de �r lite on�diga d� alla v�rden i vissa redan ligger i en full struct, men vill h�lla det cleant med namngivning :-)
	struct TransformMessage{
		char objectName[MAX_NAME_SIZE]; //ifall dessa ska �ndras till dynamiskt allokerade s� kolla FileHandler->CorrectName!!
		char parentName[MAX_NAME_SIZE];
		TransformData transformData;
	};
	struct CameraMessage{
		char objectName[MAX_NAME_SIZE];
		char transformName[MAX_NAME_SIZE];
		CameraData cameraData;
	};
	struct MeshMessage{
		char objectName[MAX_NAME_SIZE];
		int nrOfTransforms;
		NameMaxNameStruct *transformNames; //skriver man s�h�r? en vector av char arrayer som �r MAX_NAME_SIZE stora
		//char transformName[MAX_NAME_SIZE];
		char materialName[MAX_NAME_SIZE];
		int meshID; //kolla ifall denna meshen ska instance draw:as!
		int materialID;
		MeshData *meshData;
	};
	struct MaterialMessage{ //namnet p� den ligger i headern sen
		char objectName[MAX_NAME_SIZE];

		char textureName[MAX_NAME_SIZE];
        char glowMapName[MAX_NAME_SIZE];
        char specularMapName[MAX_NAME_SIZE];
        char bumpMapName[MAX_NAME_SIZE];        
        
		int materialType;
		MaterialData materialData;
	};
	struct LightMessage{
		//ljusv�rden
		char objectName[MAX_NAME_SIZE];
		char transformName[MAX_NAME_SIZE];
		LightData lightdata;
	};
	//struct NameMessage // ligger i ObjectData nu f�r easyness
	//{
	//	char name1[MAX_NAME_SIZE]; //det nya namnet vid namechange
	//	char name2[MAX_NAME_SIZE];
	//};

	TransformMessage *transformMessage = nullptr;
	CameraMessage *cameraMessage = nullptr;
	MeshMessage *meshMessage = nullptr;
	MaterialMessage *materialMessage = nullptr;
	LightMessage *lightMessage = nullptr;
	NameMessage *nameMessage = nullptr;

	size_t transformMessage_MaxSize = 512;
	size_t cameraMessage_MaxSize = 512;
	size_t meshMessage_MaxSize = 1024 * 1024 * 4; //kan ju fan inte h�rdkodas! (maxstorlek de e luuuugnt)
	size_t materialMessage_MaxSize = 2048;
	size_t lightMessage_MaxSize = 512;
	size_t nameMessage_MaxSize = 256;

	//vector<GameObject*> gameObjects;
	vector<Material*> materials;
	vector<Transform*> allTransforms;
	vector<Transform*> allMeshTransforms;
	vector<Transform*> allLightTransforms;
	vector<Transform*> allCameraTransforms;

	Transform* currentCameraTransform = nullptr;

	void MeshChange(MessageHeader mh, MeshMessage *mm); //l�gger till ett nytt objekt om det inte redan finns eller updaterar en gammal, tar hand om den aktualla meshen
	void MeshAdded(MessageHeader mh, MeshMessage *mm);
	void MeshRenamed(MessageHeader mh, NameMessage *mm);

	void TransformAdded(MessageHeader mh, TransformMessage *mm);
	void TransformChange(MessageHeader mh, TransformMessage *mm);
	void TransformDeleted(MessageHeader mh, NameMessage *mm);
	void TransformRenamed(MessageHeader mh, NameMessage *mm);

	void MaterialAdded(MessageHeader mh, MaterialMessage *mm);
	void MaterialChange(MessageHeader mh, MaterialMessage *mm);
	void MaterialDeleted(MessageHeader mh, NameMessage *mm);
	void MaterialRenamed(MessageHeader mh, NameMessage *mm);

	void LightAdded(MessageHeader mh, LightMessage *mm);
	void LightChange(MessageHeader mh, LightMessage *mm);
	void LightRenamed(MessageHeader mh, NameMessage *mm);

	void CameraAdded(MessageHeader mh, CameraMessage *mm);
	void CameraChange(MessageHeader mh, CameraMessage *mm);
	//ingen delete coz p� transformen
	void CameraRenamed(MessageHeader mh, NameMessage *mm);
	void CameraSwitch(MessageHeader mh, CameraMessage *mm); //5

};