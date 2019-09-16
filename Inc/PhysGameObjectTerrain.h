#ifndef _PHYS_GAMEOBJECT_TERRAIN_H_
#define _PHYS_GAMEOBJECT_TERRAIN_H_

namespace GPL
{
	class PhysTerrain : public PhysStatic
	{
		DECLARE_RTTI
	public:
		PhysTerrain(PhysGameScene* GameScene, SceneTerrainData* Data):PhysStatic(GameScene, Data)
		{ _TerrainActorList.clear(); }

		virtual bool Init();

		virtual EGameObjectType GetObjectType();

	protected:
		virtual void InitPhys();

		virtual void TermPhys();
		
		virtual void DoTermPhysActor(NxScene* Scene, NxActor* Actor);

		std::vector<NxActor*>	_TerrainActorList;
	};
}

#endif