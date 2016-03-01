#pragma once
#ifndef LIGHT_H
#define LIGHT_H
#endif

#include "TA files/Entity.h"
//#include "Transform.h"

class Transform; //forward declaration, den ska bara k�nna till att det finns en class som heter Transform, denna pekar p� transform och transform pekar p� denna

class Light : public Entity{
	
public:
	void *lightDataP = nullptr;
	char *name;
	LightData lightData;

	Transform *transform = nullptr; //s� jag alltid kan komma �t transformv�rdet direkt
	LightCBufferData lightCBufferData; //datan som skickas in i cbuffern, structen ligger i ObjectData
	
	Light() {
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