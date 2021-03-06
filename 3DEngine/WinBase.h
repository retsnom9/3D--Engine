#ifndef __WINBASE_H__
#define __WINBASE_H__

class Application;

class WinBase
{
public:
	bool enabled = true;
	Application * App;

	WinBase(Application* parent, bool start_enabled = true) : App(parent)
	{
		enabled = start_enabled;
	}

	~WinBase()
	{}


	void SetEnable(bool b) { enabled = b; };
	bool GetEnable() { return enabled; };

	virtual bool Start()
	{
		return true;
	}

	virtual bool Update()
	{
		return true;
	}
	virtual bool PostUpdate()
	{
		return true;
	}

	virtual bool CleanUp()
	{
		return true;
	}


};

#endif // !__WINBASE_H__
