//=============================================================================
// ������г������������⣬�����ڲ��ӿ�ʹ�õ����ݺ��ṩ���ⲿ�����ݶ���ת����Phys Scale��ʽ��
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


// ���������ײʱ�����ݽṹ
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

// ��ײ����
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

//	@brief	�������
struct FiringParam
{
	NxVec3		StartPos;
	NxVec3		StartDir;			// ���䷽��
	NxVec3		InitVelocity;		// ���ٶ�
	int			PartID;				// �ڹ�ID
	float		DeltaYaw;			// ��ǰ������Ŀ�곯��ļнǣ�������
};

typedef std::vector<FiringParam>	FirePartInfo;

//	@brief	�����������
struct TorpedoFireParam
{
	int				AttachId;
	FirePartInfo	FiredParts;
};

typedef std::vector<TorpedoFireParam>	TorpedoFireInfo;

struct DamageInfo
{
	NxVec3	_HitPos;		// ����λ��
	float	_Distance;		// ��ը���뽢���ľ���
	bool	_SafeTime;		// �Ƿ��ڰ�ȫʱ���ڷ������¼�
	float	_Angle;			// �ӵ��ʹ��е����ļн�
	bool	_DirectHit;		// �Ƿ�ֱ������
	GameObject* _Obj;		// ��Ӱ������壨ShipComponent OR PhysGameObject��
};

struct HitNotifyInfo
{
	NxVec3	_HitPos;			// ����λ��
	NxVec3	_HitNormal;			// ���е�shape���߷���
	NxVec3	_HitDirection;		// ����ʱ�ڵ������䷽��

	HitNotifyInfo()
	{
		_HitPos = NxVec3(0.f);
		_HitNormal = NxVec3(0.f);
		_HitDirection = NxVec3(0.f);
	}
};

// ��ͬ�ٶ�������Ʋ�ͬ������
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
	float	_FirePointLastTime;								//	��������ʱ��
	bool	_RecordAimingOccasion;							//	�����ļ�¼��ʽ true �ǿ���ʱ���¼ false �� ���к��¼
	float	_AccuracyFormulaA;								//	���㹫ʽ��A��ֵ
	float	_AccuracyFormulaB;								//	���㹫ʽ��B��ֵ
	float	_AccuracyFormulaC;								//	���㹫ʽ��C��ֵ
	float	_AccuracyFormulaD;								//	���㹫ʽ��D��ֵ
	float	_AccuracyLimitMin;								//	��׼�ȵ�����Ӱ�����Сֵ

	float	_GPL_PhysWorld_HalfLength;						//	�������볤
	float	_GPL_PhysWorld_HalfWidth;						//	���������
	float	_GPL_PhysWorld_HalfHeight;						//	���������

	float	_GPL_MAX_HALF_WORLD;							//	�������볤
	float	_GPL_SKIN_WIDTH;
	float	_GPL_PHYS_GRAVITY;								//	���������Է���Ϊ��ʽ����UE3��Gravity=100����/��
	float	_GPL_PHYS_CLIENT_MAX_TIMESTEP;					//	�ͻ�������ģ��̶�����
	float	_GPL_PHYS_SERVER_MAX_TIMESTEP;					//	���������ģ��̶�����
	int		_GPL_PROJECTILE_TICK_COUNT;						//	�ڵ�����ʱģ��Ĵ�������ȡֵNX_MAX_I32����GPL_PHYS_CLIENT_MAX_TIMESTEP��GPL_PHYS_SERVER_MAX_TIMESTEP����
	float	_GPL_PROJECTILE_SIMULATED_TIME_MAX;				//	���ϴ�ģ��������ʱ��

	float	_GPL_FIRE_AVAILABLE_TOLERANCE;					//	in degree����������ʱ����ĽǶ�ƫ��ֵ(_CurrentYaw - _DesiredYaw)

	float	_GPL_WaterLayerOneHeight;						//	ˮ��һ�����
	float	_GPL_WaterLayerTwoHeight;						//	ˮ�¶������

	float	_GPL_SubmarineAccelerate;						//	Ǳˮͧ���ٽ׶α���
	float	_GPL_SubmarineConstant;							//	Ǳˮͧ���ٽ׶α���

	int		_GPL_FlightLayer_CruiseHeight;					//	���п��� - Ѳ���߶�
	int		_GPL_FlightLayer_DiveLimit;						//	���п��� - ���弫�޸߶�
	int		_GPL_FlightLayer_TorpedoRelease;				//	���п��� - ����Ͷ�Ÿ߶�

	float	_GPL_ReverseForce_DampingFactor;				//	��ǰ��״̬������״̬ʱ������ϵ��
	float	_GPL_ForwardForce_DampingFactor;				//	�ӵ���״̬��ǰ��״̬ʱ������ϵ��
	float	_GPL_ReverseSpeed_Rate;							//	��������ٶ����ǰ���ٶȵı���
	float	_GPL_ReverseForce_Rate;							//	������������ǰ�����ı���
	float	_GPL_TorqueDampingFactor;						//	Ť������ϵ������Ӱ��Ǽ��ٶȵĴ�С
	float	_GPL_FactorOfAngularVelocity;					//	Ӱ����ٶ�����
	float	_GPL_Aircraft_TakeOff_Angle;					//	�����������
	float	_GPL_Aircraft_Landing_Angle;					//	�������

	float	_GPL_PickTrace_Range;							//	ʰȡ�뾶
	bool	_GPL_Aircraft_Process_Blocks;					//	���ػ����Ͽ���
	float	_GPL_Aircraft_Block_Detect_Interval;			//	�ϰ����ʱ����
	int		_GPL_Aircraft_ActionCache_Length;				//	���ػ��������泤��
	float	_Aircraft_Interpose_Speed;						//	���ػ���ʾ�����ֵ�ٶ�
	float	_PhysAircraft_PositionInterp_Speed;				//	���ػ���������ֵ�ٶ�
	float	_PhysAircraft_RotationInterp_Speed;				//	���ػ���������ֵ�ٶ�

	float	_GPL_Sink_Roll_Limit;							//	��û�������
	float	_GPL_Sink_Pitch_Limit;							//	��û��������
	float	_GPL_Sink_Speed_Limit;							//	��û�ٶ�����
	float	_GPL_Sink_ForceFactor_LowerLimit;				//	��û����ϵ������
	float	_GPL_Sink_ForceFactor_UpperLimit;				//	��û����ϵ������
	float	_GPL_Sink_TorqueFactor_LowerLimit;				//	��ûŤ��ϵ������
	float	_GPL_Sink_TorqueFactor_UpperLimit;				//	��ûŤ��ϵ������
	float	_GPL_Sink_DampingFactor;						//	��û����ϵ��
	float	_GPL_Sink_Torque_DampingFactor;					//	��ûŤ������ϵ��

	float	_GPL_TorpedoSalvo_DeltaTangentVelocity;			//  �����������  �����ٶȲ�
	float	_GPL_TorpedoSalvo_DeltaAngle;					//  �����������  ����н�

	float	_GPL_Bomber_ReCollimate_PrecisionFactorA;		//	��ը������ָ��Ŀ����Ͷ�����Ȳ���Ӱ��Ĳ���A	
	float	_GPL_Bomber_ReCollimate_PrecisionFactorB;		//	��ը������ָ��Ŀ����Ͷ�����Ȳ���Ӱ��Ĳ���B

	float	_GPL_TorpedoPlane_LaunchTorlerance;				//	���׻�Ͷ���ݴ����
	float	_GPL_TorpedoPlane_LaunchAngleTorlerance;		//	���׻�Ͷ���ݴ�Ƕ�

	float	_GPL_Aircraft_Routing_Torlerance;				//	���ػ�·���滮�ݴ���루�뾶��
	float	_GPL_Aircraft_Routing_AngleTorlerance;			//	���ػ�·���滮�ݴ�Ƕ�

	float	_GPL_Aircraft_DiveAttack_SpeedFactor;			//	��ը�������ٶ�����
	float	_GPL_Aircraft_DiveAttack_LimitSpeed;			//	��ը�������ٶ�����
	float	_GPL_Aircraft_DiveAttack_MinInterval;			//	��ը����С����ʱ����
	float	_GPL_Aircraft_DiveAttack_MaxInterval;			//	��ը����󸩳�ʱ����

	bool	_GPL_MagneticTorpedo_FriendlyDetective;			//	�Ÿ�Ӧ�����Ƿ����Ѿ�

	std::vector<int>	_PartTypeListOfSupportContinuousFire;	// ֧��Խ��Խ׼�߼���PartType�б�

	std::vector<DampingFragment>	_DampingFragmentList;		// �����ٶ����������ϵ��

	std::vector<ReduceLevel>		_AccuracyDrivingReduce;		// ������ʻ��ɵ���׼����˥��

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