//=============================================================================
// 物理库中除了配置数据外，所有内部接口使用的数据和提供给外部的数据都已转换至Phys Scale制式下
//=============================================================================
#ifndef _PHYS_PUBLIC_H_
#define _PHYS_PUBLIC_H_

namespace GPL
{ // begin namespace

#if SHIPPING_PC_GAME
#define gplDebugf		__noop
#define gplDebugfANSI	__noop
#else
#define gplDebugf		gplPrintf
#define gplDebugfANSI	gplPrintfANSI
#endif

class PhysGameScene;
class PhysGameObject;
class PhysGameObject_Projectile;
class VehicleSimBase;
class VehicleSimShip;
class PhysVehicle;
class ShipComponent;
class GameObject;


// 物理对象碰撞时的数据结构
enum ECollisionEvent
{
	ENE_On_Start_Touch			= (1<<1),
	ENE_On_End_Touch			= (1<<2),
	ENE_On_Touch				= (1<<3),
	ENE_Out_Of_World			= (1<<4),
	ENE_Self_Destruction		= (1<<5)
};

struct CollisionContactInfo
{
	ECollisionEvent	_Event;
	NxVec3			_ContactPosition;
	NxVec3			_ContactNormal;
	NxVec3			_ContactVelocity[2];

	void SwapOrder()
	{
		NxVec3 Temp;
		Temp = _ContactVelocity[1];
		_ContactVelocity[1] = _ContactVelocity[0];
		_ContactVelocity[0] = Temp;

		_ContactNormal *= -1.f;
	}
};

struct CollisionImpactData
{
	NxVec3		_TotalNormalForceVector;		// The total contact normal force that was applied for this pair
	NxVec3		_TotalFrictionForceVector;		// The total tangential force that was applied for this pair
	std::vector<CollisionContactInfo>	_ContactInfos;

	void SwapContactOrder()
	{
		for(int i = 0; i < (int)_ContactInfos.size(); ++i)
		{
			_ContactInfos[i].SwapOrder();
		}
	}
};

// 碰撞数据
struct CollisionNotifyInfo
{
	bool	_bValidActor0;
	bool	_bValidActor1;

	PhysGameObject*		_Actor0;
	PhysGameObject*		_Actor1;
	
	CollisionImpactData	_CollisionData;

	CollisionNotifyInfo()
		: _bValidActor0(false), _bValidActor1(false), _Actor0(NULL), _Actor1(NULL)
	{}
};

//	@brief	射击参数
struct FiringParam
{
	NxVec3		StartPos;
	NxVec3		StartDir;			// 发射方向
	NxVec3		InitVelocity;		// 初速度
	int			PartID;				// 炮管ID
	float		DeltaYaw;			// 当前朝向与目标朝向的夹角（度数）
};

typedef std::vector<FiringParam>	FirePartInfo;

//	@brief	鱼雷射击参数
struct TorpedoFireParam
{
	int				AttachId;
	FirePartInfo	FiredParts;
};

typedef std::vector<TorpedoFireParam>	TorpedoFireInfo;

struct DamageInfo
{
	NxVec3	_HitPos;		// 命中位置
	float	_Distance;		// 爆炸点离舰船的距离
	bool	_SafeTime;		// 是否在安全时间内发生的事件
	float	_Angle;			// 子弹和打中点的面的夹角
	bool	_DirectHit;		// 是否直接命中
	GameObject* _Obj;		// 受影响的物体（ShipComponent OR PhysGameObject）
};

struct HitNotifyInfo
{
	NxVec3	_HitPos;			// 命中位置
	NxVec3	_HitNormal;			// 命中的shape法线方向
	NxVec3	_HitDirection;		// 命中时炮弹的入射方向

	HitNotifyInfo()
	{
		_HitPos = NxVec3(0.f);
		_HitNormal = NxVec3(0.f);
		_HitDirection = NxVec3(0.f);
	}
};

// 不同速度区间控制不同的阻尼
struct DampingFragment
{
	float	Speed;
	float	Damping;
};

struct ReduceLevel
{
	float	SpeedRate;
	float	ReduceFactor;
};

struct PhysGameSetting
{
	float	_FirePointLastTime;								//	射击点持续时间
	bool	_RecordAimingOccasion;							//	射击点的记录方式 true 是开火时候记录 false 是 命中后记录
	float	_AccuracyFormulaA;								//	计算公式中A的值
	float	_AccuracyFormulaB;								//	计算公式中B的值
	float	_AccuracyFormulaC;								//	计算公式中C的值
	float	_AccuracyFormulaD;								//	计算公式中D的值
	float	_AccuracyLimitMin;								//	精准度的下限影响的最小值

	float	_GPL_PhysWorld_HalfLength;						//	物理场景半长
	float	_GPL_PhysWorld_HalfWidth;						//	物理场景半宽
	float	_GPL_PhysWorld_HalfHeight;						//	物理场景半高

	float	_GPL_MAX_HALF_WORLD;							//	场景最大半长
	float	_GPL_SKIN_WIDTH;
	float	_GPL_PHYS_GRAVITY;								//	美术制作以分米为制式，故UE3层Gravity=100分米/秒
	float	_GPL_PHYS_CLIENT_MAX_TIMESTEP;					//	客户端物理模拟固定步长
	float	_GPL_PHYS_SERVER_MAX_TIMESTEP;					//	服务端物理模拟固定步长
	int		_GPL_PROJECTILE_TICK_COUNT;						//	炮弹飞行时模拟的次数，若取值NX_MAX_I32则以GPL_PHYS_CLIENT_MAX_TIMESTEP或GPL_PHYS_SERVER_MAX_TIMESTEP运行
	float	_GPL_PROJECTILE_SIMULATED_TIME_MAX;				//	距上次模拟的最大间隔时间

	float	_GPL_FIRE_AVAILABLE_TOLERANCE;					//	in degree，武器开火时允许的角度偏差值(_CurrentYaw - _DesiredYaw)

	float	_GPL_WaterLayerOneHeight;						//	水下一层深度
	float	_GPL_WaterLayerTwoHeight;						//	水下二层深度

	float	_GPL_SubmarineAccelerate;						//	潜水艇加速阶段比例
	float	_GPL_SubmarineConstant;							//	潜水艇匀速阶段比例

	int		_GPL_FlightLayer_CruiseHeight;					//	飞行空域 - 巡航高度
	int		_GPL_FlightLayer_DiveLimit;						//	飞行空域 - 俯冲极限高度
	int		_GPL_FlightLayer_TorpedoRelease;				//	飞行空域 - 鱼雷投放高度

	float	_GPL_ReverseForce_DampingFactor;				//	从前进状态到倒退状态时的阻尼系数
	float	_GPL_ForwardForce_DampingFactor;				//	从倒退状态到前进状态时的阻尼系数
	float	_GPL_ReverseSpeed_Rate;							//	后退最大速度相对前进速度的比例
	float	_GPL_ReverseForce_Rate;							//	后退最大力相对前进力的比例
	float	_GPL_TorqueDampingFactor;						//	扭矩阻尼系数，仅影响角加速度的大小
	float	_GPL_FactorOfAngularVelocity;					//	影响角速度因子
	float	_GPL_Aircraft_TakeOff_Angle;					//	起飞拉升仰角
	float	_GPL_Aircraft_Landing_Angle;					//	降落倾角

	float	_GPL_PickTrace_Range;							//	拾取半径
	bool	_GPL_Aircraft_Process_Blocks;					//	舰载机避障开关
	float	_GPL_Aircraft_Block_Detect_Interval;			//	障碍检测时间间隔
	int		_GPL_Aircraft_ActionCache_Length;				//	舰载机动作缓存长度
	float	_Aircraft_Interpose_Speed;						//	舰载机显示对象插值速度
	float	_PhysAircraft_PositionInterp_Speed;				//	舰载机物理对象插值速度
	float	_PhysAircraft_RotationInterp_Speed;				//	舰载机物理对象插值速度

	float	_GPL_Sink_Roll_Limit;							//	沉没倾角限制
	float	_GPL_Sink_Pitch_Limit;							//	沉没仰角限制
	float	_GPL_Sink_Speed_Limit;							//	沉没速度限制
	float	_GPL_Sink_ForceFactor_LowerLimit;				//	沉没拉力系数下限
	float	_GPL_Sink_ForceFactor_UpperLimit;				//	沉没拉力系数上限
	float	_GPL_Sink_TorqueFactor_LowerLimit;				//	沉没扭矩系数下限
	float	_GPL_Sink_TorqueFactor_UpperLimit;				//	沉没扭矩系数上限
	float	_GPL_Sink_DampingFactor;						//	沉没阻尼系数
	float	_GPL_Sink_Torque_DampingFactor;					//	沉没扭矩阻尼系数

	float	_GPL_TorpedoSalvo_DeltaTangentVelocity;			//  鱼雷齐射参数  切向速度差
	float	_GPL_TorpedoSalvo_DeltaAngle;					//  鱼雷齐射参数  径向夹角

	float	_GPL_Bomber_ReCollimate_PrecisionFactorA;		//	轰炸机重新指定目标点对投弹精度产生影响的参数A	
	float	_GPL_Bomber_ReCollimate_PrecisionFactorB;		//	轰炸机重新指定目标点对投弹精度产生影响的参数B

	float	_GPL_TorpedoPlane_LaunchTorlerance;				//	鱼雷机投弹容错距离
	float	_GPL_TorpedoPlane_LaunchAngleTorlerance;		//	鱼雷机投弹容错角度

	float	_GPL_Aircraft_Routing_Torlerance;				//	舰载机路径规划容错距离（半径）
	float	_GPL_Aircraft_Routing_AngleTorlerance;			//	舰载机路径规划容错角度

	float	_GPL_Aircraft_DiveAttack_SpeedFactor;			//	轰炸机俯冲速度因子
	float	_GPL_Aircraft_DiveAttack_LimitSpeed;			//	轰炸机俯冲速度限制
	float	_GPL_Aircraft_DiveAttack_MinInterval;			//	轰炸机最小俯冲时间间隔
	float	_GPL_Aircraft_DiveAttack_MaxInterval;			//	轰炸机最大俯冲时间间隔

	bool	_GPL_MagneticTorpedo_FriendlyDetective;			//	磁感应鱼雷是否检测友军

	std::vector<int>	_PartTypeListOfSupportContinuousFire;	// 支持越打越准逻辑的PartType列表

	std::vector<DampingFragment>	_DampingFragmentList;		// 各个速度区间的阻尼系数

	std::vector<ReduceLevel>		_AccuracyDrivingReduce;		// 舰船行驶造成的瞄准精度衰减

	bool FindDampingFragment(float InSpeed, DampingFragment& OutFragment)
	{
		int Index = -1;
		int Length = (int)_DampingFragmentList.size();
		for(int i = Length-1; i >= 0; --i)
		{
			if( _DampingFragmentList[i].Speed <= InSpeed )
			{
				Index = i;
				break;
			}
		}
		if( Index == -1 )
			return false;

		OutFragment = _DampingFragmentList[Index];
		return true;
	}

	PhysGameSetting()
	{
		_FirePointLastTime = 0.f;
		_RecordAimingOccasion = false;
		_AccuracyFormulaA = 0.f;
		_AccuracyFormulaB = 0.f;
		_AccuracyFormulaC = 0.f;
		_AccuracyFormulaD=0.f;
		_AccuracyLimitMin=0.f;
		_GPL_PhysWorld_HalfLength = 10000;
		_GPL_PhysWorld_HalfWidth = 10000;
		_GPL_PhysWorld_HalfHeight = 25000;
		_GPL_MAX_HALF_WORLD = 2000.f;
		_GPL_SKIN_WIDTH = 0.025f;
		_GPL_PHYS_GRAVITY = -100.f;
		_GPL_PHYS_CLIENT_MAX_TIMESTEP = 1.f/30.f;
		_GPL_PHYS_SERVER_MAX_TIMESTEP = 1.f/10.f;
		_GPL_PROJECTILE_TICK_COUNT = 6;
		_GPL_PROJECTILE_SIMULATED_TIME_MAX = 1.f;
		_GPL_FIRE_AVAILABLE_TOLERANCE = 1.f;
		_GPL_WaterLayerOneHeight=0;
		_GPL_WaterLayerTwoHeight=0;
		_GPL_SubmarineAccelerate=0.3;
		_GPL_SubmarineConstant=0.5;
		_GPL_FlightLayer_CruiseHeight = 3000;
		_GPL_FlightLayer_DiveLimit = 1500;
		_GPL_FlightLayer_TorpedoRelease = 500;
		_GPL_ReverseForce_DampingFactor = 1.f;
		_GPL_ForwardForce_DampingFactor = 1.f;
		_GPL_ReverseSpeed_Rate=0.5f;
		_GPL_ReverseForce_Rate=0.5f;
		_GPL_TorqueDampingFactor=0.8f;
		_GPL_FactorOfAngularVelocity=0.2f;
		_GPL_Aircraft_TakeOff_Angle=45.f;
		_GPL_Aircraft_Landing_Angle=30.f;
		_GPL_PickTrace_Range=50.f;
		_GPL_Aircraft_Process_Blocks = false;
		_GPL_Aircraft_Block_Detect_Interval = 2000;
		_GPL_Aircraft_ActionCache_Length = 10;
		_Aircraft_Interpose_Speed = 1.f;
		_PhysAircraft_PositionInterp_Speed = 1.f;
		_PhysAircraft_RotationInterp_Speed = 1.f;
		_GPL_Sink_Roll_Limit = 45.f;
		_GPL_Sink_Pitch_Limit = 45.f;
		_GPL_Sink_Speed_Limit = 1.f;
		_GPL_Sink_ForceFactor_LowerLimit = 0.1f;
		_GPL_Sink_ForceFactor_UpperLimit = 1;
		_GPL_Sink_TorqueFactor_LowerLimit = 0.5f;
		_GPL_Sink_TorqueFactor_UpperLimit = 1;
		_GPL_Sink_DampingFactor = 1;
		_GPL_Sink_Torque_DampingFactor = 1;
		_GPL_TorpedoSalvo_DeltaTangentVelocity = 0.f;
		_GPL_TorpedoSalvo_DeltaAngle = 0.f;
		_GPL_Bomber_ReCollimate_PrecisionFactorA = 0.f;
		_GPL_Bomber_ReCollimate_PrecisionFactorB = 0.f;
		_GPL_TorpedoPlane_LaunchTorlerance = 0.f;
		_GPL_TorpedoPlane_LaunchAngleTorlerance = 0.f;
		_GPL_Aircraft_Routing_Torlerance = 0.f;
		_GPL_Aircraft_Routing_AngleTorlerance = 0.f;
		_GPL_Aircraft_DiveAttack_SpeedFactor = 2.f;
		_GPL_Aircraft_DiveAttack_LimitSpeed = 100.f;
		_GPL_Aircraft_DiveAttack_MinInterval = 0.05f;
		_GPL_Aircraft_DiveAttack_MaxInterval = 0.2f;
		_GPL_MagneticTorpedo_FriendlyDetective = true;
		_PartTypeListOfSupportContinuousFire.clear();
		_DampingFragmentList.clear();
	}
};
} // end namespace

#endif