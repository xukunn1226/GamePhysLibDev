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

	//	@brief	计算俯冲轰炸点
	//	@param	[TargetPos]		基准目标点
	//	@param	[TargetDir]		基准方向
	//	@param	[RadialMiss]	最大径向误差（椭圆半轴长）
	//	@param	[TangentialMiss]最大切向误差（椭圆半轴长）
	//	@param	[Scale]			缩放系数
	//	@param	[Angle]			偏移角度（in Radian）
	//	@return	[NxVec3]		俯冲目标点(in World-Space)
	NxVec3 CalcDiveTarget(const NxVec3& TargetPos, const NxVec3& TargetDir, float RadialMiss, float TangentialMiss, float Scale, float Angle);

	NxVec3	VectorInterp(const NxVec3& Current, const NxVec3& Target, NxReal DeltaTime, NxReal InterpSpeed);

	ECollisionEvent NxEventTranslateCollisionEvent(NxU32 Events);

	// 选取目标点, Ocean, Terrain, StaticObject, Vehicle ship
	// 返回true，表示射线检测到物体，HitLoc有效
	bool TargetTrace(PhysGameScene* Scene, NxVec3 Start, NxVec3 Dir, bool CheckVehicle, NxVec3& HitLoc);
	
	// 返回true，表示前方一定距离内检测到障碍物
	bool ObstacleTrace(PhysGameScene* Scene, NxVec3 Start, NxVec3 Dir, float Extent, float Radius, bool CheckVehicle, NxVec3& HitLoc);

	// 这个trace与CameraTrace不同的地方在于该trace不会与边界空气墙产生碰撞
	PhysGameObject* CameraTraceEx(PhysGameScene* Scene, NxVec3 Start, NxVec3 End, float Extent, NxVec3& HitLoc, bool InLobby = false);

	// Actor检测
	PhysGameObject* ActorTrace(PhysGameScene* Scene, int Groups, NxVec3 Start, NxVec3 End, float Extent, NxVec3& HitLoc);
	// add by yeleiyu@icee.cn for decal project
	PhysGameObject* DecalTrace(PhysGameScene* Scene, int Groups, NxVec3 Start, NxVec3 End, float Extent, NxVec3& HitLoc , NxVec3& HitNormal);
	PhysGameObject* DecalTrace(PhysGameScene* Scene, int Groups, NxVec3 Start, NxVec3 End, float Extent, NxVec3& HitLoc , NxVec3& HitNormal,int& IsWeapon,int& AttachedId);
	//	组件跟踪
	GameObject*	ComponentTrace(PhysGameScene* Scene, int Groups, const NxVec3& Start, const NxVec3& End, float Extent);

	class DynamicAreaRaycastReport : public NxUserRaycastReport
	{
	public:
		DynamicAreaRaycastReport(PhysGameObject* Target, NxRaycastHit* hit)
			: _Target(Target)
			, _Hit(hit)
		{}

		// 返回TRUE，继续检测；返回FALSE，停止检测
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

	//	@brief	动态区域包含检测
	//	@param	[Scene]			物理场景
	//	@param	[TestObject]	检测对象（船）
	//	@param	[TargetObject]	目标区域（雾区）
	//	@param	[TraceGroup]	检测组
	//	@param	[TestSocket]	检测对象特征Socket
	bool DynamicAreaContainTest(PhysGameScene* Scene, PhysGameObject* TestObject, PhysGameObject* TargetObject, int TraceGroup, ESocketType TestSocket = EST_Socket_Invalid);

	/**
	 * 计算炮弹发射角度
	 * @param		SourceLoc	起始点
	 * @param		TargetLoc	目标点
	 * @param		VelSize		发射速度大小
	 * @param		GravityZ	重力加速度
	 * @param[out]	Pitch	发射角度(in degree)
	 * @return		true,可达到目标点；false，反之不可，初速度不够
	 *
	 * #1 formula: g*t^2 - 2v*t + 2s = 0;  v:垂直方向初速度; s: VertDist
	 * #2 formula: s = v*t; v: 水平方向速度; s: HoriDist
	 *
	 *
	 * formula: Distance = Vel * Vel * sin(2*Angle) / Gravity;
	 */
	bool CalcProjectileAngular(NxVec3 SourceLoc, NxVec3 TargetLoc, double VelSize, double GravityZ, float& Pitch );

	// 计算最远射程，仅限抛物线炮弹
	NxVec3 GetFarmostTarget(NxVec3 StartPos, NxVec3 Gravity, NxVec3 InitVelocity);

	float GetFarmostDistance(NxVec3 StartPos, NxVec3 Gravity, NxVec3 InitVelocity, NxVec3 TargetPos);

	/*
		取圆锥
	*/
	NxVec3 PickCone(NxVec3 const& Dir, float ConeHalfAngleRad,float SphereAngleRad);

	NxVec3 RotateAngleAxis( const float Angle, const NxVec3& Axis ,const NxVec3 &Dir) ;

	//true 是命中，false 是超出爆炸时间或最长飞行时间
	bool TraceProjectile(
		PhysGameScene* PhysScene, 
		PhysGameObject* Sponsor, 
		ProjectileDesc& ProjDesc, 
		float SimulatedTime, 
		bool CheckVehicle,			// 是否检测舰船（用于区分鹰眼视角和普通视角炮线） [12/24/2013 chenpu]
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