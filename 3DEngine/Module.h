#ifndef __MODULE_H__
#define __MODULE_H__

#include "Globals.h"
#include "Parson/parson.h"
#include "JsonDoc.h"
#include <array>

class Application;

class Module
{
private :
	bool enabled;

protected:
	std::string name;
public:
	Application* App;
	void SetName(const char* _name) { name = *_name; };
	const char* GetName() const { return name.c_str(); };


	Module(Application* parent, bool start_enabled = true) : App(parent)
	{}

	virtual ~Module()
	{}

	virtual bool Init() 
	{
		return true; 
	}

	virtual bool Start()
	{
		return true;
	}

	virtual update_status PreUpdate(float dt)
	{
		return UPDATE_CONTINUE;
	}

	virtual update_status Update(float dt)
	{
		return UPDATE_CONTINUE;
	}

	virtual update_status PostUpdate(float dt)
	{
		return UPDATE_CONTINUE;
	}

	virtual bool CleanUp() 
	{ 
		return true; 
	}

	virtual bool Load(json_object_t* doc)
	{
		return true;
	}

	virtual bool Save(json_object_t* doc)
	{
		return true;
	}
};

#endif //__MODULE_H__