#pragma once

//-----------------------------------------------------------------------------
class GameComponent
{
public:
	virtual ~GameComponent() {}
	// Preconfigure game
	virtual void Prepare() {}
	// Called before loading content
	virtual void InitOnce() {}
	virtual void LoadLanguage() {}
	virtual void LoadData(ResourceManager& res_mgr) {}
	// Called after loading everything
	virtual void PostInit() {}
	virtual void Cleanup() {}
};
