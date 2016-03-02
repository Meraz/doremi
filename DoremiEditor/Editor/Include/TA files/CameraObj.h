#pragma once
#ifndef CAMERAOBJ_H
#define CAMERAOBJ_H
#endif

#include "TA files/Entity.h"
//#include "Transform.h"

class Transform; //forward declaration, den ska bara k�nna till att det finns en class som heter Transform, denna pekar p� transform och transform pekar p� denna

class CameraObj : public Entity{
	struct CameraCBufferData {
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
		float cameraPos[4];
	};

public:
	void *cameraDataP = nullptr;
	char *name;
	CameraData cameraData;

	Transform *transform = nullptr; //s� jag alltid kan komma �t transformv�rdet direkt
	CameraCBufferData cameraCBufferData;

	//XMFLOAT4X4 mview;
	//XMFLOAT4X4 mprojection;

	CameraObj() {
	}
	~CameraObj(){
	}

	void UpdateCBuffer(UINT screenWidth, UINT screenHeight);
	void CreateCBuffer();

	void EmptyVariables() {
		free(cameraDataP);
		//cameraCbuffer->Release(); //inte s�kert jag vill detta, kanske remapa ist�llet! updatesubresource
	}
};