//------------------------------------------------------------------------
//	@brief	舰载机物理对象
//	@author	chenpu
//	@date	2013-1-24
//------------------------------------------------------------------------
#include "AircraftFSM.h"

#pragma once

namespace GPL {

//	@brief	编队最大舰载机数量
#define	SQUAD_AIRCRAFT_MAX	(5)

//	@brief	默认速度
#define DEFAULT_SPEED		(10.f)

//	@brief	默认转向半径
#define DEFAULT_RADIUS		(10.f)

//	@brief	速度倍率上限
#define SPEED_FACTOR_UPPER_LIMIT	(2.f)

//	@brief	速度倍率下限
#define SPEED_FACTOR_LOWER_LIMIT	(0.005f)

//	@brief	属性访问接口
#define DECLARE_PROPERTY_ACCESSOR(Type, Prop)	\
	inline	void	Set##Prop(Type _value);	\
	inline	Type	Get##Prop();

	//	@brief	舰载机指令
	enum OperationCommand
	{
		_TakeOff = 0,			//	起飞
		_Cruise,				//	巡航
		_Convoy,				//	护航
		_Attack,				//	攻击
		_Recovery,				//	回航
		_LayerChange,			//	调整飞行高度
	};

	//	@brief	飞行层次
	enum FlightLayer
	{
		ExtremeLow = 0,	//	贴海（地）
		Lower,			//	低空
		Higher,			//	高空
		LayerNum
	};

	//	@brief	舰载机标记
	enum AircraftFlag
	{
		LockHeader = 1,		//	锁定长机
		VerticalMove,		//	纵向位移
		Ascend,				//	拉升
		Descend,			//	下降
		DiveAttack,			//	俯冲轰炸
		ExigencePull,		//	紧急拉升
		Falling,			//	坠落
		Collimate,			//	校准
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

		//	@brief	推送初始状态
		//	@note	Initialize only
		bool					PushInitState(AircraftAction action, const NxVec3& pos, const NxVec3& dir, PhysGameObject* obj);

		//	@brief	获得舰载机
		static PhysAircraft*	GetAircraft(PhysVehicle* pOwner, UINT squad, UINT index);

		//	@brief	接受指令
		//	@param	[cmd]			执行指令
		//	@param	[tar]			目标点
		//	@param	[dir]			方向
		//	@param	[pObj]			目标对象
		//	@return	[bool]			命令能否执行
		bool					OnCommand(OperationCommand cmd, const NxVec3& tar, const NxVec3& dir, PhysGameObject* pObj = NULL);

		//	@brief	接受指令（Ureal单位参数）
		bool					UOnCommand(OperationCommand cmd, const NxVec3& tar, const NxVec3& dir, PhysGameObject* pObj = NULL);

		//	@brief	追加一个指令
		//	@param	[cmd]			执行指令
		//	@param	[tar]			目标点
		//	@param	[dir]			方向
		//	@param	[pObj]			目标对象
		//	@return	[bool]			是否替换追加
		bool					PendingCommand(OperationCommand cmd, const NxVec3& tar, const NxVec3& dir, PhysGameObject* pObj = NULL);

		//	@brief	追加一个指令（Ureal单位参数）
		bool					UPendingCommand(OperationCommand cmd, const NxVec3& tar, const NxVec3& dir, PhysGameObject* pObj = NULL);

		//	@brief	动作变更
		//	@param	[Act]			动作ID
		//	@param	[tar]			目标点
		//	@param	[dir]			方向
		void					ChangeAction(AircraftAction Act, const NxVec3& tar, const NxVec3& dir);

		//	@brief	动作变更（Ureal单位参数）
		void					UChangeAction(AircraftAction Act, const NxVec3& tar, const NxVec3& dir);
		
		//	@brief	强制移动
		//	@param	[Pos]	位置
		//	@param	[Dir]	朝向
		void					ForceTransform(const NxVec3& Pos, const NxVec3& Dir);

		//	@brief	位置模拟
		//	@param	[Time]	经过时间
		//	@param	[Pos]	模拟位置
		//	@param	[Dir]	模拟朝向
		NxMat34					TransformSimulate(float Time, bool ignoreDir = true);

		//	@brief	坠机事件响应
		//	@param	[bShoot]	是否被击落
		void					OnCrash(bool bShoot = true);

		//	@brief	动作完成调用
		void					OnActionFinished();

		//	@brief	无指令动作
		void					NonCommandAction();

		//	@brief	俯冲
		float					Dive(float DeltaTime);

		//	@brief	紧急拉升
		float					Pull(float DeltaTime);

		//	@brief	爬升
		float					Incline(float DeltaTime);

		//	@brief	下降
		float					Decline(float DeltaTime);

		//	@brief	坠毁
		float					FallingDown(float DeltaTime);

		//	@brief	脱离甲板事件
		void					OnLeaveBoard();

		//	@brief	目标丢失事件
		virtual void			OnTargetLost(PhysGameObject* Target);

		//	@brief	获得编队相对位置
		//	@note	长机当前朝向
		NxVec3					GetFormationOffset();

		//	@brief	获得编队相对位置
		//	@param	[dir]	指定朝向
		NxVec3					GetFormationOffset(const NxVec3& dir);

		//	@brief	获得编队位置全局坐标
		NxVec3					GetFormationGlobalPosition();

		//	@brief	应用编队位置
		//	@note	直接编队偏移作为自己的位置（还有点问题，先别用）
		void					ApplyFormationTrans();

		//	@brief	设置标志位
		void					SetFlag(AircraftFlag flag, bool enable);

		//	@brief	检查标志是否开启
		inline bool				CheckFlag(AircraftFlag flag)	{ return CHECK_STATE(_Flags, flag); }

		//	@brief	获得舰载机类型
		EPartType				GetAircraftType();

		//	@brief	获得飞行层次
		FlightLayer				GetFlightLayer();

		//	@brief	获得当前状态
		inline GameObjectState*	GetCurrentState()				{ return _FSM.GetActiveState(); }

		//	@brief	获得当前状态枚举值
		inline AircraftAction	GetCurrentStateEnum()				
		{
			GameObjectState* State = _FSM.GetActiveState();
			return (NULL != State) ? AircraftAction(State->GetEnumValue()) : ActionNum;
		}

		//	@brief	获取意图（最后一个待执行动作）
		inline AircraftAction	GetIntent()						{ return _FSM.GetLastAction(); }

		//	@brief	母舰
		void					SetBaseShip(PhysGameObject* pOwner);
		PhysVehicle*			GetBaseShip();
		NxVec3					GetStartLocation();
		NxVec3					GetLeaveLocation();
		NxVec3					GetCatchLocation();

		//	@brief	编队ID
		inline void				SetSquadID(UINT id)				{ _SquadID = id; }
		inline UINT				GetSquadID()					{ return _SquadID; }

		//	@brief	队内编号
		void					SetSquadIndex(int index);
		inline int				GetSquadIndex()					{ return _SquadIndex; }

		//	@brief	长机
		void					SetHeader(PhysAircraft* header);
		inline PhysAircraft*	GetHeader()						{ return _Header; }

		//	@brief	僚机
		void					RegisterWing(PhysAircraft* wing);
		void					UnregisterWing(PhysAircraft* wing, bool resort = true);
		inline void				ClearWings()					{ _Wings.clear(); }
		UINT					GetWingNum()					{ return (UINT)_Wings.size(); }
		PhysAircraft*			GetWing(UINT idx);

		//	@brief	是否生存
		bool			IsAlive();

		//	@brief	盘旋中心
		NxVec3			GetConvoluteCenter();

		//	@brief	防卫中心
		NxVec3			GetGuardCenter();

		//	@brief	最小绕行半径
		float			GetMinRadius();
		
		//	@brief	盘旋半径
		float			GetConvoluteRadius();

		//	@brief	转向半径
		float			GetRadius(WiseType wise);

		//	@brief	速度因子
		void			SetSpeedFactor(float fFactor);

		//	@brief	速度标量
		float			GetSpeed();

		//	@brief	速度矢量
		NxVec3			GetVelocity();
		NxVec3			UGetVelocity()	{ return GetVelocity() * P2GScale; }

		//	@brief	朝向
		void			SetForward(const NxVec3& forward);
		NxVec3			GetForward();

		//	@brief	投弹精度
		inline void		InitPrecisionFactor()	{ _PrecisionFactor = 10000.f; }
		void			ModifyPrecisionFactor(float Delta);
		float			GetFinalPrecision();
		void			OnBombTargetChange(const NxVec3& tar);

		//	@brief	弹道类型
		ETrajectoryMethod	GetTrajectoryMethod();

		//	@brief	巡航速度
		DECLARE_PROPERTY_ACCESSOR(float, CruiseSpeed)

		//	@brief	侦查范围
		DECLARE_PROPERTY_ACCESSOR(float, SensorRange)

		//	@brief	炮弹重力
		DECLARE_PROPERTY_ACCESSOR(float, TrajectoryGravity)

		//	@brief	爬升斜率
		DECLARE_PROPERTY_ACCESSOR(float, AscendSlope)

		//	@brief	俯冲斜率
		DECLARE_PROPERTY_ACCESSOR(float, SwoopSlope)

		//	@brief	反应时间
		DECLARE_PROPERTY_ACCESSOR(float, EchoInterval)

		//	@brief	贴海飞行时间
		DECLARE_PROPERTY_ACCESSOR(float, ExLowFlyingTime)

		//	@brief	换层速度
		DECLARE_PROPERTY_ACCESSOR(float, LayerChangeSpeed)

		//	@brief	初始投弹精度
		DECLARE_PROPERTY_ACCESSOR(float, InitPrecision)

		//	@brief	最准投弹精度
		DECLARE_PROPERTY_ACCESSOR(float, Precision)

		//	@brief	投弹精度系数
		DECLARE_PROPERTY_ACCESSOR(float, PrecisionFactor)

		//	@brief	缩圈速度
		DECLARE_PROPERTY_ACCESSOR(float, Collimation)

		//	@brief	转向精度影响
		DECLARE_PROPERTY_ACCESSOR(float, TurningAffect)

		//	@brief	换层精度影响
		DECLARE_PROPERTY_ACCESSOR(float, LayerAffect)

		//	@brief	目标层级
		DECLARE_PROPERTY_ACCESSOR(FlightLayer, PendingLayer)

		//	@brief	俯冲加速度
		DECLARE_PROPERTY_ACCESSOR(float, DiveAcceleration)

		//	@brief	拉升加速度
		DECLARE_PROPERTY_ACCESSOR(float, PullAcceleration)

		//	@brief	投弹散布离心率
		DECLARE_PROPERTY_ACCESSOR(float, DiffuseEccentricity)

		//	@brief	战斗机可攻击距离
		DECLARE_PROPERTY_ACCESSOR(float, AttackExtent)

		//	@brief	战斗机可攻击角度
		DECLARE_PROPERTY_ACCESSOR(float, AttackScope)

		//	@brief	位置相关重载
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

		//	@brief	动作队列长度
		inline int		GetActionListLength()	{ return _FSM.GetPendingActionNum(); }

		//	@brief	设置拉升加速度
		inline void		SetExigencePullAcceleration(float Acceleration) { _ExigencePullAcceleration = Acceleration; }

		//	@brief	清空纵向速度
		inline void		ResetVerticalSpeed()	{ _VerticalSpeed = 0.f; }

		//	@brief	获取当前纵向速度
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

		//	@brief	指令分析
		void			CommandParse(OperationCommand cmd, const NxVec3& tar, const NxVec3& dir, PhysGameObject* pObj);

		//	@brief	执行一个动作
		DWORD			ExecuteAction();

		//	@brief	预设指令转移
		void			PendingActionCopy(AircraftFSM& fsm);

		//	@brief	后置补正（位置）
		//	@note	用于队形维持
		void			PostFix(float DeltaTime);

		//	@brief	攻击范围扫描
		void			ScanAttackRange(float DeltaTime);

		//	@brief	障碍扫描
		void			ScanBlocks(float DeltaTime);

		//	@brief	障碍检测
		bool			BlockTest();

	protected:
		//	@brief	状态机
		AircraftFSM				_FSM;

		//	@brief	长机
		PhysAircraft*			_Header;

		//	@brief	僚机
		AircraftList			_Wings;

		//	@brief	小队ID
		UINT					_SquadID;

		//	@brief	队内编号
		int						_SquadIndex;

		//	@brief	速度因子
		float					_SpeedFactor;

		//	@brief	投弹精度因子
		float					_PrecisionFactor;

		//	@brief	舰载机标记
		UINT					_Flags;

		//	@brief	攻击范围内的飞机
		AircraftList			_AttackableList;

		//	@brief	舰船相关位置信息
		NxMat34					_StartPoint;
		NxMat34					_LeavePoint;
		NxMat34					_CatchPoint;

		//	@brief	变换矩阵
		NxMat34					_WorldTransform;

		//	@brief	水平朝向
		NxVec3					_Forward;

		//	@brief	垂直方向移动速度
		float					_VerticalSpeed;

		//	@brief	紧急拉升加速度
		float					_ExigencePullAcceleration;

		//	@brief	检测时间
		float					_DetectInterval;

		//	@brief	累积时间
		float					_AccumTime;

		//	@brief	目标层级
		FlightLayer				_PendingLayer;

	public:
		//	@brief	是否本地玩家
		bool			_LocalPlayer;

		//	@brief	指令移动
		bool			_MovePost;

		//	@brief	起始位置
		NxVec3			_UStartPos;

		//	@brief	起始方向
		NxVec3			_UStartDir;

		//	@brief	目标位置
		NxVec3			_UTargetPos;

		//	@brief	目标方向
		NxVec3			_UTargetDir;

		//	@brief	拦截范围碰撞体
		RangeTrigger*	_InterceptTrigger;

		//	@brief	追击范围碰撞体
		RangeTrigger*	_ChaseTrigger;
	};

#include "PhysGameObjectAircraft.inl"

}	//	namespace GPL