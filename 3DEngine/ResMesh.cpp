#include "ResMesh.h"
#include "MathGeoLib/MathGeoLib.h"
#include "Application.h"

#include "Glew/include/glew.h"
#include "SDL\include\SDL_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>

#include "mmgr/mmgr.h"

ResMesh::ResMesh(uint32_t UUID): Resource(UUID, RESMESH)
{
}


ResMesh::~ResMesh()
{
}


void ResMesh::GenerateBuffer()
{
	
	glGenBuffers(1, (GLuint*) &(id_indice));
	glBindBuffer(GL_ARRAY_BUFFER, id_indice);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uint) * num_indice, indice, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void ResMesh::Draw()
{

	//float* transC = new float[num_textC];
	//memcpy(transC, textC, sizeof(float) * num_textC);


	//for (uint i = 0; i < num_textC; i += 2)
	//{
	//	transC[i] += tex.position.x;
	//	transC[i + 1] += tex.position.y;

	//	transC[i] /= tex.scale.x;
	//	transC[i + 1] /= tex.scale.y;

	//	float2 vec = { transC[i], transC[i + 1] };
	//	float angle = tex.angle * (pi / 180);

	//
	//	float2 result = vec;
	//	result.x = (vec.x *  cos(angle)) + (vec.y * sin(angle));
	//	result.y = (vec.x * -sin(angle)) + (vec.y * cos(angle));

	//	transC[i] = result.x;
	//	transC[i + 1] = result.y;
	//}

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);



	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_indice);
	glVertexPointer(3, GL_FLOAT, 0, &vertex[0]);

	glTexCoordPointer(2, GL_FLOAT, 0, &textC[0]);
	glDrawElements(GL_TRIANGLES, num_indice, GL_UNSIGNED_INT, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	//delete[] transC;
	//transC = nullptr;
}



void ResMesh::DrawNormals()
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


void ResMesh::CleanUp()
{

	glDeleteBuffers(1, (GLuint*) &(id_indice));

	delete[] vertex;
	vertex = nullptr;

	delete[] indice;
	indice = nullptr;

	delete[] normals;
	normals = nullptr;

	delete[] colors;
	colors = nullptr;

	delete[] textC;
	textC = nullptr;

}

void ResMesh::Init()
{

	//Reloading mesh and copying the data
	ResMesh m = App->loader->meshImporter.ReloadMesh(exportedFile.c_str());
	
	id_indice = m.id_indice; 
	num_indice = m.num_indice;
	indice = m.indice;

	id_vertex = m.id_vertex;
	num_vertex = m.num_vertex;
	vertex = m.vertex;

	num_normals = m.num_normals;
	normals = m.normals;

	id_colors = m.id_colors;
	num_colors = m.num_colors;
	colors = m.colors;

	id_textC = m.id_textC;
	num_textC = m.num_textC;
	textC = m.textC;

}
