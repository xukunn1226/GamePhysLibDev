#ifndef _PHYS_GAMEOBJECT_STATIC_H_
#define _PHYS_GAMEOBJECT_STATIC_H_

namespace GPL
{
	class PhysStatic : public PhysGameObject
	{
		DECLARE_RTTI
	public:
		PhysStatic(PhysGameScene* GameScene, SceneStaticData* Data):PhysGameObject(GameScene, Data)
		{}
		virtual bool Init();
		virtual void Term();
		virtual void Tick(float DeltaTime);
		
		virtual NxMat34	GetGlobalPos() const;
		virtual NxMat34	UGetGlobalPos() const;
		virtual NxMat33	GetGlobalOritation() const;
		virtual NxQuat	GetGlobalOritationQuat() const;
		virtual NxVec3	GetGlobalPosition() const;
		virtual NxVec3	UGetGlobalPosition() const;

	protected:
		virtual void InitPhys();
		virtual void PostInitPhys();
		virtual void PrevTermPhys();
	};


}

#endif	