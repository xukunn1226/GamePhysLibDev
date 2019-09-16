#ifndef _PHYS_GAMEOBJECT_TRIGGERBRUSH_H_
#define _PHYS_GAMEOBJECT_TRIGGERBRUSH_H_

namespace GPL
{
	class PhysTrigger : public PhysStatic
	{
		DECLARE_RTTI
	public:
		PhysTrigger(PhysGameScene* GameScene, SceneStaticData* Data):PhysStatic(GameScene, Data)
		{}
		virtual bool Init();
		virtual void Term();
		virtual void Tick(float DeltaTime);

		int _PhyAreaType;
		int _AreaId;

	protected:
		virtual void InitPhys();
		virtual void PostInitPhys();
		virtual void PrevTermPhys();
	};

	//	@brief	���򴥷���
	//	@note	���ý������ж������ڽ��ػ�����/׷����Χ��⣬�Լ�������������
	class RangeTrigger : public PhysTrigger
	{
		DECLARE_RTTI

	public:
		RangeTrigger(PhysGameScene* GameScene
			, float Radius
			, float Extends
			, bool CheckVehicle = false
			, bool CheckAircraft = true) 
			: PhysTrigger(GameScene, NULL)
			, _Radius(Radius)
			, _Extends(Extends)
			, _Activated(false)
			, _CheckVehicle(CheckVehicle)
			, _CheckAircraft(CheckAircraft)
		{
		}
		virtual ~RangeTrigger()	{}

		virtual bool	Init();
		virtual void	Term();
		virtual void	Tick(float DeltaTime);

		void			Activate();
		void			Deactive();
		inline bool		IsActivated()	{ return _Activated; }

	protected:
		virtual void	InitPhys();
		virtual void	PostInitPhys();
		virtual void	PrevTermPhys();

	protected:
		float			_Radius;
		float			_Extends;
		bool			_Activated;
		bool			_CheckVehicle;
		bool			_CheckAircraft;
	};

	//	@brief	����������
	//	@note	����Turrent�����򴥷���
	class AssociatedTrigger : public RangeTrigger
	{
		DECLARE_RTTI

	public:
		AssociatedTrigger(PhysGameScene* GameScene, float Radius, float Extends, bool CheckVehicle = false, bool CheckAircraft = true)
			: RangeTrigger(GameScene, Radius, Extends, CheckVehicle, CheckAircraft)
			, _Host(NULL)
		{
		}
		virtual ~AssociatedTrigger()	{}

		virtual bool	Init();
		virtual void	Term();
		virtual void	Tick(float DeltaTime);

		void			Bind(Turrent* Host);

	protected:
		virtual void	InitPhys();
		virtual void	PostInitPhys();
		virtual void	PrevTermPhys();

	protected:
		Turrent*		_Host;
	};
}

#endif
