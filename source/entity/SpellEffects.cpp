// efekty czarów, eksplozje, electro, drain
#include "Pch.h"
#include "GameCore.h"
#include "SpellEffects.h"
#include "Unit.h"
#include "Spell.h"
#include "ParticleSystem.h"
#include "ResourceManager.h"
#include "SaveState.h"
#include "BitStreamFunc.h"
#include "GameResources.h"
#include "LevelArea.h"

EntityType<Electro>::Impl EntityType<Electro>::impl;

//=================================================================================================
void Explo::Save(FileWriter& f)
{
	f << pos;
	f << size;
	f << sizemax;
	f << dmg;
	f << hitted;
	f << owner;
	f << tex->filename;
}

//=================================================================================================
void Explo::Load(FileReader& f)
{
	f >> pos;
	f >> size;
	f >> sizemax;
	f >> dmg;
	f >> hitted;
	f >> owner;
	tex = res_mgr->Load<Texture>(f.ReadString1());
}

//=================================================================================================
void Explo::Write(BitStreamWriter& f)
{
	f << tex->filename;
	f << pos;
	f << size;
	f << sizemax;
}

//=================================================================================================
bool Explo::Read(BitStreamReader& f)
{
	const string& tex_id = f.ReadString1();
	f >> pos;
	f >> size;
	f >> sizemax;
	if(!f)
		return false;
	tex = res_mgr->Load<Texture>(tex_id);
	return true;
}

//=================================================================================================
void Electro::Update(LevelArea& area, float dt)
{
	for(Line& line : lines)
	{
		line.t += dt;
		if(!line.trail)
		{
			if(line.t >= 0.5f)
			{
				line.trail = new TrailParticleEmitter;
				line.trail->width = 0.1f;
				line.trail->tex = game_res->tLightingLine;

				static vector<Vec3> pts;
				pts.clear();
				pts.push_back(line.from);
				line.trail->first = 0;

				int steps = int(Vec3::Distance(line.from, line.to) * 10);
				Vec3 dir = line.to - line.from;
				const Vec3 step = dir / float(steps);
				Vec3 prev_off = Vec3::Zero;

				for(int i = 1; i < steps; ++i)
				{
					Vec3 p = line.from + step * (float(i) + Random(-0.25f, 0.25f));
					Vec3 r = Vec3::Random(Vec3(-0.3f, -0.3f, -0.3f), Vec3(0.3f, 0.3f, 0.3f));
					prev_off = (r + prev_off) / 2;
					pts.push_back(p + prev_off);
				}

				pts.push_back(line.to);

				line.trail->InitManual(pts, Vec4(0.2f, 0.2f, 1.f, 1.f));
				UpdateLineColor(*line.trail);
			}
		}
		else
		{
			if(line.t >= 1.f)
			{
				DeleteElement(area.tmp->tpes, line.trail);
				line.trail = nullptr;
			}
			else
				UpdateLineColor(*line.trail);
		}
	}
}

//=================================================================================================
void Electro::AddLine(const Vec3& from, const Vec3& to)
{
	Line& line = Add1(lines);
	line.t = 0.f;
	line.from = from;
	line.to = to;
	line.trail = nullptr;
}

//=================================================================================================
void Electro::UpdateLineColor(TrailParticleEmitter& tp)
{
	const int count = (int)tp.parts.size();
	for(int i = 0; i < count; ++i)
	{
		TrailParticle& p = tp.parts[i];
		float a = float(count - min(count, (int)abs(i - count * (tp.timer / 0.25f)))) / count;
		p.color.w = min(0.5f, a * a);
	}
}

//=================================================================================================
void Electro::Save(FileWriter& f)
{
	f << id;
	f << lines.size();
	for(Line& line : lines)
	{
		f << (line.trail ? line.trail->id : -1);
		f << line.from;
		f << line.to;
		f << line.t;
	}
	f << hitted.size();
	for(Entity<Unit> unit : hitted)
		f << unit;
	f << dmg;
	f << owner;
	f << spell->id;
	f << valid;
	f << hitsome;
	f << start_pos;
}

//=================================================================================================
void Electro::Load(FileReader& f, LevelArea& area)
{
	if(LOAD_VERSION >= V_0_12)
		f >> id;
	Register();
	lines.resize(f.Read<uint>());
	if(LOAD_VERSION >= V_DEV)
	{
		for(Line& line : lines)
		{
			line.trail = TrailParticleEmitter::GetById(f.Read<int>());
			f >> line.from;
			f >> line.to;
			f >> line.t;
		}
	}
	else
	{
		for(Line& line : lines)
		{
			static vector<Vec3> pts;
			pts.clear();
			f.ReadVector4(pts);
			f >> line.t;
			line.trail = nullptr;
			line.from = pts.front();
			line.to = pts.back();
		}
		Update(area, 0);
	}
	hitted.resize(f.Read<uint>());
	for(Entity<Unit>& unit : hitted)
		f >> unit;
	f >> dmg;
	f >> owner;
	const string& spell_id = f.ReadString1();
	spell = Spell::TryGet(spell_id);
	if(!spell)
		throw Format("Missing spell '%s' for electro.", spell_id.c_str());
	f >> valid;
	f >> hitsome;
	f >> start_pos;
}

//=================================================================================================
void Electro::Write(BitStreamWriter& f)
{
	f << id;
	f << spell->id;
	f.WriteCasted<byte>(lines.size());
	for(Line& line : lines)
	{
		f << line.from;
		f << line.to;
		f << line.t;
	}
}

//=================================================================================================
bool Electro::Read(BitStreamReader& f)
{
	f >> id;
	spell = Spell::TryGet(f.ReadString1());

	byte count;
	f >> count;
	if(!f.Ensure(count * LINE_MIN_SIZE))
		return false;
	lines.reserve(count);
	Vec3 from, to;
	float t;
	for(byte i = 0; i < count; ++i)
	{
		f >> from;
		f >> to;
		f >> t;
		AddLine(from, to);
		lines.back().t = t;
	}

	valid = true;
	Register();
	return true;
}

//=================================================================================================
void Drain::Save(FileWriter& f)
{
	f << from->id;
	f << to->id;
	f << pe->id;
	f << t;
}

//=================================================================================================
void Drain::Load(FileReader& f)
{
	from = Unit::GetById(f.Read<int>());
	to = Unit::GetById(f.Read<int>());
	pe = ParticleEmitter::GetById(f.Read<int>());
	f >> t;
}
