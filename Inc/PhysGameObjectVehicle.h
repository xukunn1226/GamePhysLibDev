#ifndef _PHYS_GAMEOBJECT_VEHICLE_H_
#define _PHYS_GAMEOBJECT_VEHICLE_H_

namespace GPL
{

// Phys scale��ʽ�µ�ֵ
struct VehicleState
{
	NxVec3		Position;
	NxQuat		Quaternion;
	NxVec3		LinVel;
	NxVec3		AngVel;
	NxReal		Angle;	// in degree
};

// UE��ʽ�µ�ֵ
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

	// �ƶ����ƽӿ�
	EMoveFlag GetMoveFlag()
	{
		return _SimObj ? _SimObj->GetMoveFlag() : MoveFlag_None;
	}

	// �����ƶ���ʶ
	void ResetMoveFlag(EMoveFlag Flag);

	// ��ʼ�����߼�
	void DeathSinkSimulate();
	void RandomSinkParams();
	void SetSinkParams(float fRoll, float fPitch, float fForceFactor, float fTorqueFactor);

	int GetActivePartType() { return _ActivePartType; }

	// ����
	void Revive();

	/**
	 * ֹͣ�ƶ�
	 */
	void Stop();
	/**
	 * ǰ���ƶ�����
	 * @bForward	true,������ס��W"��false����W���ɿ�
	 */
	void MoveForward(bool bForward);
	/**
	 * �����ƶ�����
	 * @bBackward	true,������ס��S"��false����S���ɿ�
	 */
	void MoveBackward(bool bBackward);
	/**
	 * ��ת�����
	 * @bRight	true,������ס��D"��false����D���ɿ�
	 */
	void TurnRight(bool bRight);
	/**
	 * ��ת�����
	 * @bLeft	true,������ס��A"��false����A���ɿ�
	 */
	void TurnLeft(bool bLeft);
	/**
	 * ����ǰ����
	 * @bForward  true, ������ false������
	 */
	void ShiftDriving(bool bForward);
	/**
	 * ����ת��
	 * @bRightTurn  true, ��ת�򵵣� false����ת��
	 */
	void ShiftSteering(bool bRightTurn);
	/**
	 * Ǳͧ�ϸ�
	 * @param	Level(-2, 0]	�ϸ����ڼ���, Level: 0,ˮ��㣻-1��ˮ��һ�㣻-2��ˮ�¶���
	 */
	void Riseup(int CurLevel,int PendingLevel);
	/**
	 * Ǳͧ��Ǳ
	 * @param	Level[-2, 0)	��Ǳ���ڼ���, Level: 0,ˮ��㣻-1��ˮ��һ�㣻-2��ˮ�¶���
	 */
	void Sink(int CurLevel,int PendingLevel);
	/**
	 * �Ƿ���ǰ��,����,Ѳ������
	 */
	virtual bool HaveAnyMoveOp();
	/**
	 * �Ƿ���ת��Ѳ������
	 */
	virtual bool HaveAnyTurnOp();

	/**
	 * �Ƿ��ڽ���Ѳ����
	 */
	virtual bool IsInCruiseDrive();
	
	/**
	 * �Ƿ���ת��Ѳ����
	 */
	virtual bool IsInCruiseSteer();

	// ��õ�ǰ�ٶ�������ٶȱ�ֵ
	float GetMaxVelocityRatio();

	// �ؾ��Ƿ�ΪǱͧ
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
	// for server, ���ݷ�����Э���ȡ����
	// AngleYaw		unsigned char, in degree[0 - 360]
	// AngleVelYaw	signed char, in degree[-128, 127]
	void UGetVehicleStateForServer(float& PosX, float& PosY, float& PosZ, unsigned char& AngleYaw, float& LinVelX, float& LinVelY, float& LinVelZ, signed char& AngleVelYaw);

	
	VehicleSimBase* GetVehicleSimulator() const { return _SimObj; }
	
	// ���ǰ���ٶ�
	float GetMaxForwardSpeed();
	float UGetMaxForwardSpeed();

	float GetInitializedMaxForwardSpeed();
	float UGetInitializedMaxForwardSpeed();

	// ������ٶ�
	float GetMaxReverseSpeed();
	float UGetMaxReverseSpeed();
	float GetInitializedMaxReverseSpeed();
	float UGetInitializedMaxReverseSpeed();

	// ����ʱ�������
	float GetMaxReverseForce();

	//Ŀ����ٶ�
	float GetPendingTurnRate();

	// �����ٶ�
	float GetMaxTurnRate();

	NxVec3 UGetCurVelocity();
	float  UGetCurVelocityMag();

	// Ѳ��ģʽʱ�ĵ�λ�ٶ�
	float GetCruiseSpeed(int ShiftIndex, bool bForward);

	// Ѳ��ģʽʱ�Ľ��ٶ�
	float GetCruiseTurnRate(int ShiftIndex, float MaxTurnRate);

	// ��ת������ʱ�ٶ�Ϊһ��Ѳ��ʱ���ٶ�
	float GetSpeedWhenNoThrottle();

	// ���������
	void USetMaxForwardSpeed(float MaxSpeed);

	// �������ǰ��FORCE
	void USetMaxForwardForce(float MaxForce);

	// �������Ť��
	void USetMaxTorque(float MaxTorque);

	// ������ˮ��
	void USetMass(float Mass);

	// ������̨ת�����ٶ�
	void USetTurretAngleVelocity(int AttachedID, float TurretAngleVelocity);

	// ���������ڵ����ٶ�
	void USetInitVelocity(int AttachedID, float InitVelocity);

	// ����������ڵ������������ٶ�
	void USetGravity(int AttachedID, float Gravity);

	// ����Ĭ�����ɢ��뾶
	void USetAccuracyDefault(int AttachedID, float AccuracyDefault);

	// ������̨ת��������ȷ�ȵ�Ӱ��
	void USetAccuracyTurrent(int AttachedID, float AccuracyTurrent);

	// ���ý�����ʻ�������ȷ�ȵ�Ӱ��
	void USetAccuracyShipDrive(int AttachedID, float AccuracyShipDrive);

	// ���ý���ת��������ȷ�ȵ�Ӱ��
	void USetAccuracyShipSteer(int AttachedID, float AccuracyShipSteer);

	// ���������ȷ��ÿ��ָ�ֵ
	void USetAccuracyRecover(int AttachedID, float AccuracyRecover);

	// ���ÿ��ڶ������ȷ�ȵ�Ӱ��
	void USetAccuracyFire(int AttachedID, float AccuracyFire);

	//����Ǳˮͧ �ϸ���Ǳ��
	void USetMaxRiseSinkForce(float MaxForce);
	
	/////////////////////////// ����װ��ӿ�
	void InitAllAttachment();

	// װ�ز���
	ShipComponent* AttachAttachment( ComponentData* CompData, Socket AttachedTo );

	// ���ݲ���ID��ȡ��������
	ShipComponent* GetShipComponent( int AttachedID );
	
	void SetAimTarget( EPartType PartType, NxVec3 TargetLoc);
	void USetAimTarget( EPartType PartType, NxVec3 TargetLoc);

	bool IsVelocityFoward();// �ٶ��Ƿ�����ǰ��

	void OnSetFired(int AttachID,int FirePartID, NxVec3 TargetPos = NxVec3(0.f), EPartType FireFromPartType = EPT_Invalid);//����ʱ�����

	void OnProjectileHit(PhysProjectile * Projectile, float AccuracyContinuousFire, EPartType FireFromPartType = EPT_Invalid);//�ڵ����е�ʱ�����

	// �Ƿ�֧��Խ��Խ׼�߼�
	bool SupportContinuousFire(EPartType PartType);

	// ��¼��ʧЧ
	void EnableRecordPoint(NxVec3 RecordPos);
	void DisableRecordPoint();
	inline bool IsRecordValid()					{ return _bValidRecordPosition; }
	inline NxVec3 GetRecordPoint()				{ return _RecordPos; }

	// �Ƿ�����������Ӱ��
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

	//	@brief	��������������
	void	GetTorpedoFireParam(TorpedoFireInfo& FireInfo, bool bSubmarine);
	void	UGetTorpedoFireParam(TorpedoFireInfo& FireInfo, bool bSubmarine);

	//  [10/23/2013 chenpu]	��ȡ�㸲����
	float	GetCapsizalRoll();
	float	GetCapsizalPitch();
	float	GetCapsizalTorqueFactor();
	float	GetSinkForceFactor();
	bool	IsSinking();
	bool	CheckUnderSurface();

	//	ȥ������һ����ײ����
	void RemoveCollision();

	// �ָ�������ײ����
	void RecoverCollision();

	//	@brief	�ɼ����
	void			SetDetectable(bool bDetectable);
	inline bool		IsDetectable()					{ return _bDetectable; }

	//	@brief	���������ؽӿ�
	void	InitCriticalFireData(int PartType, int MinTimes, int MaxTimes, float Accuracy);
	void	ClearCriticalFireRecord(int PartType);
	void	ResetCriticalFireData(int PartType);
	float	GetCriticalFireAccuracyMultiplier(int PartType);

	NxVec3					_PrevVelocity;
	NxVec3					_Velocity;
	NxVec3					_PrevAngularVelocity;
	NxVec3					_AngularVelocity;			// [in radians] ���ٶ�
	float					_VelocitySize;
	float					_AngularVelocityZSize;

	float					_OriginalLocZ;			// ��ʼλ��Zֵ
	float					_DesiredLocZ;			// Ŀ��λ��Zֵ
	float					_CurrLocZ;				// ��ǰλ��Zֵ
	float					_DeltaLocZ;				// ״̬�����ı�ʱ����ǰλ�þ�Ŀ��λ�õ�Zֵ
	
	int						_CurLevel;
	int						_PendingLevel;

#if 1
	NxBox					_EigenBox;
#endif

protected:
	virtual void InitPhys();

	virtual void PostInitPhys();

	virtual void PrevTermPhys();

	//������Ͻӿڣ�AI��ǹ�����߼�����Ҫʹ�����������
	void CheckManchineGun(float DeltaTime);
	
	void UpdateMassProp(float Mass, float CMassOffsetX);

	// ������׼�����¼������������ľ�׼��Ӱ��ֵ
	float CalcDistAccuracyMultiplier(NxVec3 AimingPos);

	float GetLevelLocation(int level);

	void ResetCriticalFireData(CriticalFireData& InData);

#ifdef _CLIENT_RUNTIME
	bool	CreateSubmarineSurfaceDummy();

	void	SyncSubmarineSurfaceDummy();
#endif // _CLIENT_RUNTIME

protected:
	VehicleSimBase*				_SimObj;				// ��ǰSimulationType
	VehicleSimBase*				_SimObjWhenDeath;		// ����ʱ��
	VehicleSimBase*				_SimObjWhenLive;		// ������ʱ��

	ComponentList				_AttachedComponents;	// ��AttachedIdΪ�±�����
	
	float _MachineGunCheckTime;
	float _MachineGunAttackRange;

	float _MachineGunCheckTimeDelta;

	int _ActivePartType;

	NxVec3	_TargetPos;
	bool	_bValidRecordPosition;			// ��¼���Ƿ���Ч
	NxVec3	_RecordPos;						// ��¼��λ��

	float	_FirePointLastTime;				// ��¼�������ʱ��
	bool	_RecordAimingOccasion;			// �����ļ�¼��ʽ true �ǿ���ʱ���¼ false �� ���к��¼
	float	_AccuracyFormulaA;
	float	_AccuracyFormulaB;
	float	_AccuracyFormulaC;
	float	_AccuracyFormulaD;
	float   _LastFireEscapedTime;			// �ϴ�������ȥ��ʱ��

	float	_CurAccuracyMultiplier;			// ��ǰ��׼�ȵı���
	float	_FinalAccuracyMultiplier;		// Ŀ�������׼�ȵı���
	float	_FireAccuracyMultiplier;		// ����Ի�����׼�ȵı���
	float	_DistAccuracyMultiplier;		// ��׼�����¼�����ƫ���Ӱ��ľ�׼�ȱ���
	float	_AccuracyRecover;				// ��Ȧ�ٶ�
	bool	_bSupportContinuousFire;		// ��ǰѡ���PartType�Ƿ�֧��Խ��Խ׼�߼�

	bool	_bDetectable;					//	�ɼ����

	CriticalFireData	_MainArtilleryFireData;
	CriticalFireData	_SubArtilleryFireData;

#ifdef _CLIENT_RUNTIME
	NxShape*	_SubmarineSurfaceDummy;		//	ǱˮͧDummy
#endif // _CLIENT_RUNTIME
};


}

#endif