#ifndef _GAMEOBJECT_FACTORY_H_
#define _GAMEOBJECT_FACTORY_H_

namespace GPL
{
struct UAttachedCompData
{
	int			_CompId;
	EPartType	_PartType;

	// ��������
	int			_FirePartNum;
	float		_InitVelocity;					// �����ڵ����ٶ�
	float		_TurretAngleVelocity;			// degree/sec, ��̨ת�����ٶ�
	float		_Gravity;						// ������ڵ������������ٶȣ����������������ڵ�
	float		_AccuracyDefault;				// Ĭ�����ɢ��뾶
	float		_AccuracyTurrent;				//��̨ת��������ȷ�ȵ�Ӱ��
	float		_AccuracyShipDrive;				//������ʻ�������ȷ�ȵ�Ӱ��
	float		_AccuracyShipSteer;				//����ת��������ȷ�ȵ�Ӱ��
	float		_AccuracyRecover;				//�����ȷ��ÿ��ָ�ֵ
	float		_AccuracyFire;					//���ڶ������ȷ�ȵ�Ӱ��
	float		_AccuracyContinuousFire;		//���������׼�����߽��͵ı���	
	float		_ArtilleryDiffuseParam;			//����ɢ�����

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
	NxMat34				_InitGlobalPose;				// �ؾ߳�ʼ��λ��
	NxVec3				_LinearVelocity;				// ���ٶ�
	float				_MaxForwardSpeed;				// ���ǰ�����ٶ�
	float				_MaxForwardForce;				// ǰ��ʱ���Ĵ�С
	float				_TurnTorque;					// ת��Ť��
	float				_MaxRiseSinkForce;				// �ϸ���ǱFORCE��С
	float				_Mass;							// ����
	float				_CMassOffsetX;					// ������X�᷽���OFFSET
	float				_Length;						// ����
	float				_Width;							// ����
	float				_Height;						// ����

	std::vector<UAttachedCompData>	_AttachedCompList;	// װ���Ĳ���ID List
};

struct UAttachedCompDataEx
{
	int			_CompId;
	int			_AttachId;

	// ��������
	int			_FirePartNum;
	float		_InitVelocity;					// �����ڵ����ٶ�
	float		_TurretAngleVelocity;			// degree/sec, ��̨ת�����ٶ�
	float		_Gravity;						// ������ڵ������������ٶȣ����������������ڵ�
	float		_AccuracyDefault;				// Ĭ�����ɢ��뾶
	float		_AccuracyTurrent;				//��̨ת��������ȷ�ȵ�Ӱ��
	float		_AccuracyShipDrive;				//������ʻ�������ȷ�ȵ�Ӱ��
	float		_AccuracyShipSteer;				//����ת��������ȷ�ȵ�Ӱ��
	float		_AccuracyRecover;				//�����ȷ��ÿ��ָ�ֵ
	float		_AccuracyFire;					//���ڶ������ȷ�ȵ�Ӱ��
	float		_AccuracyContinuousFire;		//���������׼�����߽��͵ı���	
	float		_ArtilleryDiffuseParam;			//����ɢ�����

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
	NxMat34				_InitGlobalPose;				// �ؾ߳�ʼ��λ��
	NxVec3				_LinearVelocity;				// ���ٶ�
	float				_MaxForwardSpeed;				// ���ǰ�����ٶ�
	float				_MaxForwardForce;				// ǰ��ʱ���Ĵ�С
	float				_TurnTorque;					// ת��Ť��
	float				_MaxRiseSinkForce;				// �ϸ���ǱFORCE��С
	float				_Mass;							// ����
	float				_CMassOffsetX;					// ������X�᷽���OFFSET
	float				_Length;						// ����
	float				_Width;							// ����
	float				_Height;						// ����

	std::vector<UAttachedCompDataEx>	_AttachedCompList;	// װ���Ĳ���ID List
};

//	@brief	���ػ�����
//	@note	�����ӳɼ�����ɷ������·�
struct AircraftDesc
{
	int					_CompId;
	int					_WeaponPartType;		//	���ػ�����	
	float				_Speed;					//	����
	float				_Consume;				//	�ͺ�
	float				_MinRadius;				//	��Сת��뾶
	float				_SensorRange;			//	��鷶Χ
	float				_InterceptRange;		//	���ط�Χ
	float				_AttackExtent;			//	������Χ�뾶
	float				_AttackScope;			//	������Χ�н�
	float				_ChaseRange;			//	׷����Χ
	float				_Gravity;				//	�ڵ�����
	float				_InitPrecision;			//	��ʼͶ������
	float				_Precision;				//	��׼Ͷ�����ȣ��ٷֱȣ�
	float				_Collimation;			//	��Ȧ�ٶȣ��ٷֱȣ�
	float				_TurningAffect;			//	ת�򾫶�Ӱ�죨�ٷֱȣ�
	float				_LayerAffect;			//	���㾫��Ӱ�죨�ٷֱȣ�
	float				_AscendAngle;			//	��������
	float				_SwoopAngle;			//	�������
	float				_ExLowFlyingTime;		//	��������ʱ��
	float				_LayerChangeSpeed;		//	�����ٶ�
	float				_EchoInterval;			//	��Ӧʱ��
	float				_DiveAcceleration;		//	������ٶ�
	float				_PullAcceleration;		//	�������ٶ�
	float				_DiffuseEccentricity;	//	Ͷ��ɢ��������

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
	float				_SafeTime;				//��ȫʱ�䣬��������ײ���ڵ���ʧ����λ����
	float				_BurstTime;				//������ʱ��ͱ�ը����λ����
	float				_BurstRange;			//����������˺��ķ�Χ
	NxVec3				_BrakingVelocity;		//�����ƶ��ٶ�,���������ס���ˮը����ˮ������ʱ���ٶȣ������ٶ�����������
	NxVec3				_TargetPos;				// ���Ŀ��㣬��׼���߼���һ������

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
	float				_SafeTime;				//	��ȫʱ�䣬��������ײ���ڵ���ʧ����λ����
	float				_BurstTime;				//	������ʱ��ͱ�ը����λ����
	float				_BurstRange;			//	����������˺��ķ�Χ
	NxVec3				_BrakingVelocity;		//	�����ƶ��ٶ�,���������ס���ˮը����ˮ������ʱ���ٶȣ������ٶ�����������
	NxVec3				_TargetPos;				//	���Ŀ��㣬��׼���߼���һ������
	float				_MagneticSensorRange;	//	�Ÿ����ż�ⷶΧ

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
	NxMat34				_InitGlobalPose;				// ��ʼ��λ��
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

// �ⲿ���ýӿ�
PhysVehicle* UCreateShip(PhysGameScene* PhysScene, UShipDesc& Desc);
PhysVehicle* UCreateShipEx(PhysGameScene* PhysScene, UShipDescEx& Desc);
PhysAircraft* CreateAircraft(PhysGameScene* PhysScene, AircraftDesc& Desc);
PhysProjectile* UCreateProjectile(PhysGameScene* PhysScene, PhysGameObject* Sponsor, int AttachId, UProjectileDesc& ProjDesc);
PhysTrigger* UCreateTrigger(PhysGameScene* PhysScene, UTriggerDesc& Desc);

PhysGameObject* CreatePhysGameObject(PhysGameScene* Scene, EGameObjectType Type, ComponentData* ObjectData);

void DestroyPhysGameObject(class PhysGameScene* Scene, PhysGameObject* GameObject);

// �ڲ����ýӿ�
NxShapeDesc* CreateComponentShape(const ShapeDesc* const CustomShapeDesc, NxVec3 shapeScale = NxVec3(1.0f,1.0f,1.0f));
void ReleaseShapeDescList(std::vector<NxShapeDesc*>& CachedShapeDescList);
PhysProjectile* CreateProjectile(PhysGameScene* PhysScene, PhysGameObject* Sponsor, int AttachId, ProjectileDesc& ProjDesc);
PhysTrigger* CreateTrigger(PhysGameScene* PhysScene, UTriggerDesc& Desc);


}

#endif