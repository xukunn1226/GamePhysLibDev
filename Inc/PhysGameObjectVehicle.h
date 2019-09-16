#ifndef _PHYS_GAMEOBJECT_VEHICLE_H_
#define _PHYS_GAMEOBJECT_VEHICLE_H_

namespace GPL
{

// Phys scale制式下的值
struct VehicleState
{
	NxVec3		Position;
	NxQuat		Quaternion;
	NxVec3		LinVel;
	NxVec3		AngVel;
	NxReal		Angle;	// in degree
};

// UE制式下的值
struct UVehicleState
{
	NxVec3		Position;
	NxQuat		Quaternion;
	NxVec3		LinVel;
	NxVec3		AngVel;
	NxReal		Angle;	// in degree
};

struct CriticalFireData
{
	int			NecessaryTimesMin;
	int			NecessaryTimesMax;
	int			NecessaryTimes;
	int			AccumTimes;
	float		AccuracyMultiplier;
};

class PhysVehicle : public PhysGameObject
{
	DECLARE_RTTI

public:
	PhysVehicle(PhysGameScene* GameScene, VehicleData* Data)
		: PhysGameObject(GameScene, Data)
		, _SimObj(NULL), _SimObjWhenDeath(NULL), _SimObjWhenLive(NULL)
		, _bValidRecordPosition(false)
		, _RecordPos(NxVec3(0.f))
		, _MachineGunCheckTime(0.2f)
		, _MachineGunAttackRange(100)
		, _MachineGunCheckTimeDelta(0.0f)
		, _CurLevel(0)
		, _PendingLevel(0)
		, _OriginalLocZ(0.f)
		, _DesiredLocZ(0.f)
		, _CurrLocZ(0.f)
		, _DeltaLocZ(0.f)
		, _PrevVelocity(NxVec3(0.f))
		, _Velocity(NxVec3(0.f))
		, _PrevAngularVelocity(NxVec3(0.f))
		, _AngularVelocity(NxVec3(0.f))
		, _VelocitySize(0.f)
		, _AngularVelocityZSize(0.f)
		, _FinalAccuracyMultiplier(1.f)
		, _CurAccuracyMultiplier(1.f)
		, _DistAccuracyMultiplier(1.f)
		, _FireAccuracyMultiplier(1.f)
		, _AccuracyRecover(0.f)
		, _bSupportContinuousFire(true)
		, _bDetectable(false)
#ifdef _CLIENT_RUNTIME
		, _SubmarineSurfaceDummy(NULL)
#endif // _CLIENT_RUNTIME
	{}
	
	virtual bool Init();

	virtual void Tick(float DeltaTime);
	
	virtual bool IsValidOp(EMoveFlag Flag);

	bool HaveAnyFlag(EMoveFlag Flag)
	{
		return _SimObj != NULL ? _SimObj->HaveAnyFlag(Flag) : false;
	}

	// 移动控制接口
	EMoveFlag GetMoveFlag()
	{
		return _SimObj ? _SimObj->GetMoveFlag() : MoveFlag_None;
	}

	// 重置移动标识
	void ResetMoveFlag(EMoveFlag Flag);

	// 开始死亡逻辑
	void DeathSinkSimulate();
	void RandomSinkParams();
	void SetSinkParams(float fRoll, float fPitch, float fForceFactor, float fTorqueFactor);

	int GetActivePartType() { return _ActivePartType; }

	// 复活
	void Revive();

	/**
	 * 停止移动
	 */
	void Stop();
	/**
	 * 前向移动控制
	 * @bForward	true,持续按住“W"；false，“W”松开
	 */
	void MoveForward(bool bForward);
	/**
	 * 后向移动控制
	 * @bBackward	true,持续按住“S"；false，“S”松开
	 */
	void MoveBackward(bool bBackward);
	/**
	 * 右转向控制
	 * @bRight	true,持续按住“D"；false，“D”松开
	 */
	void TurnRight(bool bRight);
	/**
	 * 左转向控制
	 * @bLeft	true,持续按住“A"；false，“A”松开
	 */
	void TurnLeft(bool bLeft);
	/**
	 * 升降前进档
	 * @bForward  true, 升档； false，降档
	 */
	void ShiftDriving(bool bForward);
	/**
	 * 升降转向档
	 * @bRightTurn  true, 右转向档； false，左转向档
	 */
	void ShiftSteering(bool bRightTurn);
	/**
	 * 潜艇上浮
	 * @param	Level(-2, 0]	上浮至第几层, Level: 0,水面层；-1，水下一层；-2，水下二层
	 */
	void Riseup(int CurLevel,int PendingLevel);
	/**
	 * 潜艇下潜
	 * @param	Level[-2, 0)	下潜至第几层, Level: 0,水面层；-1，水下一层；-2，水下二层
	 */
	void Sink(int CurLevel,int PendingLevel);
	/**
	 * 是否有前进,后退,巡航操作
	 */
	virtual bool HaveAnyMoveOp();
	/**
	 * 是否有转向，巡航操作
	 */
	virtual bool HaveAnyTurnOp();

	/**
	 * 是否在进退巡航中
	 */
	virtual bool IsInCruiseDrive();
	
	/**
	 * 是否在转向巡航中
	 */
	virtual bool IsInCruiseSteer();

	// 获得当前速度与最大速度比值
	float GetMaxVelocityRatio();

	// 载具是否为潜艇
	bool IsSubmarine();

	int  GetDriveGear();
	int  GetSteerGear();
	bool  GetMaxDriveGear(int &drive,int &reverse);
	bool  GetMaxSteerGear(int &steer);

	void SetDriveGear(int iGear);
	void SetSteerGear(int iGear);

	int ClampDriveGear(int iGear);
	int ClampSteerGear(int iGear);
	
	bool SetLevelInfo(int CurLevel, int PendingLevel);

	void UGetVehicleState(UVehicleState& NewState);

	void GetVehicleState(VehicleState& NewState);
	// for server, 根据服务器协议获取数据
	// AngleYaw		unsigned char, in degree[0 - 360]
	// AngleVelYaw	signed char, in degree[-128, 127]
	void UGetVehicleStateForServer(float& PosX, float& PosY, float& PosZ, unsigned char& AngleYaw, float& LinVelX, float& LinVelY, float& LinVelZ, signed char& AngleVelYaw);

	
	VehicleSimBase* GetVehicleSimulator() const { return _SimObj; }
	
	// 最大前进速度
	float GetMaxForwardSpeed();
	float UGetMaxForwardSpeed();

	float GetInitializedMaxForwardSpeed();
	float UGetInitializedMaxForwardSpeed();

	// 最大倒退速度
	float GetMaxReverseSpeed();
	float UGetMaxReverseSpeed();
	float GetInitializedMaxReverseSpeed();
	float UGetInitializedMaxReverseSpeed();

	// 倒退时的最大力
	float GetMaxReverseForce();

	//目标角速度
	float GetPendingTurnRate();

	// 最大角速度
	float GetMaxTurnRate();

	NxVec3 UGetCurVelocity();
	float  UGetCurVelocityMag();

	// 巡航模式时的档位速度
	float GetCruiseSpeed(int ShiftIndex, bool bForward);

	// 巡航模式时的角速度
	float GetCruiseTurnRate(int ShiftIndex, float MaxTurnRate);

	// 仅转向驱动时速度为一档巡航时的速度
	float GetSpeedWhenNoThrottle();

	// 设置最大航速
	void USetMaxForwardSpeed(float MaxSpeed);

	// 设置最大前进FORCE
	void USetMaxForwardForce(float MaxForce);

	// 设置最大扭矩
	void USetMaxTorque(float MaxTorque);

	// 设置排水量
	void USetMass(float Mass);

	// 设置炮台转动角速度
	void USetTurretAngleVelocity(int AttachedID, float TurretAngleVelocity);

	// 设置武器炮弹初速度
	void USetInitVelocity(int AttachedID, float InitVelocity);

	// 设置射出的炮弹所受重力加速度
	void USetGravity(int AttachedID, float Gravity);

	// 设置默认射击散射半径
	void USetAccuracyDefault(int AttachedID, float AccuracyDefault);

	// 设置炮台转向对射击精确度的影响
	void USetAccuracyTurrent(int AttachedID, float AccuracyTurrent);

	// 设置舰船行驶对射击精确度的影响
	void USetAccuracyShipDrive(int AttachedID, float AccuracyShipDrive);

	// 设置舰船转向对射击精确度的影响
	void USetAccuracyShipSteer(int AttachedID, float AccuracyShipSteer);

	// 设置射击精确度每秒恢复值
	void USetAccuracyRecover(int AttachedID, float AccuracyRecover);

	// 设置开炮对射击精确度的影响
	void USetAccuracyFire(int AttachedID, float AccuracyFire);

	//设置潜水艇 上浮下潜力
	void USetMaxRiseSinkForce(float MaxForce);
	
	/////////////////////////// 部件装配接口
	void InitAllAttachment();

	// 装载部件
	ShipComponent* AttachAttachment( ComponentData* CompData, Socket AttachedTo );

	// 根据部件ID获取部件对象
	ShipComponent* GetShipComponent( int AttachedID );
	
	void SetAimTarget( EPartType PartType, NxVec3 TargetLoc);
	void USetAimTarget( EPartType PartType, NxVec3 TargetLoc);

	bool IsVelocityFoward();// 速度是否是向前的

	void OnSetFired(int AttachID,int FirePartID, NxVec3 TargetPos = NxVec3(0.f), EPartType FireFromPartType = EPT_Invalid);//开火时候调用

	void OnProjectileHit(PhysProjectile * Projectile, float AccuracyContinuousFire, EPartType FireFromPartType = EPT_Invalid);//炮弹命中的时候调用

	// 是否支持越打越准逻辑
	bool SupportContinuousFire(EPartType PartType);

	// 记录点失效
	void EnableRecordPoint(NxVec3 RecordPos);
	void DisableRecordPoint();
	inline bool IsRecordValid()					{ return _bValidRecordPosition; }
	inline NxVec3 GetRecordPoint()				{ return _RecordPos; }

	// 是否存在射击精度影响
	bool HasFireAimingAffect();

	float GetLimitAccuracy();
	float GetEvaluateAccuracy(const NxVec3& AimingPoint);
	inline float GetFinalAccuracy()				{ return _FinalAccuracyMultiplier; }
	NxVec3 GetLastAimingPos();
	float GetEscapedTime();

	void OnNotifyStopRiseOrSink();

	void ForceReachDesiredLayer();

	NxU32 UGetAIManchineGunFiringRange(NxF32 Radius, PhysGameObject** PhysGameObjs, NxU32 PhysGameObjsNum, bool accurateCollision);

	void GetAccuracyInfo(int& AttachedId,
						 float& AccuracyOfLinearVelocity, 
						 float& AccuracyOfAngularVelocity, 
						 float& AccuracyOfTurrentTurn, 
						 float& MaxAccuracyOfTurrent,
						 float& CurAccuracyOfTurrent,
						 float& CurAccuracyMultiplier, 
						 float& FireAccuracyMultiplier,
						 float& DistAccuracyMultiplier,
						 float& FinalAccuracyMultiplier);

	//	@brief	获得鱼雷齐射参数
	void	GetTorpedoFireParam(TorpedoFireInfo& FireInfo, bool bSubmarine);
	void	UGetTorpedoFireParam(TorpedoFireInfo& FireInfo, bool bSubmarine);

	//  [10/23/2013 chenpu]	获取倾覆参数
	float	GetCapsizalRoll();
	float	GetCapsizalPitch();
	float	GetCapsizalTorqueFactor();
	float	GetSinkForceFactor();
	bool	IsSinking();
	bool	CheckUnderSurface();

	//	去除舰船一般碰撞属性
	void RemoveCollision();

	// 恢复舰船碰撞属性
	void RecoverCollision();

	//	@brief	可检测性
	void			SetDetectable(bool bDetectable);
	inline bool		IsDetectable()					{ return _bDetectable; }

	//	@brief	必中射击相关接口
	void	InitCriticalFireData(int PartType, int MinTimes, int MaxTimes, float Accuracy);
	void	ClearCriticalFireRecord(int PartType);
	void	ResetCriticalFireData(int PartType);
	float	GetCriticalFireAccuracyMultiplier(int PartType);

	NxVec3					_PrevVelocity;
	NxVec3					_Velocity;
	NxVec3					_PrevAngularVelocity;
	NxVec3					_AngularVelocity;			// [in radians] 角速度
	float					_VelocitySize;
	float					_AngularVelocityZSize;

	float					_OriginalLocZ;			// 初始位置Z值
	float					_DesiredLocZ;			// 目标位置Z值
	float					_CurrLocZ;				// 当前位置Z值
	float					_DeltaLocZ;				// 状态发生改变时，当前位置距目标位置的Z值
	
	int						_CurLevel;
	int						_PendingLevel;

#if 1
	NxBox					_EigenBox;
#endif

protected:
	virtual void InitPhys();

	virtual void PostInitPhys();

	virtual void PrevTermPhys();

	//这个是老接口，AI机枪有新逻辑，不要使用这个！！！
	void CheckManchineGun(float DeltaTime);
	
	void UpdateMassProp(float Mass, float CMassOffsetX);

	// 计算瞄准点与记录点间距离而产生的精准度影响值
	float CalcDistAccuracyMultiplier(NxVec3 AimingPos);

	float GetLevelLocation(int level);

	void ResetCriticalFireData(CriticalFireData& InData);

#ifdef _CLIENT_RUNTIME
	bool	CreateSubmarineSurfaceDummy();

	void	SyncSubmarineSurfaceDummy();
#endif // _CLIENT_RUNTIME

protected:
	VehicleSimBase*				_SimObj;				// 当前SimulationType
	VehicleSimBase*				_SimObjWhenDeath;		// 死亡时的
	VehicleSimBase*				_SimObjWhenLive;		// 非死亡时的

	ComponentList				_AttachedComponents;	// 以AttachedId为下标索引
	
	float _MachineGunCheckTime;
	float _MachineGunAttackRange;

	float _MachineGunCheckTimeDelta;

	int _ActivePartType;

	NxVec3	_TargetPos;
	bool	_bValidRecordPosition;			// 记录点是否有效
	NxVec3	_RecordPos;						// 记录点位置

	float	_FirePointLastTime;				// 记录点持续的时间
	bool	_RecordAimingOccasion;			// 射击点的记录方式 true 是开火时候记录 false 是 命中后记录
	float	_AccuracyFormulaA;
	float	_AccuracyFormulaB;
	float	_AccuracyFormulaC;
	float	_AccuracyFormulaD;
	float   _LastFireEscapedTime;			// 上次射击后过去的时间

	float	_CurAccuracyMultiplier;			// 当前精准度的倍乘
	float	_FinalAccuracyMultiplier;		// 目标基础精准度的倍乘
	float	_FireAccuracyMultiplier;		// 开火对基础精准度的倍乘
	float	_DistAccuracyMultiplier;		// 瞄准点与记录点距离偏差而影响的精准度倍乘
	float	_AccuracyRecover;				// 缩圈速度
	bool	_bSupportContinuousFire;		// 当前选择的PartType是否支持越打越准逻辑

	bool	_bDetectable;					//	可检测性

	CriticalFireData	_MainArtilleryFireData;
	CriticalFireData	_SubArtilleryFireData;

#ifdef _CLIENT_RUNTIME
	NxShape*	_SubmarineSurfaceDummy;		//	潜水艇Dummy
#endif // _CLIENT_RUNTIME
};


}

#endif