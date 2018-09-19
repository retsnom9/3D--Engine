#include "ModuleGui.h"
#include "Application.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_sdl.h"
#include "ImGui/imgui_impl_opengl2.h"


ModuleGui::ModuleGui(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}


ModuleGui::~ModuleGui()
{
}


bool ModuleGui::Start()
{
	ImGui::CreateContext();

	ImGui_ImplSDL2_InitForOpenGL(App->window->window, App->renderer3D->context);
	ImGui_ImplOpenGL2_Init();

	return true;
}

update_status ModuleGui::PreUpdate(float dt)
{
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame(App->window->window);
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();

	if (ImGui::Button("close"))
		quit = true;

	if (ImGui::Button("random float"))
	{
		random_f = ldexp(pcg32_random_r(&rng), -32);
	}
	ImGui::Text("%.2f", random_f);

	ImGui::InputInt("Min", &min);
	ImGui::InputInt("Max", &max);
	if (ImGui::Button("random number between two numbers"))
	{
		if (max - min > 0)
			random_bounded = (int)pcg32_boundedrand_r(&rng, max - min + 1) + min;
		else
			random_bounded = 0;
	}
	ImGui::Text("%d", random_bounded);
	return UPDATE_CONTINUE;
}



update_status ModuleGui::Update(float dt)
{
	return UPDATE_CONTINUE;
}


update_status ModuleGui::PostUpdate(float dt)
{
	update_status status = UPDATE_CONTINUE;

	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

	if (quit)
		status = UPDATE_STOP;

	return status;
}

bool ModuleGui::CleanUp()
{
	//ImGui_ImplSdl_Shutdown();

	LOG("Cleaning UP IMGUI Module")
		bool ret = true;

	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	return true;
}