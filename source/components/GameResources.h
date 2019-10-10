#pragma once

class GameResources
{
public:
	static constexpr int ITEM_IMAGE_SIZE = 64;

	~GameResources();
	void Init();
	void LoadLanguage();
	void LoadData();
	void GenerateItemIconTask(TaskData& task_data);
	void GenerateItemIcon(Item& item);
	void DrawItemIcon(const Item& item, RenderTarget* target, float rot);

	TexturePtr tBlack, tPortal, tWarning, tError;
	TexturePtr tBlood[BLOOD_MAX], tBloodSplat[BLOOD_MAX], tSpark, tSpawn, tLightingLine;
	TexturePtr tGrass, tGrass2, tGrass3, tRoad, tFootpath, tField;
	TexturePack tFloor[2], tWall[2], tCeil[2], tFloorBase, tWallBase, tCeilBase;
	VertexDataPtr vdStairsUp, vdStairsDown, vdDoorHole;
	MeshPtr aBox, aCylinder, aSphere, aCapsule;
	MeshPtr aHuman, aHair[5], aBeard[5], aMustache[2], aEyebrows;
	MeshPtr aArrow, aSkybox, aBag, aChest, aGrating, aDoorWall, aDoorWall2, aStairsDown, aStairsDown2, aStairsUp, aSpellball, aPressurePlate, aDoor, aDoor2, aStun;

private:
	void CreateMissingTexture();

	typedef std::map<Mesh*, Texture*> ItemTextureMap;
	ItemTextureMap item_texture_map;
	vector<Texture*> over_item_textures;
	RenderTarget* rt_item;
	Texture missing_item_texture;
	cstring txLoadGuiTextures, txLoadParticles, txLoadPhysicMeshes, txLoadModels, txLoadSpells, txLoadSounds, txLoadMusic;
};
