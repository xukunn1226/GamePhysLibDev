#ifndef _PHYS_GAMEOBJECT_PROJECTILE_H_
#define _PHYS_GAMEOBJECT_PROJECTILE_H_

#include <map>

namespace GPL
{
class PhysProjectile : public PhysGameObject
{
public:
	PhysProjectile(PhysGameScene* GameScene, ProjectileData* Data)
	 : PhysGameObject(GameScene, Data), _Sponsor(NULL), _bForceSelfDestruction(false)
	{}
	virtual ~PhysProjectile(){}

	enum EHitState
	{
		enHitIdle,			// 正常状态，没有命中
		enHit,				// 命中
		enSelfDestory		// 自己销毁
	};

	struct MagneticTargetStatus
	{
		PhysGameObject* _Target;
		bool			_Lost;
		float			_Distance;
	};

	virtual bool Init();

	virtual void Tick(float DeltaTime);

	void InitProData();

	// set/get sponsor
	void SetSponsor(PhysGameObject* Sponsor);
	PhysGameObject *GetSponsor(); 
	
	virtual NxVec3	GetGlobalPosition() const;

	// 从起始点到目标点的飞行时间
	virtual float GetFlightTime(NxVec3 TargetPos);

	// 经ElapsedTime后炮弹的位置
	virtual NxVec3 GetFlightLocation(float ElapsedTime);

	// 经ElapsedTime后炮弹速度
	virtual NxVec3 GetFlightVelocity(float ElapsedTime);

	//	@brief	获取弹道类型
	ETrajectoryMethod	GetTrajectoryMethod();

	//	@brief	获取目标位置
	NxVec3 GetTargetPos();

	//	@brief	获取炮弹朝向
	NxVec3 GetTowards();

	// 获得自适应的炮弹模拟时间
	float GetAdaptiveSimulatedTime();

	// 当前炮弹已飞行的时间及是否命中
	void GetCurFlightTime(float& ElapsedFlightTime, bool& bHit)
	{
		ElapsedFlightTime = _ElapsedFlightTime;
		bHit = (_HitState!=enHitIdle);
	}

	void ForceSelfDestruction();

	void EnableUtilityTest(bool val) { _bUtilityTest = val; }
	void EnableVehicleTest(bool val) { _bVehicleTest = val; }

	NxVec3 GetHitPos() const { return _HitPos; }
	EHitState GetHitState() const { return _HitState; }
	void SetTestSimulatedTime(float val) { _TestSimulatedTime = val; }

	//	@brief	目标是否处于火力覆盖范围内
	bool	IsTargetInCoverage(float LowerLimitAngle, float UpperLimitAngle);

	//	@brief	设置发射部件的AttachId
	inline void		SetSponsorAttachId(int AttachId)	{ _SponsorAttachId = AttachId; }
	//	@brief	获取发射部件的AttachId
	inline int		GetSponsorAttachId()				{ return _SponsorAttachId; }
	//	@brief	获得第一个有效打击目标
	inline PhysGameObject*	GetFirstValidHitActor()		{ return _FirstValidHitActor; }

protected:
	class ProjectileRaycastReport : public NxUserRaycastReport
	{
	public:
		ProjectileRaycastReport(PhysGameObject* Sponsor, NxRaycastHit* hit)
			: _Sponsor(Sponsor)
			, _Hit(hit)
			, _ValidHit(false)
		{}

		// 返回TRUE，继续检测；返回FALSE，停止检测
		virtual bool onHit(const NxRaycastHit& hits);

	public:
		PhysGameObject* _Sponsor;
		NxRaycastHit*	_Hit;
		bool			_ValidHit;
	};

	PhysProjectile() {}
	PhysProjectile(const PhysProjectile& Copy) {}
	PhysProjectile& operator=(const PhysProjectile& right) {}
	
	/**
	 * 计算起始点至目标点的飞行时间
	 */
	float GetPartFlightTime(ETrajectoryMethod method, NxVec3 Gravity, NxVec3 StartPos,NxVec3 InitVelocity,NxVec3 TargetPos);

	/**
	 * 从起始点开始经过ElapsedTime后炮弹飞行到的位置
	 */
	NxVec3 GetPartFlightLocation(ETrajectoryMethod method,NxVec3 StartPos,NxVec3 InitVelocity,float ElapsedTime);

	/**
	 * 计算起始速度经过ElapsedTime后的速度
	 */
	NxVec3 GetPartFlightVelocity(ETrajectoryMethod method,NxVec3 InitVelocity,float ElapsedTime);

	/**
	 * 炮弹落水后飞行的速度，仅针对鱼雷、深水炸弹有效
	 */
	NxVec3 GetBrakingVelocity(ETrajectoryMethod method);

	virtual void InitPhys();

	/**
	 * 判断[StartPos,EndPos]是否有碰撞产生
	 * 返回TRUE，发生碰撞；FALSE,没有发生碰撞
	 */
	virtual bool IsHitObject(NxVec3 StartPos, NxVec3 EndPos);

	/**
	 * 磁感应鱼雷引爆检测
	 */
	bool MagneticBurstTest(float DeltaTime);

	/**
	 * 炮弹被引爆后，计算爆炸范围伤害，通知受到伤害的ShipComponent
	 */
	void CheckBurstHit(NxVec3 pos,NxVec3 normal, NxVec3 HitDirection, ShipComponent* IngoreComponent, PhysGameObject* IngoreActor);

	/**
	 * 发送命中结果
	 * 返回TRUE，炮弹继续检测内部碰撞信息；FALSE，中止继续检测
	 */
	bool SendStrikeMsg(GameObject* HitComp,
					   NxVec3 StrikeNormal=NxVec3(0.0f,0.0f,1.0f),
					   NxVec3 StrikePos=NxVec3(0.0f),
					   PhysGameObject* vehile=NULL,
					   float Distance = 0.f,
					   bool bDirectHit=true);

	// 发送最后命中点的通知
	void PostBurstMsg(NxVec3 HitPos,NxVec3 HitNormal, NxVec3 HitDirection);

	/**
	 * 检测炮弹飞行轨迹是否需要改变（仅当鱼雷、深水炸弹落水后需要改变）
	 * 返回TRUE，轨迹发生改变；FALSE，不需要改变
	 */
	bool CheckChangeTrajectory(PhysGameObject * HitObj,const NxRaycastHit &HitInfo);

	/**
	 * 计算爆炸点距舰船的最近距离
	 */
	float GetBurstDistance(const NxVec3 &BurstPos,PhysVehicle* ship);

	/**
	 * 某段射线内碰撞结果
	 */
	void raycastObject(
		const NxVec3& StartPos, 
		const NxVec3& EndPos, 
		NxRaycastHit& Hit,
		ShipComponent*& HitComp,
		PhysGameObject*& HitActor,
		bool RaycastAll = false, 
		bool bCheckVehicle = true, 
		bool bCheckAircraft = true);

	/**
	 * 使用Sweep检测碰撞
	 * 返回TRUE，发生碰撞；FALSE,没有发生碰撞
	 */
	bool SweepObject(const NxVec3 & StartPos,const NxVec3 &EndPos);	
	
	enum TMStep
	{
		enIdle,//标准阶段，抛物线
		enOnWater//在水面上，直线
	};


	PhysGameObject*		_Sponsor;
	int					_SponsorAttachId;			//	发射部件的AttachId
	float				_IntervalSimulatedTime;		//	step time of raycast
	float				_ElapsedSimulatedTime;
	float				_TotalFlightSimulatedTime;
	float				_ElapsedFlightTime;
	NxVec3				_CurFlightLoc;
	NxVec3				_TargetPos;
	EHitState			_HitState;
	NxVec3				_HitWaterPose;
	float				_OnWaterTime;				//	在水面上行驶的时间，安全时间从这个时候开始算；
	TMStep				_Step;
	bool				_bForceSelfDestruction;		//	force self-destruction

	bool				_bUtilityTest;				//	工具类测试标志
	bool				_bVehicleTest;				//	舰船检测标志
	NxVec3				_HitPos;					//	工具类测试使用
	float				_TestSimulatedTime;			//

	PhysGameObject*		_FirstValidHitActor;				//	第一个碰到的物理对象

	/************************************************************************/
	/* 磁感引信鱼雷扩展														*/
	/************************************************************************/
	typedef std::vector<MagneticTargetStatus>	MagneticTestResult;
	MagneticTestResult	_ValidMagneticTarget;		//	有效磁感应目标
	float				_MagneticTestInterval;		//	磁感检测时间间隔
	float				_MagneticTestTime;			//	磁感应检测计时
};	

}

#endif