#pragma once

//-----------------------------------------------------------------------------
enum COLLISION_GROUP
{
	CG_TERRAIN = 1 << 7,
	CG_BUILDING = 1 << 8,
	CG_UNIT = 1 << 9,
	CG_OBJECT = 1 << 10,
	CG_DOOR = 1 << 11,
	CG_COLLIDER = 1 << 12,
	CG_CAMERA_COLLIDER = 1 << 13,
	CG_BARRIER = 1 << 14, // blocks only units
	CG_UNIT_DEAD = 1 << 15
	// 1<<15 is max!
};

//-----------------------------------------------------------------------------
typedef bool (Level::*CollisionCheck)(const CollisionObject& co, const Vec3& pos, float radius) const;
typedef bool (Level::*CollisionCheckRect)(const CollisionObject& co, const Box2d& box) const;

//-----------------------------------------------------------------------------
struct CollisionObject
{
	enum Type : short
	{
		RECTANGLE,
		SPHERE,
		CUSTOM,
		RECTANGLE_ROT
	};

	Vec2 pt;
	union
	{
		struct
		{
			float w, h, rot, radius;
		};
		struct
		{
			CollisionCheck check;
			CollisionCheckRect check_rect;
			int extra;
		};
	};
	void* owner; // pointer to Object/Chest/Usable
	Type type;
	bool cam_collider;
};

//-----------------------------------------------------------------------------
struct CameraCollider
{
	Box box;
};

//-----------------------------------------------------------------------------
enum LINE_TEST_RESULT
{
	LT_IGNORE,
	LT_COLLIDE,
	LT_END
};
