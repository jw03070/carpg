#include "Pch.h"
#include "GameCore.h"
#include "GameResources.h"
#include "Item.h"
#include "Action.h"
#include "Building.h"
#include "SceneNode.h"
#include "Game.h"
#include <ResourceManager.h>
#include <Mesh.h>
#include <Render.h>
#include <DirectX.h>

GameResources* global::game_res;

//=================================================================================================
GameResources::~GameResources()
{
	for(auto& item : item_texture_map)
		delete item.second;
	for(Texture* tex : over_item_textures)
		delete tex;
}

//=================================================================================================
void GameResources::Init()
{
	aHuman = res_mgr->Load<Mesh>("human.qmsh");
	rt_item = render->CreateRenderTarget(Int2(ITEM_IMAGE_SIZE, ITEM_IMAGE_SIZE));
	CreateMissingTexture();
}

//=================================================================================================
void GameResources::CreateMissingTexture()
{
	TEX tex = render->CreateTexture(Int2(ITEM_IMAGE_SIZE, ITEM_IMAGE_SIZE));
	TextureLock lock(tex);
	const uint col[2] = { Color(255, 0, 255), Color(0, 255, 0) };
	for(int y = 0; y < ITEM_IMAGE_SIZE; ++y)
	{
		uint* pix = lock[y];
		for(int x = 0; x < ITEM_IMAGE_SIZE; ++x)
		{
			*pix = col[(x >= ITEM_IMAGE_SIZE / 2 ? 1 : 0) + (y >= ITEM_IMAGE_SIZE / 2 ? 1 : 0) % 2];
			++pix;
		}
	}

	missing_item_texture.tex = tex;
	missing_item_texture.state = ResourceState::Loaded;
}

//=================================================================================================
void GameResources::LoadLanguage()
{
	txLoadGuiTextures = Str("loadGuiTextures");
	txLoadParticles = Str("loadParticles");
	txLoadPhysicMeshes = Str("loadPhysicMeshes");
	txLoadModels = Str("loadModels");
	txLoadSpells = Str("loadSpells");
	txLoadSounds = Str("loadSounds");
	txLoadMusic = Str("loadMusic");
	//txGenerateWorld = Str("generateWorld");
	//txHaveErrors = Str("haveErrors");
	FIXME;
}

//=================================================================================================
void GameResources::LoadData()
{
	// gui textures
	res_mgr->AddTaskCategory(txLoadGuiTextures);
	tBlack = res_mgr->Load<Texture>("czern.bmp");
	tPortal = res_mgr->Load<Texture>("dark_portal.png");
	tWarning = res_mgr->Load<Texture>("warning.png");
	tError = res_mgr->Load<Texture>("error.png");
	Action::LoadData();

	// particles
	res_mgr->AddTaskCategory(txLoadParticles);
	tBlood[BLOOD_RED] = res_mgr->Load<Texture>("krew.png");
	tBlood[BLOOD_GREEN] = res_mgr->Load<Texture>("krew2.png");
	tBlood[BLOOD_BLACK] = res_mgr->Load<Texture>("krew3.png");
	tBlood[BLOOD_BONE] = res_mgr->Load<Texture>("iskra.png");
	tBlood[BLOOD_ROCK] = res_mgr->Load<Texture>("kamien.png");
	tBlood[BLOOD_IRON] = res_mgr->Load<Texture>("iskra.png");
	tBloodSplat[BLOOD_RED] = res_mgr->Load<Texture>("krew_slad.png");
	tBloodSplat[BLOOD_GREEN] = res_mgr->Load<Texture>("krew_slad2.png");
	tBloodSplat[BLOOD_BLACK] = res_mgr->Load<Texture>("krew_slad3.png");
	tBloodSplat[BLOOD_BONE] = nullptr;
	tBloodSplat[BLOOD_ROCK] = nullptr;
	tBloodSplat[BLOOD_IRON] = nullptr;
	tSpark = res_mgr->Load<Texture>("iskra.png");
	tSpawn = res_mgr->Load<Texture>("spawn_fog.png");
	tLightingLine = res_mgr->Load<Texture>("lighting_line.png");

	// preload terrain textures
	tGrass = res_mgr->Load<Texture>("trawa.jpg");
	tGrass2 = res_mgr->Load<Texture>("Grass0157_5_S.jpg");
	tGrass3 = res_mgr->Load<Texture>("LeavesDead0045_1_S.jpg");
	tRoad = res_mgr->Load<Texture>("droga.jpg");
	tFootpath = res_mgr->Load<Texture>("ziemia.jpg");
	tField = res_mgr->Load<Texture>("pole.jpg");
	tFloorBase.diffuse = res_mgr->Load<Texture>("droga.jpg");
	tFloorBase.normal = nullptr;
	tFloorBase.specular = nullptr;
	tWallBase.diffuse = res_mgr->Load<Texture>("sciana.jpg");
	tWallBase.normal = res_mgr->Load<Texture>("sciana_nrm.jpg");
	tWallBase.specular = res_mgr->Load<Texture>("sciana_spec.jpg");
	tCeilBase.diffuse = res_mgr->Load<Texture>("sufit.jpg");
	tCeilBase.normal = nullptr;
	tCeilBase.specular = nullptr;
	BaseLocation::PreloadTextures();

	// physic meshes
	res_mgr->AddTaskCategory(txLoadPhysicMeshes);
	vdStairsUp = res_mgr->Load<VertexData>("schody_gora.phy");
	vdStairsDown = res_mgr->Load<VertexData>("schody_dol.phy");
	vdDoorHole = res_mgr->Load<VertexData>("nadrzwi.phy");

	// meshes
	res_mgr->AddTaskCategory(txLoadModels);
	res_mgr->Load(aHuman);
	aBox = res_mgr->Load<Mesh>("box.qmsh");
	aCylinder = res_mgr->Load<Mesh>("cylinder.qmsh");
	aSphere = res_mgr->Load<Mesh>("sphere.qmsh");
	aCapsule = res_mgr->Load<Mesh>("capsule.qmsh");
	aHair[0] = res_mgr->Load<Mesh>("hair1.qmsh");
	aHair[1] = res_mgr->Load<Mesh>("hair2.qmsh");
	aHair[2] = res_mgr->Load<Mesh>("hair3.qmsh");
	aHair[3] = res_mgr->Load<Mesh>("hair4.qmsh");
	aHair[4] = res_mgr->Load<Mesh>("hair5.qmsh");
	aEyebrows = res_mgr->Load<Mesh>("eyebrows.qmsh");
	aMustache[0] = res_mgr->Load<Mesh>("mustache1.qmsh");
	aMustache[1] = res_mgr->Load<Mesh>("mustache2.qmsh");
	aBeard[0] = res_mgr->Load<Mesh>("beard1.qmsh");
	aBeard[1] = res_mgr->Load<Mesh>("beard2.qmsh");
	aBeard[2] = res_mgr->Load<Mesh>("beard3.qmsh");
	aBeard[3] = res_mgr->Load<Mesh>("beard4.qmsh");
	aBeard[4] = res_mgr->Load<Mesh>("beardm1.qmsh");
	aArrow = res_mgr->Load<Mesh>("strzala.qmsh");
	aSkybox = res_mgr->Load<Mesh>("skybox.qmsh");
	aBag = res_mgr->Load<Mesh>("worek.qmsh");
	aChest = res_mgr->Load<Mesh>("skrzynia.qmsh");
	aGrating = res_mgr->Load<Mesh>("kratka.qmsh");
	aDoorWall = res_mgr->Load<Mesh>("nadrzwi.qmsh");
	aDoorWall2 = res_mgr->Load<Mesh>("nadrzwi2.qmsh");
	aStairsDown = res_mgr->Load<Mesh>("schody_dol.qmsh");
	aStairsDown2 = res_mgr->Load<Mesh>("schody_dol2.qmsh");
	aStairsUp = res_mgr->Load<Mesh>("schody_gora.qmsh");
	aSpellball = res_mgr->Load<Mesh>("spellball.qmsh");
	aDoor = res_mgr->Load<Mesh>("drzwi.qmsh");
	aDoor2 = res_mgr->Load<Mesh>("drzwi2.qmsh");
	aStun = res_mgr->Load<Mesh>("stunned.qmsh");

	// preload buildings
	for(Building* b : Building::buildings)
	{
		if(b->mesh)
			res_mgr->LoadMeshMetadata(b->mesh);
		if(b->inside_mesh)
			res_mgr->LoadMeshMetadata(b->inside_mesh);
	}
}

//=================================================================================================
void GameResources::GenerateItemIconTask(TaskData& task_data)
{
	Item& item = *(Item*)task_data.ptr;
	GenerateItemIcon(item);
}

//=================================================================================================
void GameResources::GenerateItemIcon(Item& item)
{
	item.state = ResourceState::Loaded;

	// use missing texture if no mesh/texture
	if(!item.mesh && !item.tex)
	{
		item.icon = &missing_item_texture;
		item.flags &= ~ITEM_GROUND_MESH;
		return;
	}

	// if item use image, set it as icon
	if(item.tex)
	{
		item.icon = item.tex;
		return;
	}

	// try to find icon using same mesh
	bool use_tex_override = false;
	if(item.type == IT_ARMOR)
		use_tex_override = !item.ToArmor().tex_override.empty();
	ItemTextureMap::iterator it;
	if(!use_tex_override)
	{
		it = item_texture_map.lower_bound(item.mesh);
		if(it != item_texture_map.end() && !(item_texture_map.key_comp()(item.mesh, it->first)))
		{
			item.icon = it->second;
			return;
		}
	}
	else
		it = item_texture_map.end();

	Texture* tex;
	do
	{
		DrawItemIcon(item, rt_item, 0.f);
		tex = render->CopyToTexture(rt_item);
	}
	while(tex == nullptr);

	item.icon = tex;
	if(it != item_texture_map.end())
		item_texture_map.insert(it, ItemTextureMap::value_type(item.mesh, tex));
	else
		over_item_textures.push_back(tex);
}

//=================================================================================================
void GameResources::DrawItemIcon(const Item& item, RenderTarget* target, float rot)
{
	IDirect3DDevice9* device = render->GetDevice();

	if(IsSet(ITEM_ALPHA, item.flags))
	{
		render->SetAlphaBlend(true);
		render->SetNoZWrite(true);
	}
	else
	{
		render->SetAlphaBlend(false);
		render->SetNoZWrite(false);
	}
	render->SetAlphaTest(false);
	render->SetNoCulling(false);
	render->SetTarget(target);

	V(device->Clear(0, nullptr, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, 0, 1.f, 0));
	V(device->BeginScene());

	Mesh& mesh = *item.mesh;
	const TexOverride* tex_override = nullptr;
	if(item.type == IT_ARMOR)
	{
		if(const Armor& armor = item.ToArmor(); !armor.tex_override.empty())
		{
			tex_override = armor.GetTextureOverride();
			assert(armor.tex_override.size() == mesh.head.n_subs);
		}
	}

	Matrix matWorld;
	Mesh::Point* point = mesh.FindPoint("cam_rot");
	if(point)
		matWorld = Matrix::CreateFromAxisAngle(point->rot, rot);
	else
		matWorld = Matrix::RotationY(rot);
	Matrix matView = Matrix::CreateLookAt(mesh.head.cam_pos, mesh.head.cam_target, mesh.head.cam_up),
		matProj = Matrix::CreatePerspectiveFieldOfView(PI / 4, 1.f, 0.1f, 25.f);

	LightData ld;
	ld.pos = mesh.head.cam_pos;
	ld.color = Vec3(1, 1, 1);
	ld.range = 10.f;

	ID3DXEffect* effect = game->eMesh;
	V(effect->SetTechnique(game->techMesh));
	V(effect->SetMatrix(game->hMeshCombined, (D3DXMATRIX*)&(matWorld * matView * matProj)));
	V(effect->SetMatrix(game->hMeshWorld, (D3DXMATRIX*)&matWorld));
	V(effect->SetVector(game->hMeshFogColor, (D3DXVECTOR4*)&Vec4(1, 1, 1, 1)));
	V(effect->SetVector(game->hMeshFogParam, (D3DXVECTOR4*)&Vec4(25.f, 50.f, 25.f, 0)));
	V(effect->SetVector(game->hMeshAmbientColor, (D3DXVECTOR4*)&Vec4(0.5f, 0.5f, 0.5f, 1)));
	V(effect->SetVector(game->hMeshTint, (D3DXVECTOR4*)&Vec4(1, 1, 1, 1)));
	V(effect->SetRawValue(game->hMeshLights, &ld, 0, sizeof(LightData)));

	V(device->SetVertexDeclaration(render->GetVertexDeclaration(mesh.vertex_decl)));
	V(device->SetStreamSource(0, mesh.vb, 0, mesh.vertex_size));
	V(device->SetIndices(mesh.ib));

	uint passes;
	V(effect->Begin(&passes, 0));
	V(effect->BeginPass(0));

	for(int i = 0; i < mesh.head.n_subs; ++i)
	{
		const Mesh::Submesh& sub = mesh.subs[i];
		V(effect->SetTexture(game->hMeshTex, mesh.GetTexture(i, tex_override)));
		V(effect->CommitChanges());
		V(device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, sub.min_ind, sub.n_ind, sub.first * 3, sub.tris));
	}

	V(effect->EndPass());
	V(effect->End());
	V(device->EndScene());

	render->SetTarget(nullptr);
}
