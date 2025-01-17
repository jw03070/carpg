#pragma once

//-----------------------------------------------------------------------------
#include "BaseUsable.h"
#include "ItemContainer.h"
#include "GameDialog.h"

//-----------------------------------------------------------------------------
struct Usable : EntityType<Usable>
{
	BaseUsable* base;
	Vec3 pos;
	float rot;
	Entity<Unit> user;
	ItemContainer* container;
	int variant;

	static const float SOUND_DIST;
	static const int MIN_SIZE = 22;

	Usable() : variant(-1), container(nullptr) {}
	~Usable() { delete container; }

	void Save(FileWriter& f, bool local);
	void Load(FileReader& f, bool local);
	void Write(BitStreamWriter& f) const;
	bool Read(BitStreamReader& f);
	Mesh* GetMesh() const;
};
