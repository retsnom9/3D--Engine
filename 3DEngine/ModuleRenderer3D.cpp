
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleWindow.h"
#include "JsonDoc.h"
#include "MathGeoLib/MathGeoLib.h"

#include "Glew/include/glew.h"
#include "SDL\include\SDL_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>



#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment (lib, "Glew/libx86/glew32.lib")  

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
		
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		lights[0].Active(true);
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_TEXTURE);
	}

	// Projection matrix for
	OnResize(App->window->w, App->window->h);

	// Create Primitives

	cube.Create();
	plane.Create();
	axis.Create();
	line.Create(3.0f);

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate(float dt)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(App->camera->GetViewMatrix());

	// light 0 on cam pos
	lights[0].SetPos(App->camera->Position.x, App->camera->Position.y, App->camera->Position.z);
	
	//Color c = App->camera->background;
	//glClearColor(c.r, c.g, c.b, c.a);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glMatrixMode(GL_MODELVIEW);
	//glLoadMatrixf(cam->GetOpenGLViewMatrix());
	if (drawCube)
		cube.Render();
	if (drawLine)
		line.Render();
	if (drawArrow)
		Arrow(0, 0, 0, 0, 0.1, 0);
	if (drawAxis)
		axis.Render();
	if (drawPlane)
		plane.Render();
	if (drawSphere)
		Sphere(0.1, 30, 30);


	DrawMeshes();

	for(uint i = 0; i < MAX_LIGHTS; ++i)
		lights[i].Render();

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

	return true;
}


void ModuleRenderer3D::OnResize(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	ProjectionMatrix = perspective(60.0f, (float)width / (float)height, 0.125f, 512.0f);
	glLoadMatrixf(&ProjectionMatrix);

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
	json_object_dotset_boolean(doc, "Renderer.Vsync", vsync);
	return true;
}


void ModuleRenderer3D::EnableVsync()
{
	vsync = !vsync;
	vsync == true ? (SDL_GL_SetSwapInterval(1) < 0) : (SDL_GL_SetSwapInterval(0) < 0);
}

void ModuleRenderer3D::EnableDepthTest()
{
	depthTest = !depthTest;
	depthTest == true ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
}

void ModuleRenderer3D::EnableCullFace() 
{
	cullface = !cullface;
	cullface == true ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
}

void ModuleRenderer3D::EnableLighting()
{
	lightning = !lightning;
	lightning == true ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
}


void ModuleRenderer3D::EnableColorMaterial()
{
	colorMaterial = !colorMaterial;
	colorMaterial == true ? glEnable(GL_COLOR_MATERIAL) : glDisable(GL_COLOR_MATERIAL);
}

void ModuleRenderer3D::EnableTexture2D()
{
	texture2D = !texture2D;
	texture2D == true ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
}

void ModuleRenderer3D::EnableWireframe()
{
	wireframe = !wireframe;
	wireframe == true ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void ModuleRenderer3D::DrawMeshes()
{
	for (std::list<Mesh*>::iterator item = meshes.begin(); item != meshes.end(); item++) 
	{
		(*item)->Draw();
		if (drawNormals)
			(*item)->DrawNormals();
	}
}



void Mesh::GenerateBuffer()
{

	//glGenBuffers(1, (GLuint*) &(id_index));
	//glBindBuffer(GL_ARRAY_BUFFER, id_index);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * num_index, &index[0], GL_STATIC_DRAW);

	glGenBuffers(1, (GLuint*) &(id_vertex));
	glBindBuffer(GL_ARRAY_BUFFER, id_vertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * num_vertex, vertex, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);



}

void Mesh::Draw()
{

	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, id_indice);
	glVertexPointer(3, GL_FLOAT, 0, vertex);
	glDrawElements(GL_TRIANGLES, num_indice, GL_UNSIGNED_INT, indice);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);


}

void Mesh::DrawNormals()
{
	if (normals == nullptr)
		return;


	for (int i = 0; i < num_normals; i += 3)
	{
		glColor4f(0.2f, 0.2f, 1.0f, 1.0f);
		glPointSize(5);
		glBegin(GL_POINTS);
		glVertex3f(vertex[i], vertex[i + 1], vertex[i + 2]);
		glEnd();

		glColor4f(0.2f, 1.0f, 0.2f, 1.0f);
		glLineWidth(1);

		glBegin(GL_LINES);
		glVertex3f(vertex[i], vertex[i + 1], vertex[i + 2]);
		glVertex3f((vertex[i] + normals[i]), vertex[i + 1] + normals[i + 1], vertex[i + 2] + normals[i + 2]);
		glEnd();
		glLineWidth(1);

	}
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}


void ModuleRenderer3D::VertexArraysCube(float origin, float size)
{
	if (!VBufferInit)
	{
		int i = 0;

		vertices[i] = origin;		//A
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin + size;		//B
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin;		//C
		i++;
		vertices[i] = origin + size;
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin + size;		//B
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin + size;		//D
		i++;
		vertices[i] = origin + size;
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin;		//C
		i++;
		vertices[i] = origin + size;
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin + size;		//D
		i++;
		vertices[i] = origin + size;
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin + size;		//E
		i++;
		vertices[i] = origin + size;
		i++;
		vertices[i] = origin - size;
		i++;
		vertices[i] = origin;		//C
		i++;
		vertices[i] = origin + size;
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin + size;		//E
		i++;
		vertices[i] = origin + size;
		i++;
		vertices[i] = origin - size;
		i++;
		vertices[i] = origin;		//F
		i++;
		vertices[i] = origin + size;
		i++;
		vertices[i] = origin - size;
		i++;
		vertices[i] = origin;		//C
		i++;
		vertices[i] = origin + size;
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin;		//C
		i++;
		vertices[i] = origin + size;
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin;		//F
		i++;
		vertices[i] = origin + size;
		i++;
		vertices[i] = origin - size;
		i++;
		vertices[i] = origin;		//G
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin - size;
		i++;
		vertices[i] = origin;		//A
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin;		//C
		i++;
		vertices[i] = origin + size;
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin;		//G
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin - size;
		i++;
		vertices[i] = origin;		//G
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin - size;
		i++;
		vertices[i] = origin;		//F
		i++;
		vertices[i] = origin + size;
		i++;
		vertices[i] = origin - size;
		i++;
		vertices[i] = origin + size;		//E
		i++;
		vertices[i] = origin + size;
		i++;
		vertices[i] = origin - size;
		i++;
		vertices[i] = origin + size;		//E
		i++;
		vertices[i] = origin + size;
		i++;
		vertices[i] = origin - size;
		i++;
		vertices[i] = origin + size;		//H
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin - size;
		i++;
		vertices[i] = origin;		//G
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin - size;
		i++;
		vertices[i] = origin + size;		//H
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin - size;
		i++;
		vertices[i] = origin + size;		//E
		i++;
		vertices[i] = origin + size;
		i++;
		vertices[i] = origin - size;
		i++;
		vertices[i] = origin + size;		//D
		i++;
		vertices[i] = origin + size;
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin + size;		//D
		i++;
		vertices[i] = origin + size;
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin + size;		//B
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin + size;		//H
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin - size;
		i++;
		vertices[i] = origin;		//G
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin - size;
		i++;
		vertices[i] = origin + size;		//H
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin - size;
		i++;
		vertices[i] = origin + size;		//B
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin + size;		//B
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin;		//A
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin;		//G
		i++;
		vertices[i] = origin;
		i++;
		vertices[i] = origin - size;

		glGenBuffers(1, (GLuint*) &(my_id));
		glBindBuffer(GL_ARRAY_BUFFER, my_id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 36 * 3, vertices, GL_STATIC_DRAW);

		VBufferInit = true;
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, my_id);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void ModuleRenderer3D::IndicesCube(float origin, float size)
{
	if (!IBufferInit)
	{
		int i = 0;

		vertices2[i] = origin;				//A
		i++;
		vertices2[i] = origin;
		i++;
		vertices2[i] = origin;
		i++;
		vertices2[i] = origin + size;		//B
		i++;
		vertices2[i] = origin;
		i++;
		vertices2[i] = origin;
		i++;
		vertices2[i] = origin;				//C
		i++;
		vertices2[i] = origin + size;
		i++;
		vertices2[i] = origin;
		i++;
		vertices2[i] = origin + size;		//D
		i++;
		vertices2[i] = origin + size;
		i++;
		vertices2[i] = origin;
		i++;
		vertices2[i] = origin + size;		//E
		i++;
		vertices2[i] = origin + size;
		i++;
		vertices2[i] = origin - size;
		i++;
		vertices2[i] = origin;				//F
		i++;
		vertices2[i] = origin + size;
		i++;
		vertices2[i] = origin - size;
		i++;
		vertices2[i] = origin;				//G
		i++;
		vertices2[i] = origin;
		i++;
		vertices2[i] = origin - size;
		i++;
		vertices2[i] = origin + size;		//H
		i++;
		vertices2[i] = origin;
		i++;
		vertices2[i] = origin - size;

		glGenBuffers(1, (GLuint*)&(my_indices));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, my_indices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * 36, &Cindices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		IBufferInit = true;
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, my_indices);
	glVertexPointer(3, GL_FLOAT, 0, &vertices2[0]);
	glDrawElements(GL_TRIANGLES, Cindices.size(), GL_UNSIGNED_INT, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);

}

void ModuleRenderer3D::Arrow(float ox, float oy, float oz, float ex, float ey, float ez)
{
	glLineWidth(3);
	glBegin(GL_LINES);
	glVertex3f(ox, oy, oz);
	glVertex3f(ex, ey, ez);
	glVertex3f(ex, ey, ez);
	glVertex3f(ex + 0.01, ey - 0.01 , ez);
	glVertex3f(ex, ey, ez);
	glVertex3f(ex - 0.01, ey - 0.01, ez);
	glEnd();
	glLineWidth(1);
}

void ModuleRenderer3D::Sphere(float radius, int stacks, int sectors, vector<float> origin)
{
	if (!SBufferInit)
	{
		float x;
		float y;
		float z;
		float xy;
		float an1; //for stacks
		float an2; //for sectors
		float currstack = pi / stacks;
		float currsector = 2 * pi / sectors;

		for (int i = 0; i <= stacks; ++i)
		{
			an1 = pi / 2 - i * currstack;
			xy = radius * cosf(an1);
			z = radius * sinf(an1);

			for (int c = 0; c <= sectors; ++c)
			{
				an2 = c * currsector;

				x = xy * cosf(an2);
				y = xy * sinf(an2);
				Svertices.push_back(x);
				Svertices.push_back(y);
				Svertices.push_back(z);
			}
		}

		int k1, k2;
		for (int i = 0; i < stacks; ++i)
		{
			k1 = i * (sectors + 1);
			k2 = k1 + sectors + 1;

			for (int c = 0; c < sectors; ++c, ++k1, ++k2)
			{
				if (i != 0)
				{
					Sindices.push_back(k1);
					Sindices.push_back(k2);
					Sindices.push_back(k1 + 1);
				}

				if (i != (stacks - 1))
				{
					Sindices.push_back(k1 + 1);
					Sindices.push_back(k2);
					Sindices.push_back(k2 + 1);
				}
			}
		}

		glGenBuffers(1, (GLuint*)&(my_Sid));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, my_Sid);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * Sindices.size(), &Sindices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		IBufferInit = true;
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, my_Sid);
	glVertexPointer(3, GL_FLOAT, 0, &Svertices[0]);
	glDrawElements(GL_TRIANGLES, Sindices.size(), GL_UNSIGNED_INT, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);
}