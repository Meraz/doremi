#pragma once


#include "TA files/Entity.h"
#include "TA files/Mesh.h"
#include "TA files/CameraObj.h"
#include "TA files/Light.h"

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

	//kommer ha en av dessa, placera dem i olika vektorer!
	Mesh *mesh = nullptr;
	Light *light = nullptr;
	CameraObj *camera = nullptr; //fick d�pa om den till obj pga oskeeaaar

	Transform()
	{
		CreateTransformCBuffer();
	}
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
};