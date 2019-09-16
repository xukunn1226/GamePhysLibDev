//------------------------------------------------------------------------
//	@brief	���ػ��������
//	@author	chenpu
//	@date	2013-1-24
//------------------------------------------------------------------------
#include "AircraftFSM.h"

#pragma once

namespace GPL {

//	@brief	�������ػ�����
#define	SQUAD_AIRCRAFT_MAX	(5)

//	@brief	Ĭ���ٶ�
#define DEFAULT_SPEED		(10.f)

//	@brief	Ĭ��ת��뾶
#define DEFAULT_RADIUS		(10.f)

//	@brief	�ٶȱ�������
#define SPEED_FACTOR_UPPER_LIMIT	(2.f)

//	@brief	�ٶȱ�������
#define SPEED_FACTOR_LOWER_LIMIT	(0.005f)

//	@brief	���Է��ʽӿ�
#define DECLARE_PROPERTY_ACCESSOR(Type, Prop)	\
	inline	void	Set##Prop(Type _value);	\
	inline	Type	Get##Prop();

	//	@brief	���ػ�ָ��
	enum OperationCommand
	{
		_TakeOff = 0,			//	���
		_Cruise,				//	Ѳ��
		_Convoy,				//	����
		_Attack,				//	����
		_Recovery,				//	�غ�
		_LayerChange,			//	�������и߶�
	};

	//	@brief	���в��
	enum FlightLayer
	{
		ExtremeLow = 0,	//	�������أ�
		Lower,			//	�Ϳ�
		Higher,			//	�߿�
		LayerNum
	};

	//	@brief	���ػ����
	enum AircraftFlag
	{
		LockHeader = 1,		//	��������
		VerticalMove,		//	����λ��
		Ascend,				//	����
		Descend,			//	�½�
		DiveAttack,			//	�����ը
		ExigencePull,		//	��������
		Falling,			//	׹��
		Collimate,			//	У׼
	};

	class PhysAircraft : public PhysGameObject
	{
		DECLARE_RTTI

		typedef std::vector<PhysAircraft*> AircraftList;

	public:
		PhysAircraft(PhysGameScene* scene, AircraftData* data)
			: PhysGameObject(scene, data)
			, _Header(NULL)
			, _SpeedFactor(1.f)
			, _SquadID(0xFF)
			, _SquadIndex(0xFF)
			, _LocalPlayer(false)
			, _MovePost(true)
			, _InterceptTrigger(NULL)
			, _ChaseTrigger(NULL)
			, _ExigencePullAcceleration(0)
			, _VerticalSpeed(0)
			, _DetectInterval(0)
			, _AccumTime(0)
			, _PendingLayer(Higher)
		{}
		virtual ~PhysAircraft();

		virtual bool	Init();

		virtual void	Tick(float DeltaTime);

		//	@brief	���ͳ�ʼ״̬
		//	@note	Initialize only
		bool					PushInitState(AircraftAction action, const NxVec3& pos, const NxVec3& dir, PhysGameObject* obj);

		//	@brief	��ý��ػ�
		static PhysAircraft*	GetAircraft(PhysVehicle* pOwner, UINT squad, UINT index);

		//	@brief	����ָ��
		//	@param	[cmd]			ִ��ָ��
		//	@param	[tar]			Ŀ���
		//	@param	[dir]			����
		//	@param	[pObj]			Ŀ�����
		//	@return	[bool]			�����ܷ�ִ��
		bool					OnCommand(OperationCommand cmd, const NxVec3& tar, const NxVec3& dir, PhysGameObject* pObj = NULL);

		//	@brief	����ָ�Ureal��λ������
		bool					UOnCommand(OperationCommand cmd, const NxVec3& tar, const NxVec3& dir, PhysGameObject* pObj = NULL);

		//	@brief	׷��һ��ָ��
		//	@param	[cmd]			ִ��ָ��
		//	@param	[tar]			Ŀ���
		//	@param	[dir]			����
		//	@param	[pObj]			Ŀ�����
		//	@return	[bool]			�Ƿ��滻׷��
		bool					PendingCommand(OperationCommand cmd, const NxVec3& tar, const NxVec3& dir, PhysGameObject* pObj = NULL);

		//	@brief	׷��һ��ָ�Ureal��λ������
		bool					UPendingCommand(OperationCommand cmd, const NxVec3& tar, const NxVec3& dir, PhysGameObject* pObj = NULL);

		//	@brief	�������
		//	@param	[Act]			����ID
		//	@param	[tar]			Ŀ���
		//	@param	[dir]			����
		void					ChangeAction(AircraftAction Act, const NxVec3& tar, const NxVec3& dir);

		//	@brief	���������Ureal��λ������
		void					UChangeAction(AircraftAction Act, const NxVec3& tar, const NxVec3& dir);
		
		//	@brief	ǿ���ƶ�
		//	@param	[Pos]	λ��
		//	@param	[Dir]	����
		void					ForceTransform(const NxVec3& Pos, const NxVec3& Dir);

		//	@brief	λ��ģ��
		//	@param	[Time]	����ʱ��
		//	@param	[Pos]	ģ��λ��
		//	@param	[Dir]	ģ�⳯��
		NxMat34					TransformSimulate(float Time, bool ignoreDir = true);

		//	@brief	׹���¼���Ӧ
		//	@param	[bShoot]	�Ƿ񱻻���
		void					OnCrash(bool bShoot = true);

		//	@brief	������ɵ���
		void					OnActionFinished();

		//	@brief	��ָ���
		void					NonCommandAction();

		//	@brief	����
		float					Dive(float DeltaTime);

		//	@brief	��������
		float					Pull(float DeltaTime);

		//	@brief	����
		float					Incline(float DeltaTime);

		//	@brief	�½�
		float					Decline(float DeltaTime);

		//	@brief	׹��
		float					FallingDown(float DeltaTime);

		//	@brief	����װ��¼�
		void					OnLeaveBoard();

		//	@brief	Ŀ�궪ʧ�¼�
		virtual void			OnTargetLost(PhysGameObject* Target);

		//	@brief	��ñ�����λ��
		//	@note	������ǰ����
		NxVec3					GetFormationOffset();

		//	@brief	��ñ�����λ��
		//	@param	[dir]	ָ������
		NxVec3					GetFormationOffset(const NxVec3& dir);

		//	@brief	��ñ��λ��ȫ������
		NxVec3					GetFormationGlobalPosition();

		//	@brief	Ӧ�ñ��λ��
		//	@note	ֱ�ӱ��ƫ����Ϊ�Լ���λ�ã����е����⣬�ȱ��ã�
		void					ApplyFormationTrans();

		//	@brief	���ñ�־λ
		void					SetFlag(AircraftFlag flag, bool enable);

		//	@brief	����־�Ƿ���
		inline bool				CheckFlag(AircraftFlag flag)	{ return CHECK_STATE(_Flags, flag); }

		//	@brief	��ý��ػ�����
		EPartType				GetAircraftType();

		//	@brief	��÷��в��
		FlightLayer				GetFlightLayer();

		//	@brief	��õ�ǰ״̬
		inline GameObjectState*	GetCurrentState()				{ return _FSM.GetActiveState(); }

		//	@brief	��õ�ǰ״̬ö��ֵ
		inline AircraftAction	GetCurrentStateEnum()				
		{
			GameObjectState* State = _FSM.GetActiveState();
			return (NULL != State) ? AircraftAction(State->GetEnumValue()) : ActionNum;
		}

		//	@brief	��ȡ��ͼ�����һ����ִ�ж�����
		inline AircraftAction	GetIntent()						{ return _FSM.GetLastAction(); }

		//	@brief	ĸ��
		void					SetBaseShip(PhysGameObject* pOwner);
		PhysVehicle*			GetBaseShip();
		NxVec3					GetStartLocation();
		NxVec3					GetLeaveLocation();
		NxVec3					GetCatchLocation();

		//	@brief	���ID
		inline void				SetSquadID(UINT id)				{ _SquadID = id; }
		inline UINT				GetSquadID()					{ return _SquadID; }

		//	@brief	���ڱ��
		void					SetSquadIndex(int index);
		inline int				GetSquadIndex()					{ return _SquadIndex; }

		//	@brief	����
		void					SetHeader(PhysAircraft* header);
		inline PhysAircraft*	GetHeader()						{ return _Header; }

		//	@brief	�Ż�
		void					RegisterWing(PhysAircraft* wing);
		void					UnregisterWing(PhysAircraft* wing, bool resort = true);
		inline void				ClearWings()					{ _Wings.clear(); }
		UINT					GetWingNum()					{ return (UINT)_Wings.size(); }
		PhysAircraft*			GetWing(UINT idx);

		//	@brief	�Ƿ�����
		bool			IsAlive();

		//	@brief	��������
		NxVec3			GetConvoluteCenter();

		//	@brief	��������
		NxVec3			GetGuardCenter();

		//	@brief	��С���а뾶
		float			GetMinRadius();
		
		//	@brief	�����뾶
		float			GetConvoluteRadius();

		//	@brief	ת��뾶
		float			GetRadius(WiseType wise);

		//	@brief	�ٶ�����
		void			SetSpeedFactor(float fFactor);

		//	@brief	�ٶȱ���
		float			GetSpeed();

		//	@brief	�ٶ�ʸ��
		NxVec3			GetVelocity();
		NxVec3			UGetVelocity()	{ return GetVelocity() * P2GScale; }

		//	@brief	����
		void			SetForward(const NxVec3& forward);
		NxVec3			GetForward();

		//	@brief	Ͷ������
		inline void		InitPrecisionFactor()	{ _PrecisionFactor = 10000.f; }
		void			ModifyPrecisionFactor(float Delta);
		float			GetFinalPrecision();
		void			OnBombTargetChange(const NxVec3& tar);

		//	@brief	��������
		ETrajectoryMethod	GetTrajectoryMethod();

		//	@brief	Ѳ���ٶ�
		DECLARE_PROPERTY_ACCESSOR(float, CruiseSpeed)

		//	@brief	��鷶Χ
		DECLARE_PROPERTY_ACCESSOR(float, SensorRange)

		//	@brief	�ڵ�����
		DECLARE_PROPERTY_ACCESSOR(float, TrajectoryGravity)

		//	@brief	����б��
		DECLARE_PROPERTY_ACCESSOR(float, AscendSlope)

		//	@brief	����б��
		DECLARE_PROPERTY_ACCESSOR(float, SwoopSlope)

		//	@brief	��Ӧʱ��
		DECLARE_PROPERTY_ACCESSOR(float, EchoInterval)

		//	@brief	��������ʱ��
		DECLARE_PROPERTY_ACCESSOR(float, ExLowFlyingTime)

		//	@brief	�����ٶ�
		DECLARE_PROPERTY_ACCESSOR(float, LayerChangeSpeed)

		//	@brief	��ʼͶ������
		DECLARE_PROPERTY_ACCESSOR(float, InitPrecision)

		//	@brief	��׼Ͷ������
		DECLARE_PROPERTY_ACCESSOR(float, Precision)

		//	@brief	Ͷ������ϵ��
		DECLARE_PROPERTY_ACCESSOR(float, PrecisionFactor)

		//	@brief	��Ȧ�ٶ�
		DECLARE_PROPERTY_ACCESSOR(float, Collimation)

		//	@brief	ת�򾫶�Ӱ��
		DECLARE_PROPERTY_ACCESSOR(float, TurningAffect)

		//	@brief	���㾫��Ӱ��
		DECLARE_PROPERTY_ACCESSOR(float, LayerAffect)

		//	@brief	Ŀ��㼶
		DECLARE_PROPERTY_ACCESSOR(FlightLayer, PendingLayer)

		//	@brief	������ٶ�
		DECLARE_PROPERTY_ACCESSOR(float, DiveAcceleration)

		//	@brief	�������ٶ�
		DECLARE_PROPERTY_ACCESSOR(float, PullAcceleration)

		//	@brief	Ͷ��ɢ��������
		DECLARE_PROPERTY_ACCESSOR(float, DiffuseEccentricity)

		//	@brief	ս�����ɹ�������
		DECLARE_PROPERTY_ACCESSOR(float, AttackExtent)

		//	@brief	ս�����ɹ����Ƕ�
		DECLARE_PROPERTY_ACCESSOR(float, AttackScope)

		//	@brief	λ���������
		virtual NxMat34	GetGlobalPos() const { return _WorldTransform; }
		virtual NxMat33	GetGlobalOritation() const { return _WorldTransform.M; }
		virtual NxQuat	GetGlobalOritationQuat() const { NxQuat Quat; _WorldTransform.M.toQuat(Quat); return Quat; }
		virtual NxVec3	GetGlobalPosition() const { return _WorldTransform.t; }
		virtual void SetGlobalPose(const NxMat34& Pose) { _WorldTransform = Pose; }
		virtual void USetGlobalPose(const NxMat34& Pose){ _WorldTransform = Pose; _WorldTransform.t *= G2PScale; }
		virtual void SetGlobalOritation(const NxMat33& Oritation) { _WorldTransform.M = Oritation; }
		virtual void SetGlobalOritationQuat(const NxQuat& Quat) { _WorldTransform.M.fromQuat(Quat); }
		virtual void SetGlobalPosition(const NxVec3& Position) { _WorldTransform.t = Position; }
		virtual void USetGlobalPosition(const NxVec3& UPosition) { _WorldTransform.t = UPosition * G2PScale; }
		inline NxVec3 UGetGlobalPosition() const { return _WorldTransform.t * P2GScale; }
		inline float GetGlobalHeight() const { return _WorldTransform.t.z; }
		inline void SetGlobalHeight(float Height) { _WorldTransform.t.z = Height; }
		inline void RestoreTargetPos(const NxVec3& Pos)	{ _UTargetPos = Pos * P2GScale; }
		inline void RestoreTargetDir(const NxVec3& Dir) { _UTargetDir = Dir; }

		//	@brief	�������г���
		inline int		GetActionListLength()	{ return _FSM.GetPendingActionNum(); }

		//	@brief	�����������ٶ�
		inline void		SetExigencePullAcceleration(float Acceleration) { _ExigencePullAcceleration = Acceleration; }

		//	@brief	��������ٶ�
		inline void		ResetVerticalSpeed()	{ _VerticalSpeed = 0.f; }

		//	@brief	��ȡ��ǰ�����ٶ�
		inline float	GetVerticalSpeed()		{ return _VerticalSpeed; }
		inline float	UGetVerticalSpeed()		{ return _VerticalSpeed * P2GScale; }

		inline void		ResetActionQueueInFSM()	{ _FSM.ClearActions(); }
		inline void		PopActionInFSM()		{ _FSM.PopAction(); }
		inline void		PushActionToFSM(AircraftAction action,  
			const NxVec3& tar, 
			const NxVec3& dir, 
			PhysGameObject* obj)	
		{
			_FSM.PushAction(action, tar, dir, obj);
		}

	protected:
		virtual void	InitPhys	();
		virtual void	PostInitPhys();
		virtual void	PrevTermPhys();

		//	@brief	ָ�����
		void			CommandParse(OperationCommand cmd, const NxVec3& tar, const NxVec3& dir, PhysGameObject* pObj);

		//	@brief	ִ��һ������
		DWORD			ExecuteAction();

		//	@brief	Ԥ��ָ��ת��
		void			PendingActionCopy(AircraftFSM& fsm);

		//	@brief	���ò�����λ�ã�
		//	@note	���ڶ���ά��
		void			PostFix(float DeltaTime);

		//	@brief	������Χɨ��
		void			ScanAttackRange(float DeltaTime);

		//	@brief	�ϰ�ɨ��
		void			ScanBlocks(float DeltaTime);

		//	@brief	�ϰ����
		bool			BlockTest();

	protected:
		//	@brief	״̬��
		AircraftFSM				_FSM;

		//	@brief	����
		PhysAircraft*			_Header;

		//	@brief	�Ż�
		AircraftList			_Wings;

		//	@brief	С��ID
		UINT					_SquadID;

		//	@brief	���ڱ��
		int						_SquadIndex;

		//	@brief	�ٶ�����
		float					_SpeedFactor;

		//	@brief	Ͷ����������
		float					_PrecisionFactor;

		//	@brief	���ػ����
		UINT					_Flags;

		//	@brief	������Χ�ڵķɻ�
		AircraftList			_AttackableList;

		//	@brief	�������λ����Ϣ
		NxMat34					_StartPoint;
		NxMat34					_LeavePoint;
		NxMat34					_CatchPoint;

		//	@brief	�任����
		NxMat34					_WorldTransform;

		//	@brief	ˮƽ����
		NxVec3					_Forward;

		//	@brief	��ֱ�����ƶ��ٶ�
		float					_VerticalSpeed;

		//	@brief	�����������ٶ�
		float					_ExigencePullAcceleration;

		//	@brief	���ʱ��
		float					_DetectInterval;

		//	@brief	�ۻ�ʱ��
		float					_AccumTime;

		//	@brief	Ŀ��㼶
		FlightLayer				_PendingLayer;

	public:
		//	@brief	�Ƿ񱾵����
		bool			_LocalPlayer;

		//	@brief	ָ���ƶ�
		bool			_MovePost;

		//	@brief	��ʼλ��
		NxVec3			_UStartPos;

		//	@brief	��ʼ����
		NxVec3			_UStartDir;

		//	@brief	Ŀ��λ��
		NxVec3			_UTargetPos;

		//	@brief	Ŀ�귽��
		NxVec3			_UTargetDir;

		//	@brief	���ط�Χ��ײ��
		RangeTrigger*	_InterceptTrigger;

		//	@brief	׷����Χ��ײ��
		RangeTrigger*	_ChaseTrigger;
	};

#include "PhysGameObjectAircraft.inl"

}	//	namespace GPL