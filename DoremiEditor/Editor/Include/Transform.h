#pragma once
#ifndef TRANSFORM_H
#define TRANSFORM_H
#endif


#include "Entity.h"
#include "Mesh.h"
#include "CameraObj.h"
#include "Light.h"

class Transform : public Entity{
	struct TransformCBufferData
	{
		XMFLOAT4X4 world;
	};
public:
	void *transformDataP = nullptr; ///pointer to the current values, anv�nds endast f�r att ta bort all gammal data i ett svep
	char *name;
	char *parentName; //n�r jag laddar till fil s� ladda namnet fr�n parent variabeln ist�llet, annars finns det risk att parentNamnet �r �ndrat med Renamed grejen
	TransformData transformData;

	Transform *parent = nullptr; //anv�nd parenten och h�mta dess transformation
	TransformCBufferData transformCBufferData;
	ID3D11Buffer *transformCBuffer;

	//kommer ha en av dessa, placera dem i olika vektorer!
	Mesh *mesh = nullptr;
	Light *light = nullptr;
	CameraObj *camera = nullptr; //fick d�pa om den till obj pga oskeeaaar

	Transform(ID3D11Device* gd, ID3D11DeviceContext* gdc)
	{
		this->gDevice = gd; //freea dessa inte h�r, g�rs i main duuh
		this->gDeviceContext = gdc;
		CreateTransformCBuffer();
	}
	Transform()
	{}
	~Transform()
	{ //kanske deleta saker som mesh, men m�jligt vi skickar separata deletes f�r dem, detsamma g�ller children
		//delete(name);
		free(transformDataP);
	}
	void UpdateCBuffer();
	void CreateTransformCBuffer();

	void EmptyVariables(){
		free(transformDataP);
	}
private:
	ID3D11Device* gDevice = nullptr;
	ID3D11DeviceContext* gDeviceContext = nullptr;
};