#ifndef _PHYS_GAMEOBJECT_PLAYERSTART_H_
#define _PHYS_GAMEOBJECT_PLAYERSTART_H_

namespace GPL
{
	class PhysPlayerStart : public PhysStatic
	{
		DECLARE_RTTI
	public:
		PhysPlayerStart(PhysGameScene* GameScene, SceneStaticData* Data):PhysStatic(GameScene, Data)
		{
			_TeamIndex = 0;
			_SeatIndex = 0;
			_bOccupy = false;
			_PriorVehicleType.clear();
		}
		virtual bool Init();

		int _TeamIndex;   //放在这里免得再去读comp表了
		int _SeatIndex;
		bool _bOccupy;
		std::vector<int>  _PriorVehicleType;
	};
	
	class PhysPlayerRespawn : public PhysPlayerStart
	{
		DECLARE_RTTI
	public:
		PhysPlayerRespawn(PhysGameScene* GameScene, SceneStaticData* Data):PhysPlayerStart(GameScene, Data)
		{
		}
	};
}

#endif // PhysGameObjectPlayerStart_h__
