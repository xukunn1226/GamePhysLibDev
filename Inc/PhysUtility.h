#ifndef _PHYS_UTILITY_H_
#define _PHYS_UTILITY_H_

namespace GPL
{

	extern const NxVec3 ZERO_VEC;
	extern const NxVec3 AXIS_VEC_X;
	extern const NxVec3 AXIS_VEC_Y;
	extern const NxVec3 AXIS_VEC_Z;

	struct ProjectileDesc;

	// SCALE CONVERSION: gameplay to physX
	NxMat34 G2PTransform(const NxMat34& uTM);
	NxVec3 G2PPosition(const NxVec3& uVec);

	//	@brief	Transfer Direction-Vec to Quaternion
	//	@note	In X-Y Plane
	//	@param	[vec]	Direction Vector
	//	@return	Quaternion
	NxQuat	Dir2Quat(const NxVec3& vec);

	//	@brief	���㸩���ը��
	//	@param	[TargetPos]		��׼Ŀ���
	//	@param	[TargetDir]		��׼����
	//	@param	[RadialMiss]	���������Բ���᳤��
	//	@param	[TangentialMiss]�����������Բ���᳤��
	//	@param	[Scale]			����ϵ��
	//	@param	[Angle]			ƫ�ƽǶȣ�in Radian��
	//	@return	[NxVec3]		����Ŀ���(in World-Space)
	NxVec3 CalcDiveTarget(const NxVec3& TargetPos, const NxVec3& TargetDir, float RadialMiss, float TangentialMiss, float Scale, float Angle);

	NxVec3	VectorInterp(const NxVec3& Current, const NxVec3& Target, NxReal DeltaTime, NxReal InterpSpeed);

	ECollisionEvent NxEventTranslateCollisionEvent(NxU32 Events);

	// ѡȡĿ���, Ocean, Terrain, StaticObject, Vehicle ship
	// ����true����ʾ���߼�⵽���壬HitLoc��Ч
	bool TargetTrace(PhysGameScene* Scene, NxVec3 Start, NxVec3 Dir, bool CheckVehicle, NxVec3& HitLoc);
	
	// ����true����ʾǰ��һ�������ڼ�⵽�ϰ���
	bool ObstacleTrace(PhysGameScene* Scene, NxVec3 Start, NxVec3 Dir, float Extent, float Radius, bool CheckVehicle, NxVec3& HitLoc);

	// ���trace��CameraTrace��ͬ�ĵط����ڸ�trace������߽����ǽ������ײ
	PhysGameObject* CameraTraceEx(PhysGameScene* Scene, NxVec3 Start, NxVec3 End, float Extent, NxVec3& HitLoc, bool InLobby = false);

	// Actor���
	PhysGameObject* ActorTrace(PhysGameScene* Scene, int Groups, NxVec3 Start, NxVec3 End, float Extent, NxVec3& HitLoc);
	// add by yeleiyu@icee.cn for decal project
	PhysGameObject* DecalTrace(PhysGameScene* Scene, int Groups, NxVec3 Start, NxVec3 End, float Extent, NxVec3& HitLoc , NxVec3& HitNormal);
	PhysGameObject* DecalTrace(PhysGameScene* Scene, int Groups, NxVec3 Start, NxVec3 End, float Extent, NxVec3& HitLoc , NxVec3& HitNormal,int& IsWeapon,int& AttachedId);
	//	�������
	GameObject*	ComponentTrace(PhysGameScene* Scene, int Groups, const NxVec3& Start, const NxVec3& End, float Extent);

	class DynamicAreaRaycastReport : public NxUserRaycastReport
	{
	public:
		DynamicAreaRaycastReport(PhysGameObject* Target, NxRaycastHit* hit)
			: _Target(Target)
			, _Hit(hit)
		{}

		// ����TRUE��������⣻����FALSE��ֹͣ���
		virtual bool onHit(const NxRaycastHit& hits)
		{
			if (hits.shape == NULL)
				return true;

			if (hits.shape->getActor().userData != _Target)
				return true;
			
			*_Hit = hits;
			return false;
		}

	public:
		PhysGameObject* _Target;
		NxRaycastHit*	_Hit;
	};

	//	@brief	��̬����������
	//	@param	[Scene]			������
	//	@param	[TestObject]	�����󣨴���
	//	@param	[TargetObject]	Ŀ������������
	//	@param	[TraceGroup]	�����
	//	@param	[TestSocket]	����������Socket
	bool DynamicAreaContainTest(PhysGameScene* Scene, PhysGameObject* TestObject, PhysGameObject* TargetObject, int TraceGroup, ESocketType TestSocket = EST_Socket_Invalid);

	/**
	 * �����ڵ�����Ƕ�
	 * @param		SourceLoc	��ʼ��
	 * @param		TargetLoc	Ŀ���
	 * @param		VelSize		�����ٶȴ�С
	 * @param		GravityZ	�������ٶ�
	 * @param[out]	Pitch	����Ƕ�(in degree)
	 * @return		true,�ɴﵽĿ��㣻false����֮���ɣ����ٶȲ���
	 *
	 * #1 formula: g*t^2 - 2v*t + 2s = 0;  v:��ֱ������ٶ�; s: VertDist
	 * #2 formula: s = v*t; v: ˮƽ�����ٶ�; s: HoriDist
	 *
	 *
	 * formula: Distance = Vel * Vel * sin(2*Angle) / Gravity;
	 */
	bool CalcProjectileAngular(NxVec3 SourceLoc, NxVec3 TargetLoc, double VelSize, double GravityZ, float& Pitch );

	// ������Զ��̣������������ڵ�
	NxVec3 GetFarmostTarget(NxVec3 StartPos, NxVec3 Gravity, NxVec3 InitVelocity);

	float GetFarmostDistance(NxVec3 StartPos, NxVec3 Gravity, NxVec3 InitVelocity, NxVec3 TargetPos);

	/*
		ȡԲ׶
	*/
	NxVec3 PickCone(NxVec3 const& Dir, float ConeHalfAngleRad,float SphereAngleRad);

	NxVec3 RotateAngleAxis( const float Angle, const NxVec3& Axis ,const NxVec3 &Dir) ;

	//true �����У�false �ǳ�����ըʱ��������ʱ��
	bool TraceProjectile(
		PhysGameScene* PhysScene, 
		PhysGameObject* Sponsor, 
		ProjectileDesc& ProjDesc, 
		float SimulatedTime, 
		bool CheckVehicle,			// �Ƿ��⽢������������ӥ���ӽǺ���ͨ�ӽ����ߣ� [12/24/2013 chenpu]
		NxVec3& HitPos,
		float& FlightTime);

	extern class PhysProjectile* _TraceProjectile;

	NxVec3 EularToDir(float Roll, float Pitch, float Yaw);

	float Frand();
	float Gaussian(float m, float s);
	float RangeGaussian(float sigmarange);

	void Trim(std::wstring& src);

	TCHAR* PrintCommand(unsigned int cmd);
	TCHAR* PrintAction(unsigned int act);
};

#endif