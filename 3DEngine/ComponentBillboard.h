#ifndef __COMPONENTBILLBOARD_H__
#define __COMPONENTBILLBOARD_H__

#include "ComponentMesh.h"

class ComponentMaterial;
class ResMesh;
class ComponentCamera;
class ComponentTransform;

enum Alignment {
	SCREEN_ALIGN,
	WORLD_ALIGN,
	AXIAL_ALIGN
};

class ComponentBillboard :
	public ComponentMesh
{
public:
	ComponentBillboard(ComponentMaterial* mat = nullptr);
	~ComponentBillboard();

	bool Start();
	bool Update();
	void UpdateUI();
	bool CleanUp();
	void FaceCamera();

	void SetTexture(ComponentMaterial* t);

	Alignment alignment = SCREEN_ALIGN;
	ComponentTransform* transform = nullptr;

private:

	void WorldAlign();
	void ScreenAlign();
	void AxialAlign();

	ComponentCamera* reference = nullptr;

};

#endif // !__COMPONENTBILLBOARD_H__