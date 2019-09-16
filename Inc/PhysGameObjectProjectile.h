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
		enHitIdle,			// ����״̬��û������
		enHit,				// ����
		enSelfDestory		// �Լ�����
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

	// ����ʼ�㵽Ŀ���ķ���ʱ��
	virtual float GetFlightTime(NxVec3 TargetPos);

	// ��ElapsedTime���ڵ���λ��
	virtual NxVec3 GetFlightLocation(float ElapsedTime);

	// ��ElapsedTime���ڵ��ٶ�
	virtual NxVec3 GetFlightVelocity(float ElapsedTime);

	//	@brief	��ȡ��������
	ETrajectoryMethod	GetTrajectoryMethod();

	//	@brief	��ȡĿ��λ��
	NxVec3 GetTargetPos();

	//	@brief	��ȡ�ڵ�����
	NxVec3 GetTowards();

	// �������Ӧ���ڵ�ģ��ʱ��
	float GetAdaptiveSimulatedTime();

	// ��ǰ�ڵ��ѷ��е�ʱ�估�Ƿ�����
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

	//	@brief	Ŀ���Ƿ��ڻ������Ƿ�Χ��
	bool	IsTargetInCoverage(float LowerLimitAngle, float UpperLimitAngle);

	//	@brief	���÷��䲿����AttachId
	inline void		SetSponsorAttachId(int AttachId)	{ _SponsorAttachId = AttachId; }
	//	@brief	��ȡ���䲿����AttachId
	inline int		GetSponsorAttachId()				{ return _SponsorAttachId; }
	//	@brief	��õ�һ����Ч���Ŀ��
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

		// ����TRUE��������⣻����FALSE��ֹͣ���
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
	 * ������ʼ����Ŀ���ķ���ʱ��
	 */
	float GetPartFlightTime(ETrajectoryMethod method, NxVec3 Gravity, NxVec3 StartPos,NxVec3 InitVelocity,NxVec3 TargetPos);

	/**
	 * ����ʼ�㿪ʼ����ElapsedTime���ڵ����е���λ��
	 */
	NxVec3 GetPartFlightLocation(ETrajectoryMethod method,NxVec3 StartPos,NxVec3 InitVelocity,float ElapsedTime);

	/**
	 * ������ʼ�ٶȾ���ElapsedTime����ٶ�
	 */
	NxVec3 GetPartFlightVelocity(ETrajectoryMethod method,NxVec3 InitVelocity,float ElapsedTime);

	/**
	 * �ڵ���ˮ����е��ٶȣ���������ס���ˮը����Ч
	 */
	NxVec3 GetBrakingVelocity(ETrajectoryMethod method);

	virtual void InitPhys();

	/**
	 * �ж�[StartPos,EndPos]�Ƿ�����ײ����
	 * ����TRUE��������ײ��FALSE,û�з�����ײ
	 */
	virtual bool IsHitObject(NxVec3 StartPos, NxVec3 EndPos);

	/**
	 * �Ÿ�Ӧ�����������
	 */
	bool MagneticBurstTest(float DeltaTime);

	/**
	 * �ڵ��������󣬼��㱬ը��Χ�˺���֪ͨ�ܵ��˺���ShipComponent
	 */
	void CheckBurstHit(NxVec3 pos,NxVec3 normal, NxVec3 HitDirection, ShipComponent* IngoreComponent, PhysGameObject* IngoreActor);

	/**
	 * �������н��
	 * ����TRUE���ڵ���������ڲ���ײ��Ϣ��FALSE����ֹ�������
	 */
	bool SendStrikeMsg(GameObject* HitComp,
					   NxVec3 StrikeNormal=NxVec3(0.0f,0.0f,1.0f),
					   NxVec3 StrikePos=NxVec3(0.0f),
					   PhysGameObject* vehile=NULL,
					   float Distance = 0.f,
					   bool bDirectHit=true);

	// ����������е��֪ͨ
	void PostBurstMsg(NxVec3 HitPos,NxVec3 HitNormal, NxVec3 HitDirection);

	/**
	 * ����ڵ����й켣�Ƿ���Ҫ�ı䣨�������ס���ˮը����ˮ����Ҫ�ı䣩
	 * ����TRUE���켣�����ı䣻FALSE������Ҫ�ı�
	 */
	bool CheckChangeTrajectory(PhysGameObject * HitObj,const NxRaycastHit &HitInfo);

	/**
	 * ���㱬ը��ར�����������
	 */
	float GetBurstDistance(const NxVec3 &BurstPos,PhysVehicle* ship);

	/**
	 * ĳ����������ײ���
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
	 * ʹ��Sweep�����ײ
	 * ����TRUE��������ײ��FALSE,û�з�����ײ
	 */
	bool SweepObject(const NxVec3 & StartPos,const NxVec3 &EndPos);	
	
	enum TMStep
	{
		enIdle,//��׼�׶Σ�������
		enOnWater//��ˮ���ϣ�ֱ��
	};


	PhysGameObject*		_Sponsor;
	int					_SponsorAttachId;			//	���䲿����AttachId
	float				_IntervalSimulatedTime;		//	step time of raycast
	float				_ElapsedSimulatedTime;
	float				_TotalFlightSimulatedTime;
	float				_ElapsedFlightTime;
	NxVec3				_CurFlightLoc;
	NxVec3				_TargetPos;
	EHitState			_HitState;
	NxVec3				_HitWaterPose;
	float				_OnWaterTime;				//	��ˮ������ʻ��ʱ�䣬��ȫʱ������ʱ��ʼ�㣻
	TMStep				_Step;
	bool				_bForceSelfDestruction;		//	force self-destruction

	bool				_bUtilityTest;				//	��������Ա�־
	bool				_bVehicleTest;				//	��������־
	NxVec3				_HitPos;					//	���������ʹ��
	float				_TestSimulatedTime;			//

	PhysGameObject*		_FirstValidHitActor;				//	��һ���������������

	/************************************************************************/
	/* �Ÿ�����������չ														*/
	/************************************************************************/
	typedef std::vector<MagneticTargetStatus>	MagneticTestResult;
	MagneticTestResult	_ValidMagneticTarget;		//	��Ч�Ÿ�ӦĿ��
	float				_MagneticTestInterval;		//	�Ÿм��ʱ����
	float				_MagneticTestTime;			//	�Ÿ�Ӧ����ʱ
};	

}

#endif