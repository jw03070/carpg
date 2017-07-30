#pragma once

//-----------------------------------------------------------------------------
struct Light
{
	Vec3 pos, color;
	float range;
	// temporary
	Vec3 t_pos, t_color;

	static const int MIN_SIZE = 28;

	void Save(FileWriter& f) const;
	void Load(FileReader& f);
	void Write(BitStream& stream) const;
	bool Read(BitStream& stream);
};