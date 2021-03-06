#ifndef __MODULELOADER_H__
#define __MODULELOADER_H__

#include "Module.h"
#include "ImporterTexture.h"
#include "ImporterMesh.h"



class GameObject;

enum Format {
	FBX,
	PNG,
	DDS,
	JSON,
	MEH,
	FNULL
};

class ModuleLoader : public Module
{
public:


	ModuleLoader(Application* app, bool start_enabled = true);

	~ModuleLoader();

	bool Start();

	update_status PreUpdate(float dt);

	update_status Update(float dt);

	update_status PostUpdate(float dt);

	bool CleanUp();

	

	void SetDropFile(char* f);
	void LoadFBX(const char* path);


	Format CheckFormat(const char* path);
	std::string GetFileName(const char* path, bool extension = false);
	std::string GetFileDir(const char* path);

	ImporterTexture texImporter;
	ImporterMesh meshImporter;

private:

	char* droppedFile = nullptr;
	Format dFformat = FNULL;

};


#endif //__MODULELOADER_H__