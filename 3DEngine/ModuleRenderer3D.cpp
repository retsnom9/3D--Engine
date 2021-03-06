#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleWindow.h"
#include "JsonDoc.h"
#include "WinConsole.h"
#include "ResMesh.h"
#include "ComponentMesh.h"
#include "ComponentCamera.h"
#include "QuadTree.h"

#include "Glew/include/glew.h"
#include "SDL\include\SDL_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include "glut/glut.h"


#include "mmgr/mmgr.h"


#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment (lib, "Glew/libx86/glew32.lib")  
#pragma comment (lib, "glut/glut32.lib")

ModuleRenderer3D::ModuleRenderer3D(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name = "Renderer";
}

// Destructor
ModuleRenderer3D::~ModuleRenderer3D()
{}

// Called before render is available
bool ModuleRenderer3D::Init()
{
	VSLOG("Creating 3D Renderer context");
	bool ret = true;
	
	//Load from config
	Load(App->config.GetObj(name.c_str()));

	//Setting attributes
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);


	//Create context
	context = SDL_GL_CreateContext(App->window->window);


	GLenum err = glewInit();
	// ... check for errors
	VSLOG("Using Glew %s", glewGetString(GLEW_VERSION));


	if(context == NULL)
	{
		VSLOG("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	
	if(ret == true)
	{
		//Use Vsync
		if(vsync && SDL_GL_SetSwapInterval(1) < 0)
			VSLOG("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());

		//Initialize Projection Matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		//Check for error
		GLenum error = glGetError();
		if(error != GL_NO_ERROR)
		{
			VSLOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		//Initialize Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
		{
			VSLOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}
		
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glClearDepth(1.0f);
		
		//Initialize clear color
		glClearColor(0.f, 0.f, 0.f, 1.f);

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
		{
			VSLOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}
		
		GLfloat LightModelAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);
		
		lights[0].ref = GL_LIGHT0;
		lights[0].ambient.Set(0.25f, 0.25f, 0.25f, 1.0f);
		lights[0].diffuse.Set(0.75f, 0.75f, 0.75f, 1.0f);
		lights[0].SetPos(0.0f, 0.0f, 2.5f);
		lights[0].Init();
		
		GLfloat MaterialAmbient[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

		GLfloat MaterialDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialDiffuse);
		
		if (depthTest) glEnable(GL_DEPTH_TEST);
		if (cullface) glEnable(GL_CULL_FACE);
		lights[0].Active(true);
		if(lightning) glEnable(GL_LIGHTING);
		if (colorMaterial) glEnable(GL_COLOR_MATERIAL);
		if(texture2D) glEnable(GL_TEXTURE);
	}

	// Projection matrix for
	//OnResize(App->window->w, App->window->h);

	// Create Primitives

	//cube.Create();
	plane.Create(0.0f, 0.0f, 0.0f, 100.0f);
	//axis.Create();
	//line.Create(3.0f);
	//arrow.Create(3.0f);
	//sphere.Create();
	//cylinder.Create();
	//capsule.Create();
	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate(float dt)
{
	
	ComponentCamera* cam;

	if (useGhostCam)
		cam = App->scene->GetGhostCam();
	else
		cam = App->scene->GetCurCam();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(cam->GetViewMatrix());

	// light 0 on cam pos
	//lights[0].SetPos(cam->Position.x,cam->Position.y, cam->Position.z);
	
	//Color c = App->camera->background;
	//glClearColor(c.r, c.g, c.b, c.a);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glMatrixMode(GL_MODELVIEW);
	//glLoadMatrixf(cam->GetOpenGLViewMatrix());

	if (drawPlane)
		plane.Render();


	// Fill objets to draw list
	if (App->scene->quadTree.GetRoot() != nullptr)
		App->scene->quadTree.GetRoot()->CollectIntersections(cam->GetFrustum());


	//Draw Meshes queue
	DrawQueue(opaqueMeshes);
	DrawQueue(transparentMeshes);

	for(uint i = 0; i < MAX_LIGHTS; ++i)
		lights[i].Render();

	if (App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
		fw.Set();

	fw.Update();

	return UPDATE_CONTINUE;
}

// PostUpdate present buffer to screen
update_status ModuleRenderer3D::PostUpdate(float dt)
{
	SDL_GL_SwapWindow(App->window->window);
	
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleRenderer3D::CleanUp()
{
	VSLOG("Destroying 3D Renderer");

	SDL_GL_DeleteContext(context);

	glDeleteBuffers(1, (GLuint*) &(plane.my_id));
	
	return true;
}


void ModuleRenderer3D::SetUseGhostCam(bool s)
{
	useGhostCam = s;
}

void ModuleRenderer3D::OnResize(int width, int height)
{


	if (useGhostCam)
		renderedCam = App->scene->GetGhostCam();
	else
		renderedCam = App->scene->GetCurCam();
	
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	App->window->w = width;
	App->window->h = height;
	ProjectionMatrix = renderedCam->ResizePerspMatrix(width, height);
	glLoadMatrixf(&ProjectionMatrix[0][0]);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

bool ModuleRenderer3D::Load(json_object_t* doc)
{

	vsync = App->config.GetObjValueInt(doc, "Vsync");
	return true;
}

bool ModuleRenderer3D::Save(json_object_t* doc)
{
	json_object_dotset_boolean(doc, "Vsync", vsync);
	return true;
}


void ModuleRenderer3D::EnableVsync(bool enable)
{
	vsync = !vsync;
	vsync == true ? (SDL_GL_SetSwapInterval(1) < 0) : (SDL_GL_SetSwapInterval(0) < 0);
}

void ModuleRenderer3D::EnableDepthTest(bool enable)
{
	depthTest = enable;
	depthTest == true ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
}

void ModuleRenderer3D::EnableCullFace(bool enable)
{
	cullface = enable;
	cullface == true ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
}

void ModuleRenderer3D::EnableLighting(bool enable)
{
	lightning = enable;
	lightning == true ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
}


void ModuleRenderer3D::EnableColorMaterial(bool enable)
{
	colorMaterial = enable;
	colorMaterial == true ? glEnable(GL_COLOR_MATERIAL) : glDisable(GL_COLOR_MATERIAL);
}

void ModuleRenderer3D::EnableTexture2D(bool enable)
{
	texture2D = enable;
	texture2D == true ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
}

void ModuleRenderer3D::EnableWireframe(bool enable)
{
	wireframe = enable;
	wireframe == true ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

bool ModuleRenderer3D::IsUsingGhostCam() const
{
	return useGhostCam;
}

void ModuleRenderer3D::ToDraw(ComponentMesh * mesh)
{
	ResTexture* tex = mesh->material->GetTexture();

	//Check if the material is transparent


	if (tex != nullptr && tex->transparent)
		transparentMeshes.push(mesh);
	
	else
		opaqueMeshes.push(mesh);
}

void ModuleRenderer3D::DrawQueue(std::priority_queue<ComponentMesh*, std::vector<ComponentMesh*>, MeshPriority>& queue)
{

	while (!queue.empty())
	{
		ComponentMesh* first = queue.top();

		first->Draw();

		queue.pop();
	}
}

void ModuleRenderer3D::ClearQueues()
{

	//while (!opaqueMeshes.empty())
	//	opaqueMeshes.pop();

	//while (!transparentMeshes.empty())
	//	transparentMeshes.pop();
	
}



float3 ModuleRenderer3D::GetCameraPos() const
{

	return renderedCam->GetFrustum().pos;
}

void ModuleRenderer3D::SetUpMat(ComponentMaterial* mat)
{
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (mat->GetTexture() != nullptr && mat->texEnabled)
		glBindTexture(GL_TEXTURE_2D, mat->GetTexture()->id);
	else
		glColor4f(mat->color.r, mat->color.g, mat->color.b, mat->color.a);
}



void ModuleRenderer3D::DrawFrustum(math::Frustum f)
{
	float3 point[8];
	f.GetCornerPoints(point);

	glColor4f(0.8f, 0.5f, 0.0f, 1.0f);
	glLineWidth(2);

	glBegin(GL_LINES);
	
	glVertex3f(point[0].x, point[0].y, point[0].z);
	glVertex3f(point[1].x, point[1].y, point[1].z);

	glVertex3f(point[0].x, point[0].y, point[0].z);
	glVertex3f(point[2].x, point[2].y, point[2].z);

	glVertex3f(point[2].x, point[2].y, point[2].z);
	glVertex3f(point[3].x, point[3].y, point[3].z);

	glVertex3f(point[1].x, point[1].y, point[1].z);
	glVertex3f(point[3].x, point[3].y, point[3].z);



	glVertex3f(point[7].x, point[7].y, point[7].z);
	glVertex3f(point[6].x, point[6].y, point[6].z);

	glVertex3f(point[7].x, point[7].y, point[7].z);
	glVertex3f(point[5].x, point[5].y, point[5].z);

	glVertex3f(point[5].x, point[5].y, point[5].z);
	glVertex3f(point[4].x, point[4].y, point[4].z);

	glVertex3f(point[6].x, point[6].y, point[6].z);
	glVertex3f(point[4].x, point[4].y, point[4].z);

	glVertex3f(point[0].x, point[0].y, point[0].z);
	glVertex3f(point[4].x, point[4].y, point[4].z);

	glVertex3f(point[1].x, point[1].y, point[1].z);
	glVertex3f(point[5].x, point[5].y, point[5].z);

	glVertex3f(point[2].x, point[2].y, point[2].z);
	glVertex3f(point[6].x, point[6].y, point[6].z);

	glVertex3f(point[3].x, point[3].y, point[3].z);
	glVertex3f(point[7].x, point[7].y, point[7].z);

	glEnd();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glLineWidth(1);
}

void ModuleRenderer3D::DrawAABB(math::AABB a, Color c)
{
	glColor4f(c.r, c.g, c.b, c.a);
	glLineWidth(2);
	
	float3 length = a.maxPoint - a.minPoint;
	float3 min = a.minPoint;
	float3 max = a.maxPoint;

	glBegin(GL_LINES);
	glVertex3f(min.x, min.y, min.z);
	glVertex3f(min.x + length.x, min.y, min.z);

	glVertex3f(min.x, min.y, min.z);
	glVertex3f(min.x, min.y + length.y, min.z);

	glVertex3f(min.x, min.y, min.z);
	glVertex3f(min.x, min.y, min.z + length.z);

	glVertex3f(min.x, min.y + length.y, min.z);
	glVertex3f(min.x + length.x, min.y + length.y, min.z);

	glVertex3f(min.x, min.y + length.y, min.z);
	glVertex3f(min.x, min.y + length.y, min.z + length.z);

	glVertex3f(min.x + length.x, min.y, min.z);
	glVertex3f(min.x + length.x, min.y + length.y, min.z);

	glVertex3f(min.x, min.y, min.z + length.z);
	glVertex3f(min.x, min.y + length.y, min.z + length.z);

	glVertex3f(max.x, max.y, max.z);
	glVertex3f(max.x - length.x, max.y, max.z);

	glVertex3f(max.x, max.y, max.z);
	glVertex3f(max.x, max.y - length.y, max.z);

	glVertex3f(max.x, max.y, max.z);
	glVertex3f(max.x, max.y, max.z - length.z);

	glVertex3f(max.x, max.y - length.y, max.z);
	glVertex3f(max.x - length.x, max.y - length.y, max.z);

	glVertex3f(max.x, max.y - length.y, max.z);
	glVertex3f(max.x, max.y - length.y, max.z - length.z);

	glEnd();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glLineWidth(1);
}

void ModuleRenderer3D::DrawOBB(math::OBB a, Color c)
{
	float3 point[8];
	a.GetCornerPoints(point);

	glColor4f(c.r, c.g, c.b, c.a);
	glLineWidth(2);

	glBegin(GL_LINES);

	glVertex3f(point[0].x, point[0].y, point[0].z);
	glVertex3f(point[1].x, point[1].y, point[1].z);

	glVertex3f(point[0].x, point[0].y, point[0].z);
	glVertex3f(point[2].x, point[2].y, point[2].z);

	glVertex3f(point[2].x, point[2].y, point[2].z);
	glVertex3f(point[3].x, point[3].y, point[3].z);

	glVertex3f(point[1].x, point[1].y, point[1].z);
	glVertex3f(point[3].x, point[3].y, point[3].z);



	glVertex3f(point[7].x, point[7].y, point[7].z);
	glVertex3f(point[6].x, point[6].y, point[6].z);

	glVertex3f(point[7].x, point[7].y, point[7].z);
	glVertex3f(point[5].x, point[5].y, point[5].z);

	glVertex3f(point[5].x, point[5].y, point[5].z);
	glVertex3f(point[4].x, point[4].y, point[4].z);

	glVertex3f(point[6].x, point[6].y, point[6].z);
	glVertex3f(point[4].x, point[4].y, point[4].z);

	glVertex3f(point[0].x, point[0].y, point[0].z);
	glVertex3f(point[4].x, point[4].y, point[4].z);

	glVertex3f(point[1].x, point[1].y, point[1].z);
	glVertex3f(point[5].x, point[5].y, point[5].z);

	glVertex3f(point[2].x, point[2].y, point[2].z);
	glVertex3f(point[6].x, point[6].y, point[6].z);

	glVertex3f(point[3].x, point[3].y, point[3].z);
	glVertex3f(point[7].x, point[7].y, point[7].z);

	glEnd();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glLineWidth(1);
}

void ModuleRenderer3D::DrawSphere(math::Sphere sphere, Color c)
{
	int division = 10;

	glColor4f(c.r, c.g, c.b, c.a);
	glPushMatrix();
	glTranslatef(sphere.pos.x, sphere.pos.y, sphere.pos.z);
	glutWireSphere(sphere.r, division, division);
	glPopMatrix();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

