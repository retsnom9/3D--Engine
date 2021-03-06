#include "GameObject.h"
#include "Component.h"
#include "Globals.h"
#include "ResMesh.h"
#include "ComponentMesh.h"
#include "ComponentCamera.h"
#include "ComponentBillboard.h"
#include "ComponentParticleEmitter.h"
#include "Application.h"
#include "QuadTree.h"
#include "pcg-c-basic-0.9/pcg_basic.h"

#include "Glew/include/glew.h"
#include "SDL\include\SDL_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>

#include "mmgr/mmgr.h"

GameObject::GameObject()
{
	transform = new ComponentTransform();
	transform->SetParent(this);
	localABB.minPoint.Set(INT_MAX, INT_MAX, INT_MAX);
	localABB.maxPoint.Set(INT_MIN, INT_MIN, INT_MIN);

	globalABB.minPoint.Set(INT_MAX, INT_MAX, INT_MAX);
	globalABB.maxPoint.Set(INT_MIN, INT_MIN, INT_MIN);

	if (uuid == 0)
		uuid = pcg32_random();
}


GameObject::~GameObject()
{
}


bool GameObject::Start()
{
	for (std::vector<GameObject*>::iterator item = objChilds.begin(); item != objChilds.end(); item++)
	{
			(*item)->Start();
	}

	for (std::vector<Component*>::iterator item = compChilds.begin(); item != compChilds.end(); item++)
	{
			(*item)->Start();
	}
	return true;
}

bool GameObject::Update()
{
	for (std::vector<GameObject*>::iterator item = objChilds.begin(); item != objChilds.end();)
	{

		if (!(*item)->CheckDelete())
		{
			if ((*item)->GetActive())
				(*item)->Update();

			item++;
		}
		else
		{
			(*item)->CleanUp();
			if (*item != nullptr)
			{
				delete *item;
				*item = nullptr;
			}

			item = objChilds.erase(item);
		}
	}

	for (std::vector<Component*>::iterator item = compChilds.begin(); item != compChilds.end();)
	{
		if (!(*item)->CheckDelete())
		{
			if ((*item)->GetActive())
				(*item)->Update();

			item++;
		}
		else
		{
			(*item)->CleanUp();
			if (*item != nullptr)
			{
				delete *item;
				*item = nullptr;
			}

			item = compChilds.erase(item);
		}
	}

	
	return true;
}

bool GameObject::PostUpdate()
{

	for (std::vector<GameObject*>::iterator item = objChilds.begin(); item != objChilds.end();)
	{
		
			(*item)->PostUpdate();
			item++;
		
	}

	return true;
}

void GameObject::UpdateUI()
{
	ImGui::PushID("go");
	char n[50];
	strcpy_s(n, 50, name.c_str());
	ImGui::InputText("Name", n, 50);
		name = n;

	ImGui::PopID();

	bool a = active;
	ImGui::Checkbox("Active", &active);
	if (a != active)
		SetChildsActive(active);

	ImGui::SameLine();

	bool s = staticobj;
	ImGui::Checkbox("Static", &staticobj);
	if (s != staticobj)
		SetStatic(staticobj);

	if (ImGui::Checkbox("Draw Bounding Box", &drawBB));


	ImGui::NewLine();
	ImGui::Text("Objects");
	ImGui::Separator();
	for (std::vector<GameObject*>::iterator item = objChilds.begin(); item != objChilds.end(); item++)
	{
		if (ImGui::CollapsingHeader((*item)->GetName()))
			(*item)->UpdateUI();
	}

	ImGui::NewLine();
	ImGui::Text("Components");
	ImGui::Separator();
	for (std::vector<Component*>::iterator item = compChilds.begin(); item != compChilds.end(); item++)
	{
			(*item)->UpdateUI();

	}

	if (ImGui::BeginMenu("Add Component"))
	{
		UpdateNewComWindow();
		ImGui::EndMenu();
	}

	ImGui::NewLine();
	ImGui::Separator();

	//if (ImGui::Button("Copy"))
	//	Copy();

	//ImGui::SameLine();

	//if (ImGui::Button("Delete"))
	//	Delete();
}

bool GameObject::CleanUp()
{
	parent = nullptr;
	
	if (selected)
	{
		App->scene->selectedObj = nullptr;
	}

	for (std::vector<GameObject*>::iterator item = objChilds.begin(); item != objChilds.end(); item++)
	{
			(*item)->CleanUp();
			if (*item != nullptr)
			{
				delete *item;
				*item = nullptr;
			}
	}
	objChilds.clear();

	for (std::vector<Component*>::iterator item = compChilds.begin(); item != compChilds.end(); item++)
	{
			(*item)->CleanUp();
			if (*item != nullptr)
			{
				delete *item;
				*item = nullptr;
			}
	}
	compChilds.clear();
	name.clear();


	return true;
}

void GameObject::Delete()
{
	toDelete = true;
}

bool GameObject::CheckDelete() const
{
	return toDelete;
}

bool GameObject::GetDrawBB() const
{
	return drawBB;
}

bool GameObject::GetActive() const
{
	return active;
}

void GameObject::SetActive(bool b)
{
	active = b;
}

const char* GameObject::GetName() const
{
	return name.c_str();
}

void GameObject::SetName(const char* n)
{
	name = n;
}

void GameObject::SetName(std::string n)
{
	name = n;
}


void GameObject::AddComponent(Component* comp)
{
	GameObject* prevparent = comp->GetParent();
	if (prevparent != this)
	{
		if (prevparent != nullptr)
		{
			Utils::RemoveFromVector(comp, prevparent->compChilds);		
		}

		comp->SetParent(this);
	}

}

void GameObject::AddGameObject(GameObject* obj)
{

	obj->SetParent(this);
	
}

GameObject* GameObject::GetParent() const
{
	return parent;
}


void GameObject::SetParent(GameObject* p, bool changeGlobal)
{
	if (!CheckIfContained(p))
	{
		if (parent != nullptr)
		{
			Utils::RemoveFromVector(this, parent->objChilds);
		}

		parent = p;
		parent->objChilds.push_back(this);

		if (changeGlobal == true)
		{
			////World Space
			float4x4 newlocal = parent->transform->globalMartix.Inverted() * transform->localMartix;

			//newlocal =  p->transform->localMartix * newlocal.Inverted();
			//transform->localMartix = newlocal;
			//transform->globalMartix = p->transform->localMartix;

			transform->localMartix = newlocal;
			CalcGlobalTransform();
		}
			
	}
	else
	{
		VSLOG("Cannot set a child as parent\n");
	}

}

bool GameObject::CheckIfContained(GameObject* obj)
{
	//Check if the element is in the list or the list of the children

	for (std::vector<GameObject*>::iterator item = objChilds.begin(); item != objChilds.end(); item++)
	{
		if ((*item) == obj || (*item)->CheckIfContained(obj))
			return true;
	}
	return false;
}

void GameObject::SetChildsActive(bool a)
{
	for (std::vector<GameObject*>::iterator item = objChilds.begin(); item != objChilds.end(); item++)
	{
		(*item)->SetActive(a);
		(*item)->SetChildsActive(a);
	}

	for (std::vector<Component*>::iterator item = compChilds.begin(); item != compChilds.end(); item++)
	{
		(*item)->SetActive(a);
	}
}

void GameObject::DeleteComp(Component* comp)
{
	Utils::RemoveFromVector(comp, compChilds);
	comp->CleanUp();
	delete comp;
}

void GameObject::DeleteGameObj(GameObject* obj)
{
	obj->Delete();
}

Component* GameObject::AddCompMesh()
{
	Component* newcomp = new ComponentMesh();
	newcomp->SetParent(this);

	return newcomp;
}

Component* GameObject::AddCompMesh(ResMesh* m)
{
	Component* newcomp = new ComponentMesh(m);
	newcomp->SetParent(this);

	return newcomp;
}

Component* GameObject::AddCompCam(float _near , float _far, float fov)
{
	Component* newcomp = new ComponentCamera(_near, _far, fov);
	newcomp->SetParent(this);

	if (App->scene->GetCurCam() == nullptr)
		App->scene->SetCurCam((ComponentCamera*)newcomp);

	return newcomp;
}

Component * GameObject::AddCompBillboard()
{
	Component* newcomp = new ComponentBillboard();
	newcomp->SetParent(this);

	return newcomp;
}

Component * GameObject::AddCompParticleEmitter()
{
	Component* newcomp = new ComponentParticleEmitter();
	newcomp->SetParent(this);

	return newcomp;
}

Component* GameObject::AddComponent(Type t)
{
	Component* newcomp = nullptr;
	switch (t)
	{
	case MESH:
		newcomp = AddCompMesh();
		break;
	case CAMERA:
		newcomp = AddCompCam();
		break;
	case BILLBOARD:
		newcomp = AddCompBillboard();
		break;
	case PARTICLE_EMITTER:
		newcomp = AddCompParticleEmitter();
		break;
	default:
		break;
	}

	newcomp->Start();

	return newcomp;
}

void GameObject::AddBox(AABB b)
{
	if (localABB.minPoint.Equals(INT_MAX, INT_MAX, INT_MAX) && localABB.maxPoint.Equals(INT_MIN, INT_MIN, INT_MIN))
	{
		localABB = b;
		oBB = localABB;
	}

	else
		localABB.Enclose(b);
}

bool GameObject::GetStatic() const
{
	return staticobj;
}
void GameObject::SetStatic(bool b)
{
	staticobj = b;

	if (staticobj)
		App->scene->quadTree.AddObject(this);

	else
		App->scene->quadTree.RemoveObject(this);
}

AABB GameObject::GetLocalABB() const
{
	return localABB;
}

AABB GameObject::GetGlobalABB() const
{
	return globalABB;
}

OBB GameObject::GetOBB() const
{
	return oBB;
}

void GameObject::UpdateNewComWindow()
{
	ImGui::Separator();

	ImGuiTreeNodeFlags node_flags =  ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf;

	Type t = Type::NULLCOMP;
	bool add = false;
	ImGui::TreeNodeEx("Camera", node_flags);

	if (ImGui::IsItemClicked())
	{
		t = CAMERA;
		add = true;
	}
	
	ImGui::TreeNodeEx("Mesh", node_flags);
	if (ImGui::IsItemClicked())
	{
		t = MESH;
		add = true;
	}

	ImGui::TreeNodeEx("Billboard", node_flags);
	if (ImGui::IsItemClicked())
	{
		t = BILLBOARD;
		add = true;
	}

	ImGui::TreeNodeEx("Particle Emitter", node_flags);
	if (ImGui::IsItemClicked())
	{
		t = PARTICLE_EMITTER;
		add = true;
	}

	if (add)
		AddComponent(t);
}



uint32_t GameObject::GetUUID() const
{
	return uuid;
}


GameObject* GameObject::GetObjByUUID(uint32_t id)
{
	//Function to get the game object with the same uuid between the children
	if (id == uuid)
		return this;
	else
	{
		for (std::vector<GameObject*>::iterator item = objChilds.begin(); item != objChilds.end(); item++)
		{
			if ((*item)->GetObjByUUID(id) != nullptr)
			{
				return (*item)->GetObjByUUID(id);
			}
		}
	}

	return nullptr;
}

void GameObject::CalcGlobalTransform()
{

	if  (parent != nullptr)
	{
		transform->globalMartix =  parent->transform->globalMartix * transform->localMartix;
	}
	OBB newobb = localABB;
	newobb.Transform(transform->globalMartix);
	
	oBB = newobb;
	globalABB.SetNegativeInfinity();
	globalABB.Enclose(oBB);

	for (std::vector<GameObject*>::iterator item = objChilds.begin(); item != objChilds.end(); item++)
	{
		(*item)->CalcGlobalTransform();
	}
}

void GameObject::Save(JSON_Array* objects, JsonDoc* doc)
{
	if (uuid == 0)
		uuid = pcg32_random();

	uint32_t parentUuid = 0;
	if (parent != nullptr)
		parentUuid = parent->GetUUID();

	JSON_Object* obj = doc->SetObj(objects);

	//Save Properties
	json_object_dotset_number(obj, "UUID", uuid);
	json_object_dotset_number(obj, "Parent UUID", parentUuid);
	json_object_dotset_string(obj, "Name", name.c_str());
	json_object_dotset_boolean(obj, "Active", active);
	json_object_dotset_boolean(obj, "Static", staticobj);

	//Save Local AABB
	JSON_Array* local = doc->SetArray(obj, "AABB");

	//Min
	json_array_append_number(local, localABB.MinX());
	json_array_append_number(local, localABB.MinY());
	json_array_append_number(local, localABB.MinZ());

	//Max
	json_array_append_number(local, localABB.MaxX());
	json_array_append_number(local, localABB.MaxY());
	json_array_append_number(local, localABB.MaxZ());

	//Save Components
	JSON_Array* components = doc->SetArray(obj, "Components");
	for (std::vector<Component*>::iterator item = compChilds.begin(); item != compChilds.end(); item++)
	{
		JSON_Object* obj = doc->SetObj(components);
		(*item)->Save(obj, doc);
	}

	//Save Object childs
	for (std::vector<GameObject*>::iterator item = objChilds.begin(); item != objChilds.end(); item++)
	{

		(*item)->Save(objects, doc);
	}
	
}
bool GameObject::Load(JSON_Object* json, JsonDoc* doc)
{
	//Returns true if the objects is set as root, so it has to be deleted
	bool ret = false;

	uuid = json_object_dotget_number(json, "UUID");
	uint32_t parentuuid = json_object_dotget_number(json, "Parent UUID");
	name = json_object_dotget_string(json, "Name");

	active = json_object_dotget_boolean(json, "Active");
	staticobj = json_object_dotget_boolean(json, "Static");

	if (parentuuid == 0)
	{
		VSLOG("Object %s set as root", name.c_str())
		App->scene->root = *this;
		ret = true;
	}
	else
	{
		GameObject* parent = App->scene->root.GetObjByUUID(parentuuid);

		if (parent != nullptr)
			SetParent(parent);
		else
		{
			VSLOG("Can't find %s's parent", name.c_str());
			App->scene->AddGameObject(this);
		}
		
	}


	//Loading Local AABB
	JSON_Array* local = doc->GetObjAr(json, "AABB");

	//Min
	float3 min;
	min.x = json_array_get_number(local, 0);
	min.z = json_array_get_number(local, 2);
	min.y = json_array_get_number(local, 1);

	//Max
	float3 max;
	max.x = json_array_get_number(local, 3);
	max.y = json_array_get_number(local, 4);
	max.z = json_array_get_number(local, 5);

	localABB.minPoint = min;
	localABB.maxPoint = max;

	//Loading Components
	JSON_Array* comp = doc->GetObjAr(json, "Components");

	int i = 0;
	JSON_Object* item = doc->GetArObj(comp, i);
	while (item != nullptr)
	{
		uint type = json_object_dotget_number(item, "Type");

		if ((Type)type == TRANSFORM)
			transform->Load(item, doc);
		else
		{
			Component* newcomp = AddComponent((Type)type);
			newcomp->Load(item, doc);
			newcomp->Start();
		}

		item = doc->GetArObj(comp, ++i);
	}

	return ret;
}

void GameObject::IsPickedABB(LineSegment picking, std::vector<GameObject*> &vec)
{
	Deselect();

	if (!objChilds.empty())
	{
		for (vector<GameObject*>::iterator it = objChilds.begin(); it != objChilds.end(); ++it)
		{
			(*it)->IsPickedABB(picking, vec);
		}
	}

	if (globalABB.Intersects(picking))
	{
		//selected = true;
		vec.push_back(this);
	}
	//else
	//	selected = false;
}

void GameObject::Select()
{
	if (App->scene->selectedObj != nullptr)
		App->scene->selectedObj->Deselect();
	selected = true;
	App->scene->selectedObj = this;
	drawBB = true;
	
}

void GameObject::Deselect()
{
	selected = false;
	drawBB = false;
}

bool GameObject::IsSelected() const
{
	return selected;
}

bool GameObject::IsInQT() const
{
	return inQT;
}

void GameObject::SetInQT(bool b)
{
	inQT = b;
}

GameObject* GameObject::Copy(GameObject* p)
{
	//Specifies the parent

	//Copy the object
	GameObject* copy = new GameObject();
	copy->SetName(name + "(" + std::to_string(numCopies) + ")");


	if (p == nullptr)
		copy->SetParent(parent);

	else
		copy->SetParent(p);

	//Set up BB's
	copy->localABB = localABB;
	copy->CalcGlobalTransform();

	//Create New components
	for (std::vector<Component*>::iterator item = compChilds.begin(); item != compChilds.end(); item++)
	{
		switch ((*item)->GetType())
		{
		case TRANSFORM:
			copy->transform = new ComponentTransform(*transform);
			copy->transform->ForceParent(copy);
			break;
		case MESH:
		{
			ComponentMesh * m = new ComponentMesh();

			m->material = ((ComponentMesh*)(*item))->material;
			m->mesh = ((ComponentMesh*)(*item))->mesh;

			m->SetParent(copy);
			m->mesh->AddInMemory();
		}
			break;
		case CAMERA:
		{
			ComponentCamera * c = new ComponentCamera(*(ComponentCamera*)(*item));
			c->SetParent(copy);
		}
			break;
		case PARTICLE_EMITTER:
		{
			ComponentParticleEmitter * e = new ComponentParticleEmitter();
			((ComponentParticleEmitter*)(*item))->CopyStats(e);
			e->SetParent(copy);
		}
		}
	
		
	}

	//Copy object childs
	for (std::vector<GameObject*>::iterator item = objChilds.begin(); item != objChilds.end(); item++)
	{
		GameObject* newchild = (*item)->Copy(copy);

	}

	numCopies++;

	VSLOG("\nCopied Game Object %s", name.c_str());
	return copy;
}
