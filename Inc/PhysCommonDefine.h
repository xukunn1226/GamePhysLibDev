#ifndef _PHYS_COMMON_DEFINE_H_
#define _PHYS_COMMON_DEFINE_H_

#define GLOBAL_DATA_MULTIPLICATOR	(0.0001f)		// 全局配置数据倍率数值，仅针对有浮点要求的数值需要换算
													// 静态配置数据和服务端下发数据中，浮点值均转换成S32形式，故需转换后使用
#define GLOBAL_DATA_MULTIPLICATOR_TIME	(0.001f)	// 关于时间的倍率数值，配置数值单位是毫秒

namespace GPL
{

const double DegToUnrRot = 182.0444;
const double UnrRotToDeg = 0.00549316540360483;
const double UnrRotToRad = 0.00009587379924285;			// Pi / 32768
const double RadToUnrRot = 10430.3783504704527;			// 32768 / Pi
const double GPL_RadToDeg = 57.2957795130823216;		// 180 / Pi
const double GPL_DegToRad = 0.017453292519943296;		// Pi / 180

// physic and gpl scale conversion
const float P2GScale = 50.0f;
const float G2PScale = 0.02f;


enum ERunMode
{
	ERM_Client = 0,
	ERM_Server = 1,
	ERM_NumMode = 2,
};

enum EShapeType
{
	EST_SHAPE_BOX,
	EST_SHAPE_PLANE,
	EST_SHAPE_CAPSULE,
	EST_SHAPE_CONVEX,
	EST_SHAPE_SPHERE,
	EST_SHAPE_HEIGHTFIELD,
	EST_SHAPE_COUNT,
};

// 组件配置类型，读取配置数据时根据此类型实例化不同对象
enum EConfigType
{
	ECT_Invalid,
	ECT_Default,				// 
	ECT_Attached_Comp,			// 舰桥、发动机
	ECT_Attached_Weapon,		// 武器
	ECT_Vehicle,				// 舰船
	ECT_Projectile,				// 炮弹
	ECT_Static,					// 场景静态物体
	ECT_Aircraft,				// 舰载机
	ECT_Ocean,
	ECT_Terrain,
	ECT_BlockingBrush,
	ECT_TriggerBrush,
	ECT_PlayerStart,
	ECT_PlayerRespawn,
	ECT_NumType
};

// 定义场景内对象类型，不同类型对应着不同的创建流程（创建实例是NxActor或NxShape）
// warning: 配置数据，不可删除、改变枚举顺序，只可往后增加新类型
enum EGameObjectType
{
	EGOT_Invalid,
	EGOT_Ocean,								// PhysOcean;
	EGOT_Terrain,							// PhysTerrain
	EGOT_Static_Object,						// PhysStatic
	EGOT_BlockingBrush,
	EGOT_Vehicle_Ship,						// PhysVehicle
	EGOT_Vehicle_Aircraft,
	EGOT_Projectile,						// PhysProjectile
	EGOT_Projectile_MachineGun,				// Not used
	EGOT_Attached_Comp,						// ShipComponent
	EGOT_Attached_Weapon,					// ShipComponent
	EGOT_TriggerBrush,
	EGOT_PlayerStart,
	EGOT_PlayerRespawn,						// 重生点
	EGOT_NumType
};

enum ELoadingFlag
{
	ELF_None = 0,
	ELF_ClientSkip,
	ELF_ServerSkip,
};

enum ESimulationType
{
	EST_Invalid,
	EST_Normal,				// WASD
	EST_Normal_Cruise,		// WASD + 巡航
	EST_Normal_Cruise_Submarine,
	EST_Death,
	EST_NumType,
};

enum ESocketType
{
	EST_Socket_Invalid,		// 无效值
	EST_Socket_Part,		// 定位部件
	EST_Socket_ViewPoint,	// 观察点
	EST_Socket_CheckPoint,	// 检查点
	EST_Socket_StartPoint,	// 出生点
	EST_Socket_LeavePoint,	// 起飞离舰点
	EST_Socket_CatchPoint,	// 降落着舰点
	EST_Socket_BurstPoint,//爆炸距离检测点
};

// 可被载具装备的组件类型
enum EPartType
{
	EPT_Invalid,					// 无效位
	EPT_Bridge,						// 舰桥
	EPT_Engine,						// 引擎
	EPT_Deck,						// 甲板
	EPT_Sideboard,					// 侧舷
	EPT_Rudder,						// 船舵
	EPT_Magazine,					// 弹药舱
	EPT_Fuel,						// 燃料舱
	EPT_Watertight,					// 水密舱
	EPT_MainArtillery,				// 主火炮
	EPT_SubArtillery,				// 副火炮
	EPT_Torpedo,					// 鱼雷
	EPT_DepthCharge,				// 深水炸弹
	EPT_AntiAircraftArtillery,		// 防空炮
	EPT_AntiAircraftGun,			// 防空机枪
	EPT_Scout,						// 侦察机
	EPT_Fight,						// 歼击机
	EPT_TorpedoPlane,				// 鱼雷机
	EPT_Bomber,						// 轰炸机
	EPT_Observation,				// 观测设备
	EPT_Communication,				// 通讯设备
	EPT_Sonar,						// 声纳
	EPT_Armor,						// 装甲
	EPT_TorpedoBack,				// 后侧鱼雷
	EPT_FunctionalShellLauncher,    // 功能弹发射器
	EPT_SearchlightLauncher,		// 探照灯
	EPT_NumType
};

// 炮弹轨迹
enum ETrajectoryMethod
{
	ETM_Invalid,
	ETM_Parabola,		// 抛物线
	ETM_Straight,		// 直线
	ETM_SectionalH,		// 先抛物线再水平直线
	ETM_SectionalV,		// 先抛物线再竖直直线
	ETM_Count
};

//	舰载机类型
enum EAircraftType
{
	EAT_Scout,		//	侦察机
	EAT_Bomber,		//	轰炸机
	EAT_Torpedor,	//	鱼雷机
	EAT_Fighter,	//	战斗机
};

//	舰载机状态
enum AircraftAction
{
	Ready = 0,				//	待机
	TakeOff,				//	起飞
	Landing,				//	降落
	Transfer,				//	移动
	Circle,					//	盘旋
	Attack,					//	攻击
	Crash,					//	坠机

	ActionNum
};

// PlayerStart容量
const unsigned char PLAYER_START_SIZE_MAX = 128;

// PlayerRespawn最大容量
const unsigned char PLAYER_RESPAWN_SIZE_MAX = 128;

// 最大检测点数量
const char VEHICLE_CHECK_POINT_MAX = 4;

// 最大观察点数量
const char VEHICLE_VIEW_POINT_MAX = 1;

// 最大爆炸检查点数量
const char VEHICLE_BURST_POINT_MAX = 10;

// 每条检查线段最多检查的阻挡物
const char MAX_BLOCK_OBJECT_PER_LINE = 5;

// 移动控制标识
const short MoveFlag_None			=	0x0000;
const short MoveFlag_Forward		=	0x0001;
const short MoveFlag_Backward		=	0x0002;
const short MoveFlag_RightTurn		=	0x0004;
const short MoveFlag_LeftTurn		=	0x0008;
const short MoveFlag_CruiseDriving	=	0x0010;
const short MoveFlag_CruiseSteering	=	0x0020;
const short MoveFlag_Riseup			=	0x0040;
const short MoveFlag_Sink			=	0x0080;
const short MoveFlag_AllOp = MoveFlag_Forward | MoveFlag_Backward | MoveFlag_RightTurn | MoveFlag_LeftTurn | MoveFlag_CruiseDriving | MoveFlag_CruiseSteering | MoveFlag_Riseup | MoveFlag_Sink;
typedef short EMoveFlag;


#define GPL_KINDA_SMALL_NUMBER	(1.e-2f)

#define GPL_PHYS_SMALL_NUMBER	(1.e-8f)

// actor.group
#define GPL_GROUP_DEFAULT							(0)			// 默认不产生contact information
#define	GPL_GROUP_NOTIFYCOLLIDE						(1)			// generate contact, e.g. NX_NOTIFY_ON_START_TOUCH

// shape.group
#define GPL_SHAPE_GROUP_DEFAULT							(0)			// 默认与其他组均产生碰撞
#define GPL_SHAPE_GROUP_OCEAN							(1)
#define GPL_SHAPE_GROUP_TERRAIN							(2)
#define GPL_SHAPE_GROUP_STATICOBJECT					(3)
#define GPL_SHAPE_GROUP_BLOCKING_VOLUME					(4)			// 阻挡舰船，阻挡CAMERA，忽略PROJECTILE
#define GPL_SHAPE_GROUP_BLOCKING_VOLUME_IGNORE_CAMERA	(5)			// 阻挡舰船，忽略CAMERA，忽略PROJECTILE
#define GPL_SHAPE_GROUP_TRIGGER_VOLUME					(6)
#define GPL_SHAPE_GROUP_VEHICLE_SHIP					(7)
#define GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME			(8)			// 舰船的Trigger Volume
#define GPL_SHAPE_GROUP_AIRCRAFT						(9)
#define GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME			(10)		//	只对舰载机产生判定
#define GPL_SHAPE_GROUP_SINK_VEHICLE					(11)		//	沉船类型碰撞，仅检测地形
#define GPL_SHAPE_GROUP_SUBMARINE_SURFACE_VOLUME		(12)		//	潜艇水面虚像物理轮廓
#define GPL_SHAPE_GROUP_FLARE_VOLUME					(13)		//	检测飞机和船（以及地形）

}

#endif