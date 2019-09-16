#ifndef _PHYS_GAMEOBJECT_Ocean_H_
#define _PHYS_GAMEOBJECT_Ocean_H_

namespace GPL
{

	class PhysOcean : public PhysStatic
	{
		DECLARE_RTTI
	public:
		PhysOcean(PhysGameScene* GameScene, SceneOceanData* Data)
			: PhysStatic(GameScene, Data)
		{}		
		
	protected:
		virtual void InitPhys();
	};
}
#endif