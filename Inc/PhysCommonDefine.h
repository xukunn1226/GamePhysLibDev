#ifndef _PHYS_COMMON_DEFINE_H_
#define _PHYS_COMMON_DEFINE_H_

#define GLOBAL_DATA_MULTIPLICATOR	(0.0001f)		// ȫ���������ݱ�����ֵ��������и���Ҫ�����ֵ��Ҫ����
													// ��̬�������ݺͷ�����·������У�����ֵ��ת����S32��ʽ������ת����ʹ��
#define GLOBAL_DATA_MULTIPLICATOR_TIME	(0.001f)	// ����ʱ��ı�����ֵ��������ֵ��λ�Ǻ���

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

// ����������ͣ���ȡ��������ʱ���ݴ�����ʵ������ͬ����
enum EConfigType
{
	ECT_Invalid,
	ECT_Default,				// 
	ECT_Attached_Comp,			// ���š�������
	ECT_Attached_Weapon,		// ����
	ECT_Vehicle,				// ����
	ECT_Projectile,				// �ڵ�
	ECT_Static,					// ������̬����
	ECT_Aircraft,				// ���ػ�
	ECT_Ocean,
	ECT_Terrain,
	ECT_BlockingBrush,
	ECT_TriggerBrush,
	ECT_PlayerStart,
	ECT_PlayerRespawn,
	ECT_NumType
};

// ���峡���ڶ������ͣ���ͬ���Ͷ�Ӧ�Ų�ͬ�Ĵ������̣�����ʵ����NxActor��NxShape��
// warning: �������ݣ�����ɾ�����ı�ö��˳��ֻ����������������
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
	EGOT_PlayerRespawn,						// ������
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
	EST_Normal_Cruise,		// WASD + Ѳ��
	EST_Normal_Cruise_Submarine,
	EST_Death,
	EST_NumType,
};

enum ESocketType
{
	EST_Socket_Invalid,		// ��Чֵ
	EST_Socket_Part,		// ��λ����
	EST_Socket_ViewPoint,	// �۲��
	EST_Socket_CheckPoint,	// ����
	EST_Socket_StartPoint,	// ������
	EST_Socket_LeavePoint,	// ����뽢��
	EST_Socket_CatchPoint,	// �����Ž���
	EST_Socket_BurstPoint,//��ը�������
};

// �ɱ��ؾ�װ�����������
enum EPartType
{
	EPT_Invalid,					// ��Чλ
	EPT_Bridge,						// ����
	EPT_Engine,						// ����
	EPT_Deck,						// �װ�
	EPT_Sideboard,					// ����
	EPT_Rudder,						// ����
	EPT_Magazine,					// ��ҩ��
	EPT_Fuel,						// ȼ�ϲ�
	EPT_Watertight,					// ˮ�ܲ�
	EPT_MainArtillery,				// ������
	EPT_SubArtillery,				// ������
	EPT_Torpedo,					// ����
	EPT_DepthCharge,				// ��ˮը��
	EPT_AntiAircraftArtillery,		// ������
	EPT_AntiAircraftGun,			// ���ջ�ǹ
	EPT_Scout,						// ����
	EPT_Fight,						// �߻���
	EPT_TorpedoPlane,				// ���׻�
	EPT_Bomber,						// ��ը��
	EPT_Observation,				// �۲��豸
	EPT_Communication,				// ͨѶ�豸
	EPT_Sonar,						// ����
	EPT_Armor,						// װ��
	EPT_TorpedoBack,				// �������
	EPT_FunctionalShellLauncher,    // ���ܵ�������
	EPT_SearchlightLauncher,		// ̽�յ�
	EPT_NumType
};

// �ڵ��켣
enum ETrajectoryMethod
{
	ETM_Invalid,
	ETM_Parabola,		// ������
	ETM_Straight,		// ֱ��
	ETM_SectionalH,		// ����������ˮƽֱ��
	ETM_SectionalV,		// ������������ֱֱ��
	ETM_Count
};

//	���ػ�����
enum EAircraftType
{
	EAT_Scout,		//	����
	EAT_Bomber,		//	��ը��
	EAT_Torpedor,	//	���׻�
	EAT_Fighter,	//	ս����
};

//	���ػ�״̬
enum AircraftAction
{
	Ready = 0,				//	����
	TakeOff,				//	���
	Landing,				//	����
	Transfer,				//	�ƶ�
	Circle,					//	����
	Attack,					//	����
	Crash,					//	׹��

	ActionNum
};

// PlayerStart����
const unsigned char PLAYER_START_SIZE_MAX = 128;

// PlayerRespawn�������
const unsigned char PLAYER_RESPAWN_SIZE_MAX = 128;

// ����������
const char VEHICLE_CHECK_POINT_MAX = 4;

// ���۲������
const char VEHICLE_VIEW_POINT_MAX = 1;

// ���ը��������
const char VEHICLE_BURST_POINT_MAX = 10;

// ÿ������߶��������赲��
const char MAX_BLOCK_OBJECT_PER_LINE = 5;

// �ƶ����Ʊ�ʶ
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
#define GPL_GROUP_DEFAULT							(0)			// Ĭ�ϲ�����contact information
#define	GPL_GROUP_NOTIFYCOLLIDE						(1)			// generate contact, e.g. NX_NOTIFY_ON_START_TOUCH

// shape.group
#define GPL_SHAPE_GROUP_DEFAULT							(0)			// Ĭ�����������������ײ
#define GPL_SHAPE_GROUP_OCEAN							(1)
#define GPL_SHAPE_GROUP_TERRAIN							(2)
#define GPL_SHAPE_GROUP_STATICOBJECT					(3)
#define GPL_SHAPE_GROUP_BLOCKING_VOLUME					(4)			// �赲�������赲CAMERA������PROJECTILE
#define GPL_SHAPE_GROUP_BLOCKING_VOLUME_IGNORE_CAMERA	(5)			// �赲����������CAMERA������PROJECTILE
#define GPL_SHAPE_GROUP_TRIGGER_VOLUME					(6)
#define GPL_SHAPE_GROUP_VEHICLE_SHIP					(7)
#define GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME			(8)			// ������Trigger Volume
#define GPL_SHAPE_GROUP_AIRCRAFT						(9)
#define GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME			(10)		//	ֻ�Խ��ػ������ж�
#define GPL_SHAPE_GROUP_SINK_VEHICLE					(11)		//	����������ײ����������
#define GPL_SHAPE_GROUP_SUBMARINE_SURFACE_VOLUME		(12)		//	Ǳͧˮ��������������
#define GPL_SHAPE_GROUP_FLARE_VOLUME					(13)		//	���ɻ��ʹ����Լ����Σ�

}

#endif