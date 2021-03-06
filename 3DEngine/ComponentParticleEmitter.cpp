#include "ComponentParticleEmitter.h"
#include "pcg-c-basic-0.9/pcg_basic.h"
#include "Application.h"
#include "MathGeoLib/MathGeoLib.h"

#include "mmgr/mmgr.h"

ComponentParticleEmitter::ComponentParticleEmitter()
{
	type = PARTICLE_EMITTER;

	emitterLifetime = -1;

	//Default Values
	speed.max = 5;
	speed.min = 0;

	particleLifetime.max = 5;
	particleLifetime.min = 1;

	startSize.max = 10;
	startSize.min = 9;

	endSize.max = 5;
	endSize.min = 4;

	startSpin.max = 2;
	startSpin.min = 1;

	endSpin.max = 0.5;
	endSpin.min = 0;

	direction = { 0,1,0 };
	gravity = { 1, 0, 0 };
	dirVartiation = 180;

	startColor.max = { 1.0f, 0.0f, 0.0f, 1.0f };
	endColor.max = { 0.0f, 0.0f, 1.0f, 1.0f };

	period = 0.1f;

	maxParicles = 100;

	area.sphere.r = 1;

	area.aabb.minPoint.Set(-0.5, -0.5, -0.5);
	area.aabb.maxPoint.Set(0.5, 0.5, 0.5);

	baseParticle.billboard = new ComponentBillboard();
}


ComponentParticleEmitter::~ComponentParticleEmitter()
{
}

bool ComponentParticleEmitter::Start()
{

	//baseParticle.billboard = App->resourceManager->GetBillboard();





	return true;
}

bool ComponentParticleEmitter::Update()
{
	float dt = App->timeManager->GetRealDeltaTime();

	if (App->timeManager->GetPlaying())
		dt = App->timeManager->GetDeltaTime();

	if (time <= emitterLifetime || emitterLifetime < 0)
	{
		if (currentParticles <= maxParicles || maxParicles == 0)
			SpawnParticles(dt);

		if (emitterLifetime > 0)
			time += dt;

	}
	
	UpdateParticles(dt);
	DrawSpawnArea();
	UpdateSpawnAreaPos();

	return true;
}

void ComponentParticleEmitter::UpdateUI()
{
	if (ImGui::CollapsingHeader("Particle Emitter"))
	{
		ImGui::NewLine();
		ImGui::Text("Emitter");
		ImGui::Separator();

		//Emitter Lifetime
		ImGui::Text("Set to -1 for infinite lifetime");
		if (ImGui::SliderFloat("LifeTime", &emitterLifetime, -1, 100))
			time = 0;
		

		if (ImGui::Button("Reset"))
			time = 0;

		ImGui::Text("LifeTime: %.2f", emitterLifetime - time);

		ImGui::NewLine();
		int particles = maxParicles;

		ImGui::Text("Set to 0 for uncapped particles");
		if (ImGui::SliderInt("Max particles", &particles, 0, 1000))
			maxParicles = particles;


		ImGui::SliderFloat("Period", &period, MINSPAWNRATE, 10);


		//Area of spawn

		if (ImGui::CollapsingHeader("Spawn Area"))
		{

			if (ImGui::Selectable("AABB", area.type == AAB))
			{
				area.type = AAB;
			}
			if (ImGui::Selectable("Sphere", area.type == SPHERE))
			{
				area.type = SPHERE;
			}
			if (ImGui::Selectable("Point", area.type == NONE))
			{
				area.type = NONE;
			}

			ImGui::Separator();
			UpdateSpawnUI();
		}


		if (ImGui::CollapsingHeader("Particle"))
		{

			//Direction
			ImGui::DragFloat3("Direction", (float*)&direction, 0.1f);
			ImGui::SliderFloat("Direction Variation", &dirVartiation, 0, 180);
			ImGui::DragFloat3("Gravity", (float*)&gravity, 0.1f);


			//LifeTime
			float minlife = particleLifetime.min;
			float maxlife = particleLifetime.max;

			ImGui::PushID("LT");

			ImGui::Text("Particle Life Time");
			ImGui::SliderFloat("Min", &particleLifetime.min, 0, particleLifetime.max);
			ImGui::SliderFloat("Max", &particleLifetime.max, particleLifetime.min, 100);


			ImGui::PopID();

			//Speed
			ImGui::PushID("Speed Variation");

			ImGui::Text("Speed");
			ImGui::SliderFloat("Min", &speed.min, 0, speed.max);
			ImGui::SliderFloat("Max", &speed.max, speed.min, 100);

			ImGui::PopID();

			//Start Size
			ImGui::PushID("SSize");

			ImGui::Text("Start Size");
			ImGui::SliderFloat("Min", &startSize.min, 0, startSize.max);
			ImGui::SliderFloat("Max", &startSize.max, startSize.min, 100);

			ImGui::PopID();

			//End Size
			ImGui::PushID("ESize");

			ImGui::Text("End Size");
			ImGui::SliderFloat("Min", &endSize.min, 0, endSize.max);
			ImGui::SliderFloat("Max", &endSize.max, endSize.min, 100);

			ImGui::PopID();

			////Start Spin
			//ImGui::PushID("SSpin");

			//ImGui::Text("Start Spin");
			//ImGui::SliderFloat("Min", &startSpin.min, 0, startSpin.max);
			//ImGui::SliderFloat("Max", &startSpin.max, startSpin.min, 100);

			//ImGui::PopID();

			////End Spin
			//ImGui::PushID("ESpin");

			//ImGui::Text("End Spin");
			//ImGui::SliderFloat("Min", &endSpin.min, 0, endSpin.max);
			//ImGui::SliderFloat("Max", &endSpin.max, endSpin.min, 100);

			//ImGui::PopID();

			//Start Color
			ImGui::PushID("SColor");

			ImGui::Text("StartColor");

			ImGui::ColorEdit4("Min", (float*)&startColor.min);
			ImGui::ColorEdit4("Max", (float*)&startColor.max);

			ImGui::PopID();

			//End Color
			ImGui::PushID("EColor");

			ImGui::Text("EndColor");

			ImGui::ColorEdit4("Min", (float*)&endColor.min);
			ImGui::ColorEdit4("Max", (float*)&endColor.max);

			ImGui::PopID();
		}

		baseParticle.billboard->UpdateUI();
	}
	ImGui::Separator();
}

bool ComponentParticleEmitter::CleanUp()
{
	for (std::list<Particle*>::iterator item = particles.begin(); item != particles.end();)
	{
		(*item)->CleanUp();
		if (*item != nullptr)
		{
			delete *item;
			*item = nullptr;
		}

		item = particles.erase(item);
		
	}

	baseParticle.billboard->CleanUp();
	delete baseParticle.billboard;


	return false;
}

bool ComponentParticleEmitter::Save(JSON_Object * json, JsonDoc * doc)
{
	json_object_dotset_number(json, "Type", type);
	json_object_dotset_boolean(json, "Active", active);

	//Emitter Info
	json_object_dotset_number(json, "Emmision Time",emisionTime);
	json_object_dotset_number(json, "Period", period);
	json_object_dotset_number(json, "Max Particles", maxParicles);

	//Particle Info
	doc->SaveRange(json, "Speed", speed);

	doc->SaveRange(json, "Life", particleLifetime);

	doc->SaveRange(json, "StartSize", startSize);
	doc->SaveRange(json, "EndSize", endSize);

	doc->SaveRange(json, "StartSpin", startSpin);
	doc->SaveRange(json, "EndSpin", endSpin);
	
	doc->SaveColor(json, "StartColorMin", startColor.min);
	doc->SaveColor(json, "StartColorMax", startColor.max);

	doc->SaveColor(json, "EndColorMin", endColor.min);
	doc->SaveColor(json, "EndColorMax", endColor.max);

	doc->SaveFloat3(json, "Gravity", gravity);
	doc->SaveFloat3(json, "Direction", direction);

	json_object_dotset_number(json, "DirVariation", dirVartiation);

	//Area of Spawn
	json_object_dotset_number(json, "AreaType", area.type);

	doc->SaveFloat3(json, "AABBSize", area.aabb.Size());
	json_object_dotset_number(json, "SphereSize", area.sphere.r);
	
	//Billboard
	if (baseParticle.billboard != nullptr)
	{
		JSON_Object* bill = doc->SetObj(json, "Billboard");

		baseParticle.billboard->Save(bill, doc);
	}

	return true;
}

bool ComponentParticleEmitter::Load(JSON_Object * json, JsonDoc * doc)
{
	active = json_object_dotget_boolean(json, "Active");

	//Emitter Info
	emisionTime = json_object_dotget_number(json, "Emision Time");
	period = json_object_dotget_number(json, "Period");
	maxParicles = json_object_dotget_number(json, "Max Particles");


	//Particle Info
	speed = doc->LoadRange(json, "Speed");
	particleLifetime = doc->LoadRange(json, "Life");
	startSize = doc->LoadRange(json, "StartSize");
	endSize = doc->LoadRange(json, "EndSize");

	startSpin = doc->LoadRange(json, "StartSpin");
	endSpin = doc->LoadRange(json, "EndSpin");

	startColor.min = doc->LoadColor(json, "StartColorMin");
	startColor.max = doc->LoadColor(json, "StartColorMax");

	endColor.min = doc->LoadColor(json, "EndColorMin");
	endColor.max = doc->LoadColor(json, "EndColorMax");

	gravity = doc->LoadFloat3(json, "Gravity");
	direction = doc->LoadFloat3(json, "Direction");

	dirVartiation = json_object_dotget_number(json, "DirVariation");


	//Area of Spawn
	uint type = json_object_dotget_number(json, "AreaType");
	area.type = (AreaType)type;

	float3 size = doc->LoadFloat3(json, "AABBSize");
	area.aabb.SetFromCenterAndSize(parent->transform->position, size);

	area.sphere.r = json_object_dotget_number(json, "SphereSize");

	//Billboard
	JSON_Object* bill = doc->GetObjObj(json, "Billboard");

	if (bill != nullptr)	
		baseParticle.billboard->Load(bill, doc);

	

	return true;
}

void ComponentParticleEmitter::CopyStats(ComponentParticleEmitter * emitter)
{
	emitter->active = active;
	emitter->emisionTime = emisionTime;
	emitter->period = period;
	emitter->maxParicles = maxParicles;

	emitter->speed = speed;
	emitter->emitterLifetime = emitterLifetime;
	emitter->startSize = startSize;
	emitter->endSize = endSize;

	emitter->startSpin = startSpin;
	emitter->endSpin = endSpin;
	emitter->startColor = startColor;
	emitter->endColor = endColor;
	
	emitter->gravity = gravity;
	emitter->direction = direction;
	emitter->dirVartiation = dirVartiation;

	emitter->area = area;

	*emitter->baseParticle.billboard = *baseParticle.billboard;

}

void ComponentParticleEmitter::CreateParticle()
{

	//Pick a random direction from the base direction with a the angle vartiation
	float3 randomInSphere = float3::RandomSphere(lcg, { 0,0,0 }, 1);
	float3 vartiation = randomInSphere.Normalized();
	vartiation.x = vartiation.x * dirVartiation * DEGTORAD; 
	vartiation.y = vartiation.y * dirVartiation * DEGTORAD;
	vartiation.z = vartiation.z * dirVartiation * DEGTORAD;


	float3 dir = direction + vartiation;

	baseParticle.Set(GetRandom(startSize), GetRandom(endSize), GetRandom(startSpin), GetRandom(endSpin), GetRandom(speed), GetRandom(particleLifetime) , GetRandomPosition(), dir, gravity, GetRandom(startColor), GetRandom(endColor));


	//Create New Particle
	Particle* newParticle = new Particle(baseParticle);
	newParticle->Start();
	particles.push_back(newParticle);


}

void ComponentParticleEmitter::SpawnParticles(float dt)
{
	uint particlesToSpawn = 1;

	if (period < dt)
	{
		particlesToSpawn = dt / period;
	}

	if (emisionTimer.Read() * dt >= period)
	{
		for (uint i = 0; i < particlesToSpawn; i++)
		{
			CreateParticle();
			currentParticles++;
		}

		emisionTimer.Start();
	}
}

void ComponentParticleEmitter::UpdateParticles(float dt)
{

	for (std::list<Particle*>::iterator item = particles.begin(); item != particles.end();)
	{
		if (!(*item)->Delete())
		{
			(*item)->Update(dt);
			orderedParticles.push(*item);
			item++;
		}
		else
		{
			currentParticles--;
			(*item)->CleanUp();
			if (*item != nullptr)
			{
				delete *item;
				*item = nullptr;
			}

			item = particles.erase(item);
		}
	}

	DrawParticles();
}

void ComponentParticleEmitter::DrawParticles()
{

	while (orderedParticles.size() != 0)
	{
		Particle* first = orderedParticles.top();

		float distance = first->DistanceToCamera();

		first->Draw();

		orderedParticles.pop();
	}


}

void ComponentParticleEmitter::Set(float minSpeed, float maxSpeed, float minLife, float maxLife, float minSSize, float maxSSize, float minESize, float maxESize, float minSSpin, float maxSSpin, float minESpin, float maxESpin, Color sColorMin, Color sColorMax, Color eColorMin, Color eColorMax, float variation, float3 direction, float3 gravity)
{
	speed.min = minSpeed;
	speed.max = maxSpeed;
	particleLifetime.min = minLife;
	particleLifetime.max = maxLife;
	startSize.min = minSSize;
	startSize.max = maxSSize;
	endSize.min = minESize;
	endSize.max = maxESize;
	startSpin.min = minSSpin;
	startSpin.max = maxSSpin;
	endSpin.min = minESpin;
	endSpin.max = maxESpin;

	startColor.min = sColorMin;
	startColor.max = sColorMax;
	endColor.min = eColorMin;
	endColor.max = eColorMax;

	gravity;
	direction;
	dirVartiation = dirVartiation;
}

float ComponentParticleEmitter::GetRandom(range<float> r)
{
	return (ldexp(pcg32_random(), -32) * (r.max - r.min)) + r.min;
}

uint ComponentParticleEmitter::GetRandom(range<uint> r)
{
	return (ldexp(pcg32_random(), -32) * (r.max - r.min)) + r.min;
}

Color ComponentParticleEmitter::GetRandom(range<Color> r)
{
	Color c;
	c.r = (ldexp(pcg32_random(), -32) * (r.max.r - r.min.r)) + r.min.r;
	c.g = (ldexp(pcg32_random(), -32) * (r.max.g - r.min.g)) + r.min.g;
	c.b = (ldexp(pcg32_random(), -32) * (r.max.b - r.min.b)) + r.min.b;
	c.a = (ldexp(pcg32_random(), -32) * (r.max.a - r.min.a)) + r.min.a;

	return c;
}

float3 ComponentParticleEmitter::GetRandomPosition()
{
	float3 ret = float3::zero;

	switch (area.type)
	{
	case SPHERE:
		ret = area.sphere.RandomPointInside(lcg);
		break;
	case AAB:
		ret = area.aabb.RandomPointInside(lcg);
		break;
	case NONE:
		ret = parent->transform->position;
		break;
	default:
		break;
	}

	return ret;
}

void ComponentParticleEmitter::DrawSpawnArea()
{
	switch (area.type)
	{
	case SPHERE:
		App->renderer3D->DrawSphere(area.sphere);
		break;
	case AAB:
		App->renderer3D->DrawAABB(area.aabb);
		break;
	case NONE:
	default:
		break;
	}
}

void ComponentParticleEmitter::UpdateSpawnAreaPos()
{
	switch (area.type)
	{
	case SPHERE:
		area.sphere.pos = parent->transform->position;
		break;
	case AAB:
		area.aabb.SetFromCenterAndSize(parent->transform->position, area.aabb.Size());
		break;
	case NONE:
	default:
		break;
	}

}

void ComponentParticleEmitter::UpdateSpawnUI()
{
	switch (area.type)
	{
	case SPHERE:

		ImGui::DragFloat("Radius", &area.sphere.r, 0.1f);
		break;
	case AAB:
	{
		float3 size = area.aabb.Size();
		if (ImGui::DragFloat3("Size", (float*)&size, 0.1f))
		{
			area.aabb.SetFromCenterAndSize(area.aabb.CenterPoint(), size);
		}
	}
		break;
	case NONE:
	default:
		break;
	}
}

ParticleInfo ComponentParticleEmitter::GetBaseParticle()
{
	return baseParticle;
}

void ComponentParticleEmitter::SetArea(AreaType type)
{
	area.type = type;
}
