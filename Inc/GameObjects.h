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

		// ����GameObject type��-1��Чֵ
		virtual EGameObjectType GetObjectType();

		ComponentData* GetComponentData() const { return _ComponentData; }

		void*				_UserData;					// ǿ�ҽ��鱣�����������󶨵Ķ���ָ��

		ComponentData*		_ComponentData;				// ʵ��������
	};


	//�Զ��shape��ɵ�component�ķ�װ��
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
		int								_AttachedId;				// ���ID
		std::vector<NxShape*>			_Shapes;
		std::vector<NxMat34>			_InitShapeLocalPose;		// ��¼ÿ��SHAPE���Socket�ĳ�ʼLocalPose
		Socket							_Socket;					// ������ڵ�SOCKET
		NxVec3							_InitialRelativeLocation;	// Socket��owner space�ĳ�ʼ���λ��
		NxQuat							_InitialRelativeRotation;	// Socket��owner space�ĳ�ʼ��Գ���
		NxVec3							_RelativeLocation;			// ͬ_InitialRelativeLocation����¼���ǵ�ǰ��ֵ
		NxQuat							_RelativeRotation;			// ͬ_InitialRelativeRotation����¼���ǵ�ǰ��ֵ
		bool							_Activated;					// ����״̬
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

		// ������Գ�ʼ����(Socket._InitRotation)�ĽǶ�ֵ,��ʱ��Ϊ��ֵ��˳ʱ��Ϊ��ֵ
		void SetDesiredYaw( float DesiredYaw );

		void SetDesiredPitch( float pitch );

#if 1
		// ��ǰ�ڹ���׼�ĳ���(_CurrentYaw)�Ƿ���Կ���
		bool IsFireAvailabe();
#else
		// ��ǰ�ڹ���׼�ĳ���(_CurrentYaw)�Ƿ���Կ���
		bool IsFireAvailabe();

		//	�ڹ�����׼�����Ƿ�ƥ��
		bool IsAimingFit();
#endif

		// �ڹ�Ŀ�곯��(_DesiredYaw)�Ƿ���Կ���
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

		void	SetFired(int FirePartID);//���ڵ�ʱ����Ҫ����
		float   GetCurAccuracy(int FirePartID);
		int		GetFirePartCount();
		float	CurrentPitch() const { return _CurrentPitch; }

		float	GetAccuracyOfLinearVelocity();		//	�����ƶ��Ծ�׼�ȵ�Ӱ��
		float	GetAccuracyOfAngularVelocity();		//	����ת��Ծ�׼�ȵ�Ӱ��
		float	GetAccuracyOfTurrentTurn();			//	��̨ת���Ծ�׼�ȵ�Ӱ��
		float	GetMaxAccuracy();					//  �ܾ�׼��Ӱ������ֵ

		bool	IsInFirePressureRange(float InYaw);

		// �ж�ĳ�ռ�������Ƿ��������
		// added by linzhen@icee.cn
		bool	IsTargetInFireLimitAngles(NxVec3 TargetLoc);
		bool	UIsTargetInFireLimitAngles(NxVec3 TargetLoc);

		// ��ȡ�ӵ�ǰת�ǵ�Ŀ��ת�ǵĽǶȲ�
		// ��������ȣ��򷵻�-1
		// ���򷵻�ֵ>=0
		float	GetTotalYawDelta();

		inline void	OnBind(GameObject* pAppendent)	{ _Appendent = pAppendent; }
		inline GameObject* GetAppendentObject()		{ return _Appendent; }

	private:
		/**
		 * @brief	�ж�����Ƕ��Ƿ��ڿ������ƽǶ���
		 * @param	[InYaw]		���Socket._InitRotation�ĽǶ�ֵ����ʱ��Ϊ��ֵ��˳ʱ��Ϊ��ֵ
		 * @return	true���Ƕ�[InYaw]�����ƿ���ĽǶ��ڣ�false����֮����
		 */
		bool	IsInFireLimitAngles(float InYaw);

		 // ������̨ת��
		 void TickRotation(float DeltaTime);

		// ���������ڹܾ�׼��
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
		float	_CurrentYaw;					// ��Գ�ʼ����(Socket._InitRotation)
		float	_CurrentPitch;
		float	_DesiredYaw;					// ��Գ�ʼ����(Socket._InitRotation)
		float	_DesiredPitch;

		NxVec3	_TargetLoc;						// ��׼Ŀ��㣨��������ϵ��
		bool	_bValidTargetLoc;

		float	_PreDesiredYaw;
		float	_PreDesiredPitch;
		bool	_PreCanRotateTo;

		float	_TurnDegree;					//��̨����ת���ĽǶ�

		float	_FirePartsAccuracy[MaxFirePartNum];
		int		_FirePartCount;
		bool	_CanRotateTo;					// _DesiredYaw�Ƿ�����ת�Ƕ�������

		GameObject*	_Appendent;
	};

	typedef std::vector<ShipComponent*>	ComponentList;
}

#endif