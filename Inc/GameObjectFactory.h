#ifndef _GAMEOBJECT_FACTORY_H_
#define _GAMEOBJECT_FACTORY_H_

namespace GPL
{
struct UAttachedCompData
{
	int			_CompId;
	EPartType	_PartType;

	// 武器属性
	int			_FirePartNum;
	float		_InitVelocity;					// 武器炮弹初速度
	float		_TurretAngleVelocity;			// degree/sec, 炮台转动角速度
	float		_Gravity;						// 射出的炮弹所受重力加速度，仅作用于抛物线炮弹
	float		_AccuracyDefault;				// 默认射击散射半径
	float		_AccuracyTurrent;				//炮台转向对射击精确度的影响
	float		_AccuracyShipDrive;				//舰船行驶对射击精确度的影响
	float		_AccuracyShipSteer;				//舰船转向对射击精确度的影响
	float		_AccuracyRecover;				//射击精确度每秒恢复值
	float		_AccuracyFire;					//开炮对射击精确度的影响
	float		_AccuracyContinuousFire;		//持续射击精准度上线降低的比例	
	float		_ArtilleryDiffuseParam;			//火炮散射参数

	UAttachedCompData()		
	{
		_FirePartNum = 0;
		_InitVelocity = 0.f;
		_Gravity = 0.f;
		_TurretAngleVelocity = 10.f;
		_AccuracyDefault=10.0f;
		_AccuracyTurrent=300.0f;
		_AccuracyShipDrive =1000.0f;				
		_AccuracyShipSteer =1000.0f;
		_AccuracyRecover=30.0f;
		_AccuracyFire=1000.0f;
		_AccuracyContinuousFire=1.0f;
		_ArtilleryDiffuseParam=1.0f;
	}
};

struct UShipDesc
{
	UShipDesc()
	{
		_CompId = 0;
		_InitGlobalPose.id();
		_LinearVelocity.zero();
		_AttachedCompList.clear();
		_MaxForwardSpeed = 0.f;
		_MaxForwardForce = 0.f;
		_TurnTorque = 0.f;
		_MaxRiseSinkForce = 0.f;
		_Mass = 0.f;
		_CMassOffsetX = 0.f;
		_Length = 0.f;
		_Width = 0.f;
		_Height = 0.f;
	}

	int					_CompId;
	NxMat34				_InitGlobalPose;				// 载具初始化位置
	NxVec3				_LinearVelocity;				// 线速度
	float				_MaxForwardSpeed;				// 最大前进档速度
	float				_MaxForwardForce;				// 前进时力的大小
	float				_TurnTorque;					// 转向扭矩
	float				_MaxRiseSinkForce;				// 上浮下潜FORCE大小
	float				_Mass;							// 质量
	float				_CMassOffsetX;					// 质心在X轴方向的OFFSET
	float				_Length;						// 船长
	float				_Width;							// 船宽
	float				_Height;						// 船高

	std::vector<UAttachedCompData>	_AttachedCompList;	// 装备的部件ID List
};

struct UAttachedCompDataEx
{
	int			_CompId;
	int			_AttachId;

	// 武器属性
	int			_FirePartNum;
	float		_InitVelocity;					// 武器炮弹初速度
	float		_TurretAngleVelocity;			// degree/sec, 炮台转动角速度
	float		_Gravity;						// 射出的炮弹所受重力加速度，仅作用于抛物线炮弹
	float		_AccuracyDefault;				// 默认射击散射半径
	float		_AccuracyTurrent;				//炮台转向对射击精确度的影响
	float		_AccuracyShipDrive;				//舰船行驶对射击精确度的影响
	float		_AccuracyShipSteer;				//舰船转向对射击精确度的影响
	float		_AccuracyRecover;				//射击精确度每秒恢复值
	float		_AccuracyFire;					//开炮对射击精确度的影响
	float		_AccuracyContinuousFire;		//持续射击精准度上线降低的比例	
	float		_ArtilleryDiffuseParam;			//火炮散射参数

	UAttachedCompDataEx()		
	{
		_FirePartNum = 0;
		_InitVelocity = 0.f;
		_Gravity = 0.f;
		_TurretAngleVelocity = 10.f;
		_AccuracyDefault=10.0f;
		_AccuracyTurrent=300.0f;
		_AccuracyShipDrive =1000.0f;				
		_AccuracyShipSteer =1000.0f;
		_AccuracyRecover=30.0f;
		_AccuracyFire=1000.0f;
		_AccuracyContinuousFire=1.0f;
		_ArtilleryDiffuseParam=1.0f;
	}
};
struct UShipDescEx
{
	UShipDescEx()
	{
		_CompId = 0;
		_InitGlobalPose.id();
		_LinearVelocity.zero();
		_AttachedCompList.clear();
		_MaxForwardSpeed = 0.f;
		_MaxForwardForce = 0.f;
		_TurnTorque = 0.f;
		_MaxRiseSinkForce = 0.f;
		_Mass = 0.f;
		_CMassOffsetX = 0.f;
		_Length = 0.f;
		_Width = 0.f;
		_Height = 0.f;
	}

	int					_CompId;
	NxMat34				_InitGlobalPose;				// 载具初始化位置
	NxVec3				_LinearVelocity;				// 线速度
	float				_MaxForwardSpeed;				// 最大前进档速度
	float				_MaxForwardForce;				// 前进时力的大小
	float				_TurnTorque;					// 转向扭矩
	float				_MaxRiseSinkForce;				// 上浮下潜FORCE大小
	float				_Mass;							// 质量
	float				_CMassOffsetX;					// 质心在X轴方向的OFFSET
	float				_Length;						// 船长
	float				_Width;							// 船宽
	float				_Height;						// 船高

	std::vector<UAttachedCompDataEx>	_AttachedCompList;	// 装备的部件ID List
};

//	@brief	舰载机数据
//	@note	经过加成计算后由服务器下发
struct AircraftDesc
{
	int					_CompId;
	int					_WeaponPartType;		//	舰载机类型	
	float				_Speed;					//	航速
	float				_Consume;				//	油耗
	float				_MinRadius;				//	最小转弯半径
	float				_SensorRange;			//	侦查范围
	float				_InterceptRange;		//	拦截范围
	float				_AttackExtent;			//	攻击范围半径
	float				_AttackScope;			//	攻击范围夹角
	float				_ChaseRange;			//	追击范围
	float				_Gravity;				//	炮弹重力
	float				_InitPrecision;			//	初始投弹精度
	float				_Precision;				//	最准投弹精度（百分比）
	float				_Collimation;			//	缩圈速度（百分比）
	float				_TurningAffect;			//	转向精度影响（百分比）
	float				_LayerAffect;			//	换层精度影响（百分比）
	float				_AscendAngle;			//	爬升仰角
	float				_SwoopAngle;			//	俯冲倾角
	float				_ExLowFlyingTime;		//	贴海飞行时间
	float				_LayerChangeSpeed;		//	换层速度
	float				_EchoInterval;			//	反应时间
	float				_DiveAcceleration;		//	俯冲加速度
	float				_PullAcceleration;		//	拉升加速度
	float				_DiffuseEccentricity;	//	投弹散布离心率

	AircraftDesc()
	{
		_CompId = 0;
		_Speed = 0.f;
		_Consume = 0.f;
		_MinRadius = 0.f;
		_SensorRange = 0.f;
		_InterceptRange = 0.f;
		_ChaseRange = 0.f;
		_DiffuseEccentricity = 0.6f;
	}
};

struct ProjectileDesc
{
	int					_CompId;
	NxVec3				_StartPos;
	NxVec3				_Gravity;
	NxVec3				_InitVelocity;
	ETrajectoryMethod	_TrajectoryMethod;
	float				_SafeTime;				//安全时间，若发生碰撞则炮弹消失，单位：秒
	float				_BurstTime;				//超过该时间就爆炸，单位：秒
	float				_BurstRange;			//引爆后计算伤害的范围
	NxVec3				_BrakingVelocity;		//自身制动速度,仅限于鱼雷、深水炸弹与水面运行时的速度，抛射速度由武器决定
	NxVec3				_TargetPos;				// 射击目标点，因精准度逻辑不一定命中

	ProjectileDesc()
	{
		_CompId = 0;
		_StartPos.zero();
		_Gravity.zero();
		_InitVelocity.zero();
		_TargetPos.zero();
		_TrajectoryMethod = ETM_Parabola;
		_SafeTime = 0.f;
		_BurstTime = 0.f;
		_BurstRange = 0.f;
		_BrakingVelocity.zero();
	}
};

struct UProjectileDesc
{
	int					_CompId;
	NxVec3				_StartPos;
	NxVec3				_Gravity;
	NxVec3				_InitVelocity;
	ETrajectoryMethod	_TrajectoryMethod;
	float				_SafeTime;				//	安全时间，若发生碰撞则炮弹消失，单位：秒
	float				_BurstTime;				//	超过该时间就爆炸，单位：秒
	float				_BurstRange;			//	引爆后计算伤害的范围
	NxVec3				_BrakingVelocity;		//	自身制动速度,仅限于鱼雷、深水炸弹与水面运行时的速度，抛射速度由武器决定
	NxVec3				_TargetPos;				//	射击目标点，因精准度逻辑不一定命中
	float				_MagneticSensorRange;	//	磁感引信检测范围

	UProjectileDesc()
	{
		_CompId = 0;
		_StartPos.zero();
		_Gravity.zero();
		_InitVelocity.zero();
		_TargetPos.zero();
		_TrajectoryMethod = ETM_Parabola;
		_SafeTime = 0.f;
		_BurstTime = 0.f;
		_BurstRange = 0.f;
		_BrakingVelocity.zero();
	}
};

struct UTriggerDesc
{
	int					_CompId;
	float				_Radius;
	float				_Extends;
	NxMat34				_InitGlobalPose;				// 初始化位置
	bool				_Associated;
	bool				_CheckVehicle;

	UTriggerDesc()
	{
		_CompId = 0;
		_Radius = 0;
		_Extends = 0;
		_Associated = false;
		_CheckVehicle = false;
		_InitGlobalPose.id();
	}
};

// 外部调用接口
PhysVehicle* UCreateShip(PhysGameScene* PhysScene, UShipDesc& Desc);
PhysVehicle* UCreateShipEx(PhysGameScene* PhysScene, UShipDescEx& Desc);
PhysAircraft* CreateAircraft(PhysGameScene* PhysScene, AircraftDesc& Desc);
PhysProjectile* UCreateProjectile(PhysGameScene* PhysScene, PhysGameObject* Sponsor, int AttachId, UProjectileDesc& ProjDesc);
PhysTrigger* UCreateTrigger(PhysGameScene* PhysScene, UTriggerDesc& Desc);

PhysGameObject* CreatePhysGameObject(PhysGameScene* Scene, EGameObjectType Type, ComponentData* ObjectData);

void DestroyPhysGameObject(class PhysGameScene* Scene, PhysGameObject* GameObject);

// 内部调用接口
NxShapeDesc* CreateComponentShape(const ShapeDesc* const CustomShapeDesc, NxVec3 shapeScale = NxVec3(1.0f,1.0f,1.0f));
void ReleaseShapeDescList(std::vector<NxShapeDesc*>& CachedShapeDescList);
PhysProjectile* CreateProjectile(PhysGameScene* PhysScene, PhysGameObject* Sponsor, int AttachId, ProjectileDesc& ProjDesc);
PhysTrigger* CreateTrigger(PhysGameScene* PhysScene, UTriggerDesc& Desc);


}

#endif