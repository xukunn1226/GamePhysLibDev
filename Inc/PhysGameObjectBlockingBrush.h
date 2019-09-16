#ifndef _PHYS_GAMEOBJECT_BLOCKINGBRUSH_H_
#define _PHYS_GAMEOBJECT_BLOCKINGBRUSH_H_
namespace GPL
{
	class PhysBlocking : public PhysStatic
	{
		DECLARE_RTTI
	public:
		PhysBlocking(PhysGameScene* GameScene, SceneBlockingData* Data):PhysStatic(GameScene, Data)
		{}
		virtual bool Init();
		virtual void Term();
		virtual void Tick(float DeltaTime);

	protected:
		virtual void InitPhys();
	};
}

#endif 
