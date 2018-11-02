#pragma once
#include "Importer.h"
#include "Globals.h"

#define DIR "Libraries\\Textures\\"
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
	ResTexture LoadTex(const char* path);
	void SaveTex(ResTexture tex);
};

