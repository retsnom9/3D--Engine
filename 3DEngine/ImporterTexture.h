#ifndef __IMPORTER_TEXTURE_H__
#define __IMPORTER_TEXTURE_H__

#include "Importer.h"
#include "Globals.h"

#define TEXT_DIR "Libraries\\Textures\\"
#define TEXT_EXTENSION ".dds"
#define CHECKERS_SIZE 20

struct ResTexture;

class ImporterTexture :
	public Importer
{
public:
	ImporterTexture();
	~ImporterTexture();

	bool Load();
	bool Save();
	bool Start();
	bool CleanUp();

	uint LoadChekerTex();
	ResTexture* LoadTex(const char* path, bool isfullpath = true);
	ResTexture ReloadTex(const char* path);
	void SaveTex(const char* path, bool isfullpath = true);
	void SaveTex(ResTexture tex);

};

#endif //__IMPORTER_TEXTURE_H__
