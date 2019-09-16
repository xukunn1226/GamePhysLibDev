#ifndef _GAMEOBJECTS_H_
#define _GAMEOBJECTS_H_

namespace GPL
{
	class GameObject
	{
		DECLARE_RTTI

	public:
		GameObject() :_UserData(NULL),_ComponentData(NULL){}

		virtual ~GameObject() {}

		virtual bool Init() = 0;

		virtual void Term() = 0;

		virtual void Tick(float DeltaTime) = 0;

		// 返回GameObject type，-1无效值
		virtual EGameObjectType GetObjectType();

		ComponentData* GetComponentData() const { return _ComponentData; }

		void*				_UserData;					// 强烈建议保存与物理对象绑定的对象指针

		ComponentData*		_ComponentData;				// 实例化数据
	};


	//对多个shape组成的component的封装类
	class ShipComponent: public GameObject
	{
		DECLARE_RTTI

	public:
		ShipComponent()
			: GameObject()
			, _Owner(NULL)
			, _AttachedId(0)
			, _Activated(true)
		{}

		virtual bool Init() { return true; }

		virtual bool InitShipComponent(ComponentData* CompData);

		virtual void AttachTo(PhysVehicle* Owner, Socket AttachedTo);

		virtual void Term();

		virtual void Tick(float DeltaTime);

		virtual void UpdatePose(float DeltaTime);

		virtual void SetRelativeLocation(NxVec3 RelativeLoc) {}

		virtual void SetRelativeRotation(NxQuat RelativeRot) {}

		virtual void SetGlobalLocation(NxVec3 GlobalLoc) {}

		virtual void SetGlobalRotation(NxQuat GlobalRot) {}

		virtual void SetAimTarget(NxVec3 TargetLoc) {}

		virtual int SetPreAimTarget(NxVec3 TargetLoc){ return 0; }

		NxMat34 GetGlobalPose();

		NxMat34 GetLocalPose();

		int GetAttachedID();
		
		Socket GetSocketAttachedTo() { return _Socket; }

		PhysVehicle* GetOwner(){return _Owner;}

		inline void	Active() { _Activated = true; }
		inline void	Deactive() { _Activated = false; }

	protected:
		PhysVehicle*					_Owner;
		int								_AttachedId;				// 组件ID
		std::vector<NxShape*>			_Shapes;
		std::vector<NxMat34>			_InitShapeLocalPose;		// 记录每个SHAPE相对Socket的初始LocalPose
		Socket							_Socket;					// 组件绑定于的SOCKET
		NxVec3							_InitialRelativeLocation;	// Socket在owner space的初始相对位置
		NxQuat							_InitialRelativeRotation;	// Socket在owner space的初始相对朝向
		NxVec3							_RelativeLocation;			// 同_InitialRelativeLocation，记录的是当前数值
		NxQuat							_RelativeRotation;			// 同_InitialRelativeRotation，记录的是当前数值
		bool							_Activated;					// 激活状态
	};
	
	class Turrent :public ShipComponent
	{
		DECLARE_RTTI
		#define MaxFirePartNum 10
	public:
		Turrent()
			: ShipComponent()
			, _Appendent(NULL)
		{}

		void GetFireParam( FiringParam& params );
		void GetRealFireParam(FiringParam& params );
		void UGetRealFireParam(FiringParam& params );
		NxVec3 GetDesireDir();
		NxVec3 GetDesireDir(float DesiredPitch);

		virtual bool InitShipComponent(ComponentData* ComponentData);

		virtual void Term();

		virtual void Tick(float DeltaTime);

		virtual void SetAimTarget(NxVec3 TargetLoc);

		virtual void UpdatePose(float DeltaTime);

		// 设置相对初始朝向(Socket._InitRotation)的角度值,逆时针为正值，顺时针为负值
		void SetDesiredYaw( float DesiredYaw );

		void SetDesiredPitch( float pitch );

#if 1
		// 当前炮管瞄准的朝向(_CurrentYaw)是否可以开火
		bool IsFireAvailabe();
#else
		// 当前炮管瞄准的朝向(_CurrentYaw)是否可以开火
		bool IsFireAvailabe();

		//	炮管与瞄准方向是否匹配
		bool IsAimingFit();
#endif

		// 炮管目标朝向(_DesiredYaw)是否可以开火
		bool IsDesireYawAvailabe();

		virtual int SetPreAimTarget(NxVec3 TargetLoc);
		void SetPreDesiredYaw(float DesiredYaw);
		void SetPreDesiredPitch(float pitch);
		NxVec3 GetPreDesireDir();
		bool IsPreDesireYawAvailabe();

		virtual void SetRelativeLocation(NxVec3 RelativeLoc) {}

		virtual void SetRelativeRotation(NxQuat RelativeRot) {}

		virtual void SetGlobalLocation(NxVec3 GlobalLoc) {}

		virtual void SetGlobalRotation(NxQuat GlobalRot) {}
	
		bool	IsAimingExact();
		float	GetFinalAccuracy(int GunTubeIndex);

		void	SetFired(int FirePartID);//开炮的时候需要调用
		float   GetCurAccuracy(int FirePartID);
		int		GetFirePartCount();
		float	CurrentPitch() const { return _CurrentPitch; }

		float	GetAccuracyOfLinearVelocity();		//	舰船移动对精准度的影响
		float	GetAccuracyOfAngularVelocity();		//	舰船转向对精准度的影响
		float	GetAccuracyOfTurrentTurn();			//	炮台转动对精准度的影响
		float	GetMaxAccuracy();					//  总精准度影响的最大值

		bool	IsInFirePressureRange(float InYaw);

		// 判断某空间坐标点是否在射界内
		// added by linzhen@icee.cn
		bool	IsTargetInFireLimitAngles(NxVec3 TargetLoc);
		bool	UIsTargetInFireLimitAngles(NxVec3 TargetLoc);

		// 获取从当前转角到目标转角的角度差
		// 若近似相等，则返回-1
		// 否则返回值>=0
		float	GetTotalYawDelta();

		inline void	OnBind(GameObject* pAppendent)	{ _Appendent = pAppendent; }
		inline GameObject* GetAppendentObject()		{ return _Appendent; }

	private:
		/**
		 * @brief	判断输入角度是否在开火限制角度内
		 * @param	[InYaw]		相对Socket._InitRotation的角度值，逆时针为正值，顺时针为负值
		 * @return	true，角度[InYaw]在限制开火的角度内；false，反之不在
		 */
		bool	IsInFireLimitAngles(float InYaw);

		 // 计算炮台转向
		 void TickRotation(float DeltaTime);

		// 计算所有炮管精准度
		void TickAccuracy(float DeltaTime);
		
		void TickAimTarget(float DeltaTime);

		float DoFastRotate(float step,float current,float desire);

		//************************************
		// Returns:   bool [true]val is in range, [false]val is out of range.
		// Qualifier: Calculate the available yaw according to val.
		// Parameter: float & val Desired yaw.
		// Parameter: float & ref Current yaw.
		// Parameter: float max Maximum yaw.
		// Parameter: float min Minimum yaw.
		// Parameter: bool fix Fix the result to minimize the angle between val and ref.
		//************************************
		bool GetAvailableYaw(float& val, float& ref, float max, float min, bool fix);

	protected:
		float	_CurrentYaw;					// 相对初始朝向(Socket._InitRotation)
		float	_CurrentPitch;
		float	_DesiredYaw;					// 相对初始朝向(Socket._InitRotation)
		float	_DesiredPitch;

		NxVec3	_TargetLoc;						// 瞄准目标点（世界坐标系）
		bool	_bValidTargetLoc;

		float	_PreDesiredYaw;
		float	_PreDesiredPitch;
		bool	_PreCanRotateTo;

		float	_TurnDegree;					//炮台持续转过的角度

		float	_FirePartsAccuracy[MaxFirePartNum];
		int		_FirePartCount;
		bool	_CanRotateTo;					// _DesiredYaw是否在旋转角度限制内

		GameObject*	_Appendent;
	};

	typedef std::vector<ShipComponent*>	ComponentList;
}

#endif