#pragma once
#ifndef LIGHT_H
#define LIGHT_H
#endif

#include "Entity.h"
//#include "Transform.h"

class Transform; //forward declaration, den ska bara k�nna till att det finns en class som heter Transform, denna pekar p� transform och transform pekar p� denna

class Light : public Entity{
	
public:
	ID3D11Device * gDevice = nullptr;
	ID3D11DeviceContext * gDeviceContext = nullptr;

	void *lightDataP = nullptr;
	char *name;
	LightData lightData;

	Transform *transform = nullptr; //s� jag alltid kan komma �t transformv�rdet direkt
	LightCBufferData lightCBufferData; //datan som skickas in i cbuffern, structen ligger i ObjectData
	
	Light(ID3D11Device *gDevice, ID3D11DeviceContext *gDevC) {
		this->gDevice = gDevice;
		this->gDeviceContext = gDevC;
		
	}
	~Light(){
		free(lightDataP);
		//lightCbuffer->Release();
	}

	void UpdateCBuffer();
	void CreateCBuffer();

	void EmptyVariables() {
		free(lightDataP);
		//delete[] (name);
		//cameraCbuffer->Release(); //inte s�kert jag vill detta, kanske remapa ist�llet! updatesubresource
	}
};