#include "WinTest.h"
#include "Application.h"
#include "ImGui/imgui.h"


WinTest::WinTest(Application* parent, bool start_enabled) : WinBase(parent, start_enabled)
{
}


WinTest::~WinTest()
{
}

bool WinTest::Update()
{
	ImGui::Begin("Test Window");
	
	if (ImGui::CollapsingHeader("Random Number Generator"))
	{
		ImGui::Text("%.2f", random_f);

		if (ImGui::Button("Generate random float between 0 and 1"))
		{
			random_f = ldexp(pcg32_random_r(&rng), -32);
		}

		ImGui::Separator();

		ImGui::SliderInt("Min", &min, 0, 100);
		ImGui::SliderInt("Max", &max, 0, 100);

		if (max < min)
			max = min;

		ImGui::Text("%d", random_bounded);

		if (ImGui::Button("Generate random integer between two numbers"))
		{
			if (max - min > 0)
				random_bounded = (int)pcg32_boundedrand_r(&rng, max - min + 1) + min;
			else
				random_bounded = 0;
		}
	}

	if (ImGui::CollapsingHeader("Primitives"))
	{
		ImGui::SliderFloat("x", &App->renderer3D->xx, -3, 6);
		ImGui::SliderFloat("y", &App->renderer3D->yy, -3, 6);
		ImGui::SliderFloat("z", &App->renderer3D->zz, -3, 6);
		ImGui::Checkbox("Grid", &App->renderer3D->drawPlane);
	}

	ImGui::End();
	return true;
}