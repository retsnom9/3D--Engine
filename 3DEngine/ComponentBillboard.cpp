#include "ComponentBillboard.h"
#include "Application.h"

#include "GameObject.h"
#include "ComponentCamera.h"
#include "ComponentTransform.h"
#include "ResMesh.h"
#include "ResTexture.h"

#include "Glew/include/glew.h"
#include "SDL\include\SDL_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>

ComponentBillboard::ComponentBillboard(ComponentMaterial* mat)
{
	transform = new ComponentTransform();
	mesh = App->resourceManager->GetBillboard();

	if (mat == nullptr)
	{
		material = new ComponentMaterial();
		material->texEnabled = false;
	}
	else
		material = mat;

	alignment = WORLD_ALIGN;

}


ComponentBillboard::~ComponentBillboard()
{
}

bool ComponentBillboard::Start()
{

	return true;
}

bool ComponentBillboard::Update()
{

	FaceCamera();


	//if (!this->parent->GetStatic() || this->parent->IsInQT())
	//{
	//	if (reference->CheckInside(this->parent->GetGlobalABB()))
	//		App->renderer3D->ToDraw(this);
	//}

	transform->CalcMatrix();

	if (material != nullptr)
		App->renderer3D->SetUpMat(material);

	glPushMatrix();
	if (parent != nullptr)
	{
		transform->position = parent->transform->position;
		glMultMatrixf((parent->transform->globalMartix.Transposed() * transform->localMartix.Transposed()).ptr());
	}
	else
		glMultMatrixf(transform->localMartix.Transposed().ptr());

	//UpdateFromParticle();
	mesh->Draw();


	glPopMatrix();

	return true;
}

void ComponentBillboard::UpdateUI()
{
	if (ImGui::CollapsingHeader("Billboard"))
	{

		ImGui::NewLine();

		if (material == nullptr)
		{
			if (ImGui::Button("Add Material"))
				ImGui::OpenPopup("select");

			if (ImGui::BeginPopup("select"))
			{
				UpdateMatWin();
				ImGui::EndPopup();
			}
		}
		else
		{
			ImGui::NewLine();
			material->UpdateUI();
		}

		if (ImGui::Button("Change Material"))
			ImGui::OpenPopup("ChangeMaterial");

		if (ImGui::BeginPopup("ChangeMaterial"))
		{
			UpdateMatWin();
			ImGui::EndPopup();
		}
		

		ImGui::NewLine();
		ImGui::Separator();
		if (ImGui::Button("Delete"))
			Delete();
	}
}

bool ComponentBillboard::CleanUp()
{
	return true;
}

void ComponentBillboard::FaceCamera()
{

	if (App->renderer3D->IsUsingGhostCam())
		reference = App->scene->GetGhostCam();
	else
		reference = App->scene->GetCurCam();


	switch (alignment)
	{
	case SCREEN_ALIGN:
		ScreenAlign();
		break;
	case WORLD_ALIGN:
		WorldAlign();
		break;
	case AXIAL_ALIGN:
		AxialAlign();
		break;
	default:
		break;
	}
}

void ComponentBillboard::SetTexture(ComponentMaterial * t)
{
	material = t;
}

void ComponentBillboard::WorldAlign()
{
	float3 normal = (reference->transform.position - transform->position).Normalized();
	float3 up = reference->GetFrustum().up;
	float3 right = normal.Cross(up);
	float3 upF = normal.Cross(right);

	float3x3 mat = float3x3::identity;
	mat.Set(right.x, right.y, right.z, upF.x, upF.y, upF.z, normal.x, normal.y, normal.z);

	transform->rotation = mat.Inverted().ToQuat();
}

void ComponentBillboard::ScreenAlign()
{
	float3 normal = (reference->transform.position - transform->position).Normalized();
	float3 up = reference->GetFrustum().up;
	float3 right = normal.Cross(up);

	float3x3 mat = float3x3::identity;
	mat.Set(-right.x, -right.y, -right.z, up.x, up.y, up.z, normal.x, normal.y, normal.z);

	transform->rotation = mat.Inverted().ToQuat();
}

void ComponentBillboard::UpdateFromParticle(float3 pos, float scale, float3 color)
{
	Update();
	transform->position = pos;
}

void ComponentBillboard::AxialAlign()
{
	float3 normal = (reference->transform.position - transform->position).Normalized();
	float3 up;
	up.Set(0.0f, 1.0f, 0.0f);
	float3 right = normal.Cross(up);

	float3x3 mat = float3x3::identity;
	mat.Set(right.x, right.y, right.z, 0.0f, 0.0f, 0.0f, normal.x, normal.y, normal.z);

	transform->rotation = mat.Inverted().ToQuat();
	transform->globalMartix.Set(1, 2, 0.0f);
	transform->globalMartix.Set(2, 2, 1.0f);
	transform->globalMartix.Set(3, 2, 0.0f);
}
