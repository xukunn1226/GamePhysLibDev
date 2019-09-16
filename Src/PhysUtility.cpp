#include "..\Inc\PhysXSupport.h"

namespace GPL
{

	const NxVec3 ZERO_VEC(0, 0, 0);
	const NxVec3 AXIS_VEC_X(1, 0, 0);
	const NxVec3 AXIS_VEC_Y(0, 1, 0);
	const NxVec3 AXIS_VEC_Z(0, 0, 1);

	PhysProjectile * _TraceProjectile = NULL;
	NxMat34 G2PTransform(const NxMat34& uTM)
	{
		NxMat34 NewTM;

		NewTM = uTM;
		NewTM.t = uTM.t * G2PScale;
		return NewTM;
	}

	NxVec3 G2PPosition(const NxVec3& uVec)
	{
		return uVec * G2PScale;
	}

	NxQuat Dir2Quat(const NxVec3& vDir)
	{
		NxQuat Rot;
		Rot.zero();

		if ( vDir.equals(ZERO_VEC, 0.01f) )
		{
			return Rot;
		}

		NxVec3 src = AXIS_VEC_X;	//	原始方向
		NxVec3 des = vDir;			//	当前方向
		src.normalize();
		des.normalize();

		NxVec3 normal = src.cross(des);

		float radian = asin(normal.magnitude());

		if ( src.dot(des) > 0 )
		{
			if ( normal.sameDirection(AXIS_VEC_Z) )
				radian = radian;
			else
				radian = NxPi * 2 - radian;
		}
		else
		{
			if ( normal.sameDirection(AXIS_VEC_Z) )
				radian = NxPi - radian;
			else
				radian = NxPi + radian;
		}

		Rot.fromAngleAxis(NxMath::radToDeg(radian), AXIS_VEC_Z);

		return Rot;
	}

	NxVec3 CalcDiveTarget(const NxVec3& TargetPos, const NxVec3& TargetDir, float RadialMiss, float TangentialMiss, float Scale, float Angle)
	{
		NxVec3 FinalTarget = TargetPos;

		float DiffuseScale = NxMath::clamp(Scale, 1.f, 0.f);
		NxVec3 Offset;
		Offset.x = RadialMiss * DiffuseScale * NxMath::cos(Angle);
		Offset.y = TangentialMiss * DiffuseScale * NxMath::sin(Angle);
		Offset.z = 0;

		NxVec3 Direction(TargetDir.x, TargetDir.y, 0);
		if (Direction.normalize() > 0.1)
		{
			NxQuat RotationYaw = Dir2Quat(Direction);
			FinalTarget += RotationYaw.rot(Offset);
		}
		else
		{
			FinalTarget += Offset;
		}

		return FinalTarget;
	}

	NxVec3 VectorInterp(const NxVec3& Current, const NxVec3& Target, NxReal DeltaTime, NxReal InterpSpeed)
	{
		if( InterpSpeed <= 0.f )
			return Target;

		// Distance to reach
		const NxVec3 Dist = Target - Current;

		// If distance is too small, just set the desired location
		if( Dist.magnitudeSquared() < 0.0001f )
			return Target;

		// Delta Move, Clamp so we do not over shoot.
		const NxVec3 DeltaMove = Dist * NxMath::clamp(DeltaTime * InterpSpeed, 1.f, 0.f);

		return Current + DeltaMove;
	}

	ECollisionEvent NxEventTranslateCollisionEvent(NxU32 Events)
	{
		int CollisionEvents = 0;
		if( Events & NX_NOTIFY_ON_START_TOUCH )
		{
			CollisionEvents |= ENE_On_Start_Touch;
		}
		if( Events & NX_NOTIFY_ON_END_TOUCH )
		{
			CollisionEvents |= ENE_On_End_Touch;
		}
		if( Events & NX_NOTIFY_ON_TOUCH )
		{
			CollisionEvents |= ENE_On_Touch;
		}
		return ECollisionEvent(CollisionEvents);
	}
	class myRaycastReport : public NxUserRaycastReport
	{
		virtual bool onHit(const NxRaycastHit& hit);
	};
	
	bool TargetTrace( PhysGameScene* Scene, NxVec3 Start, NxVec3 Dir, bool CheckVehicle, NxVec3& HitLoc )
	{
		if ( Scene == NULL )
			return false;

		NxRay WorldRay;
		WorldRay.orig = Start;
		WorldRay.dir = Dir;
		WorldRay.dir.normalize();
		
		// 不可选取GPL_SHAPE_GROUP_TRIGGER_VOLUME
		NxU32 groups = 0;
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_OCEAN);
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_TERRAIN);
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_STATICOBJECT);
		if ( CheckVehicle )
			ENABLE_STATE(groups, GPL_SHAPE_GROUP_VEHICLE_SHIP);

		NxRaycastHit HitInfo;
		NxShape* HitShape = Scene->_PhysScene->raycastClosestShape(
			WorldRay, 
			NX_ALL_SHAPES, 
			HitInfo, 
			groups,
			GPhysGameSetting._GPL_MAX_HALF_WORLD, 0 );

		if ( HitShape == NULL )
			return false;
		
		if( CheckVehicle )
		{
			//	检测到了不可见的舰船则忽略舰船重新检测
			if ( HitShape->getActor().userData != NULL )
			{
				PhysGameObject* HitObj = (PhysGameObject*)HitShape->getActor().userData;
				PhysVehicle* HitVehicle = DynamicCast(PhysVehicle, HitObj);
				if ( HitVehicle && !HitVehicle->IsDetectable() )
				{
					groups = (1 << GPL_SHAPE_GROUP_OCEAN |
						1 << GPL_SHAPE_GROUP_TERRAIN |
						1 << GPL_SHAPE_GROUP_STATICOBJECT);

					HitShape = Scene->_PhysScene->raycastClosestShape(
						WorldRay, 
						NX_ALL_SHAPES, 
						HitInfo, 
						groups,
						GPhysGameSetting._GPL_MAX_HALF_WORLD, 0 );
				}
			}
		}
		HitLoc = HitInfo.worldImpact;

		return true;
	}

	// 返回true，表示前方一定距离内检测到障碍物，HitLoc有效
	bool ObstacleTrace(PhysGameScene* Scene, NxVec3 Start, NxVec3 Dir, float Extent, float Radius, bool CheckVehicle, NxVec3& HitLoc)
	{
		if ( Scene == NULL )
			return false;

		NxVec3 Route, End;
		Route = Dir;
		Route.normalize();
		Route.multiply(Extent, Route);
		End.add(Start, Route);

		NxCapsule capsule;
		capsule.p0 = Start;
		capsule.p1 = End;
		capsule.radius = Radius;

		NxU32 groups = 0;
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_TERRAIN);
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_STATICOBJECT);
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_BLOCKING_VOLUME);
		 
		if ( CheckVehicle )
			ENABLE_STATE(groups, GPL_SHAPE_GROUP_VEHICLE_SHIP);

		bool isCollision = Scene->_PhysScene->checkOverlapCapsule(capsule, NX_ALL_SHAPES, groups);
		if(isCollision)
		{
			HitLoc = End;
		}
		
		return isCollision;
	}
	
	PhysGameObject* CameraTraceEx(PhysGameScene* Scene, NxVec3 Start, NxVec3 End, float Extent, NxVec3& HitLoc, bool InLobby/* = false*/)
	{
		if( Scene == NULL )
			return NULL;

		NxRay WorldRay;
		WorldRay.orig = Start;
		WorldRay.dir = End - Start;
		WorldRay.dir.normalize();

		NxRaycastHit HitInfo;
		NxU32 groups = 0;
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_OCEAN);
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_TERRAIN);
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_STATICOBJECT);
		ENABLE_STATE(groups, (InLobby ? GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME : GPL_SHAPE_GROUP_VEHICLE_SHIP));

		NxShape* HitShape = Scene->_PhysScene->raycastClosestShape(
			WorldRay, 
			NX_ALL_SHAPES, 
			HitInfo, 
			groups,
			Start.distance(End) + Extent, 0 );

		if (NULL != HitShape)
		{
			HitLoc = HitInfo.worldImpact - WorldRay.dir * Extent;
			return (PhysGameObject*)HitShape->getActor().userData;
		}

		return NULL;
	}

	PhysGameObject* ActorTrace(PhysGameScene* Scene, int Groups, NxVec3 Start, NxVec3 End, float Extent, NxVec3& HitLoc)
	{
		PhysGameObject* HitActor = NULL;
		if( Scene == NULL )
			return HitActor;

		NxRay WorldRay;
		WorldRay.orig = Start;
		WorldRay.dir = End - Start;
		WorldRay.dir.normalize();
				
		NxRaycastHit HitInfo;
		NxShape* HitShape = Scene->_PhysScene->raycastClosestShape(
			WorldRay, 
			NX_ALL_SHAPES, 
			HitInfo, 
			Groups,
			Start.distance(End) + Extent, 0 );

		if( HitShape != NULL )
		{
			HitActor = (PhysGameObject*)(HitShape->getActor().userData);
			if( HitActor != NULL )
			{
				HitLoc = Start + WorldRay.dir * (HitInfo.distance - Extent);
			}
		}
		return HitActor;
	}

        // add by yeleiyu@icee.cn for decal project
	PhysGameObject* DecalTrace(PhysGameScene* Scene, int Groups, NxVec3 Start, NxVec3 End, float Extent, NxVec3& HitLoc , NxVec3& HitNormal)
	{
		PhysGameObject* HitActor = NULL;
		if( Scene == NULL )
			return HitActor;

		NxRay WorldRay;
		WorldRay.orig = Start;
		WorldRay.dir = End - Start;
		WorldRay.dir.normalize();

		//if (!NxMath::equals(WorldRay.dir.magnitude(), 1.f, 0.0001f))
		//	gplDebugf(TEXT("PhysUtility::DecalTrace NxRay dir warning"));

		NxRaycastHit HitInfo;
		NxShape* HitShape = Scene->_PhysScene->raycastClosestShape(
			WorldRay, 
			NX_ALL_SHAPES, 
			HitInfo, 
			Groups,
			Start.distance(End) + Extent, 0 );

		if( HitShape != NULL )
		{
			HitActor = (PhysGameObject*)(HitShape->getActor().userData);
			if( HitActor != NULL )
			{
				HitLoc = Start + WorldRay.dir * (HitInfo.distance - Extent);
				HitNormal = HitInfo.worldNormal;
			}
		}
		
		return HitActor;
	}


	PhysGameObject* DecalTrace(PhysGameScene* Scene, int Groups, NxVec3 Start, NxVec3 End, float Extent, NxVec3& HitLoc , NxVec3& HitNormal,int& IsWeapon,int& AttachedId )
	{
		PhysGameObject* HitActor = NULL;
		if( Scene == NULL )
			return HitActor;

		NxRay WorldRay;
		WorldRay.orig = Start;
		WorldRay.dir = End - Start;
		WorldRay.dir.normalize();

		//if (!NxMath::equals(WorldRay.dir.magnitude(), 1.f, 0.0001f))
		//	gplDebugf(TEXT("PhysUtility::DecalTrace NxRay dir warning"));

		NxRaycastHit HitInfo;
		NxShape* HitShape = Scene->_PhysScene->raycastClosestShape(
			WorldRay, 
			NX_ALL_SHAPES, 
			HitInfo, 
			Groups,
			Start.distance(End) + Extent, 0 );

		if( HitShape != NULL )
		{
			ShipComponent* pComp = (ShipComponent*)HitShape->userData;
			if( pComp == NULL || (pComp->GetSocketAttachedTo()._PartType == EPT_Deck || pComp->GetSocketAttachedTo()._PartType == EPT_Sideboard) )
			{ // body
				IsWeapon=1;
				AttachedId=0;
			}
			else
			{ // others
				IsWeapon=2;
				AttachedId=pComp->GetSocketAttachedTo()._AttachedId;
				EPartType PartType = pComp->GetSocketAttachedTo()._PartType;
				if (PartType == EPT_Engine )
				{
					IsWeapon=3;
				}
				else if (PartType == EPT_Bridge )
				{
					IsWeapon=4;
				}
			}

			HitActor = (PhysGameObject*)(HitShape->getActor().userData);
			if( HitActor != NULL )
			{
				HitLoc = Start + WorldRay.dir * (HitInfo.distance - Extent);
				HitNormal = HitInfo.worldNormal;
			}
		}

		return HitActor;
	}

	GameObject* ComponentTrace(PhysGameScene* Scene, int Groups, const NxVec3& Start, const NxVec3& End, float Extent)
	{
		if (NULL == Scene)
			return NULL;

		NxRay WorldRay;
		WorldRay.orig = Start;
		WorldRay.dir = End - Start;
		NxReal MaxDis = WorldRay.dir.normalize() + Extent;
		
		NxRaycastHit Hit;
		NxShape* HitShape = Scene->_PhysScene->raycastClosestShape(
			WorldRay, 
			NX_ALL_SHAPES, 
			Hit, 
			Groups, 
			MaxDis, 
			0xffffffff);

		if (HitShape != NULL)
		{
			if (NULL != HitShape->userData)
				return (GameObject*)HitShape->userData;
			
			return (GameObject*)HitShape->getActor().userData;
		}

		return NULL;
	}

	bool DynamicAreaContainTest(PhysGameScene* Scene, PhysGameObject* TestObject, PhysGameObject* TargetObject, int TraceGroup, ESocketType TestSocket /* = EST_Socket_Invalid */)
	{
		if (NULL == Scene || NULL == Scene->_PhysScene || NULL == TestObject || NULL == TargetObject)
			return false;

		NxRay worldRay;
		NxRaycastHit hitResult;
		DynamicAreaRaycastReport report(TargetObject, &hitResult);

		std::vector<Socket> vSockets;
		if (TestSocket != EST_Socket_Invalid && TestObject->_ComponentData->FindSocket(TestSocket, vSockets))
		{
			std::vector<Socket>::iterator it;
			for (it = vSockets.begin(); it != vSockets.end(); it++)
			{
				TestObject->GetGlobalPos().multiply(it->_LocalPose.t, worldRay.orig);
				worldRay.dir = TargetObject->GetGlobalPosition() - worldRay.orig;
				NxReal Dist = worldRay.dir.normalize();
				hitResult.shape = NULL;
				if (Scene->_PhysScene->raycastAllShapes(worldRay, report, NX_ALL_SHAPES, TraceGroup, Dist) != 0)
				{
					if (hitResult.shape != NULL)
						return false;
				}
			}
		}
		else
		{
			worldRay.orig = TestObject->GetGlobalPosition();
			worldRay.dir = TargetObject->GetGlobalPosition() - worldRay.orig;
			NxReal Dist = worldRay.dir.normalize();
			NxU32 ShapeNum = Scene->_PhysScene->raycastAllShapes(worldRay, report, NX_ALL_SHAPES, TraceGroup, Dist);
			//gplDebugf(TEXT("DynamicAreaContainTest HitResult:HitShapeNum[%d]"), ShapeNum);
			if (ShapeNum > 0)
			{
				if (hitResult.shape != NULL)
					return false;
			}
		}

		return true;
	}

	bool CalcProjectileAngular(NxVec3 SourceLoc, NxVec3 TargetLoc, double VelSize, double GravityZ, float& Pitch )
	{
		double S1;		// 目标点到起始点Z方向距离
		double S2;		// 目标点到起始点水平方向距离
		double VelSizeSq;
		double A, B, C, D, E, F;
		double ArcMin;
		double ArcMax;
		double ArcsineMin;
		double ArcsineMax;
		double RadAngular;
		double TD;
		double RadAngular1;
		double RadAngular2;

		S1 = (TargetLoc.z - SourceLoc.z);
		NxVec3 tmp = (TargetLoc - SourceLoc);
		tmp.z = 0;
		S2 = tmp.magnitude();

		//	目标与起点重合，90度仰角，落点射程内
		if ( NxMath::equals(S2, 0, 0.01) )
		{
			Pitch = 90.f;
			return true;
		}

		VelSizeSq = VelSize * VelSize;
		A = 2 * S1 * VelSizeSq;
		B = S2 * VelSizeSq;
		C = S2 * S2 * GravityZ;
		D = B*B - C*C - C*A;

		if ( D < 0 )
			return false;

		D = NxMath::sqrt(D);

		TD = (2*A)*D;
		E = 2*B * (2*C + A);
		F = 4*B*B + A*A;

		ArcMax = (E + TD) / F;
		ArcMin = (E - TD) / F;
		ArcsineMin = std::min(ArcMin, ArcMax);
		ArcsineMax = std::max(ArcMin, ArcMax);
		ArcsineMax = std::min(ArcsineMax, 1.0);
		ArcsineMin = std::max(ArcsineMin, -1.0);

		RadAngular1 = NxMath::asin(ArcsineMax) * 0.5;
		RadAngular2 = NxMath::asin(ArcsineMin) * 0.5;

		if( S1 >= 0 )
		{
			RadAngular = NxMath::max(RadAngular1, RadAngular2);
		}
		else
		{
			RadAngular = NxMath::min(RadAngular1, RadAngular2);
		}
		Pitch = (float)RadAngular * GPL_RadToDeg;

		return true;
	}
	
	NxVec3 GetFarmostTarget(NxVec3 StartPos, NxVec3 Gravity, NxVec3 InitVelocity)
	{
		NxVec3 Result = StartPos;
		float ElapsedTime = 2.f * NxMath::abs(InitVelocity.z / Gravity.z);
		
		// formula: X(t) = X(0) + V * t + 0.5 * G * t*t;
		Result = StartPos + InitVelocity * ElapsedTime + 0.5f * Gravity * ElapsedTime * ElapsedTime;

		return Result;
	}

	float GetFarmostDistance(NxVec3 StartPos, NxVec3 Gravity, NxVec3 InitVelocity, NxVec3 TargetPos)
	{
		float GravityZ = Gravity.z;
		float InitVelZ = InitVelocity.z;
		float Delta = 4 * InitVelZ * InitVelZ - 4 * GravityZ * 2 * (TargetPos.z - StartPos.z);
		if( GravityZ == 0.f || Delta < 0.f )
			return 0;		// 无解

		Delta = NxMath::sqrt(Delta);
		float Ret1 = (2 * InitVelZ + Delta) / (2 * GravityZ);
		float Ret2 = (2 * InitVelZ - Delta) / (2 * GravityZ);
		float Ret = NxMath::max(Ret1, Ret2);
		return NxMath::max(Ret, 0.f);
	}

	NxVec3 RotateAngleAxis( float Angle, const NxVec3& Axis ,const NxVec3 &Dir)
	{
		const float S	= sin(Angle);
		const float C	= cos(Angle);

		const float XX	= Axis.x * Axis.x;
		const float YY	= Axis.y * Axis.y;
		const float ZZ	= Axis.z * Axis.z;

		const float XY	= Axis.x * Axis.y;
		const float YZ	= Axis.y * Axis.z;
		const float ZX	= Axis.z * Axis.x;

		const float XS	= Axis.x * S;
		const float YS	= Axis.y * S;
		const float ZS	= Axis.z * S;

		const float OMC	= 1.f - C;

		return NxVec3(
			(OMC * XX + C ) * Dir.x + (OMC * XY - ZS) * Dir.y + (OMC * ZX + YS) * Dir.z,
			(OMC * XY + ZS) * Dir.x + (OMC * YY + C ) * Dir.y + (OMC * YZ - XS) * Dir.z,
			(OMC * ZX - YS) * Dir.x + (OMC * YZ + XS) * Dir.y + (OMC * ZZ + C ) * Dir.z
			);
	}

	NxVec3 PickCone(NxVec3 const& Dir, float ConeHalfAngleRad,float SphereAngleRad)
	{
		if( ConeHalfAngleRad > 0.f && SphereAngleRad > 0.f )
		{
			float Phi = ConeHalfAngleRad;
			float Theta = SphereAngleRad;

			// get axes we need to rotate around
			//FMatrix const DirMat = FRotationMatrix(Dir.Rotation());

			float Yaw=atan2(Dir.y,Dir.x);
			float Pitch = atan2(Dir.z,sqrt(Dir.x*Dir.x+Dir.y*Dir.y));
			float Roll=0;

			const float	SR	= sin(Roll);
			const float	SP	= sin(Pitch);
			const float	SY	= sin(Yaw);
			const float	CR	= cos(Roll);
			const float	CP	= cos(Pitch);
			const float	CY	= cos(Yaw);

			// note the axis translation, since we want the variation to be around X
			//	FVector const DirZ = DirMat.GetAxis(0);       
			//	FVector const DirY = DirMat.GetAxis(1);

			NxVec3  const DirZ(CP * CY,CP * SY,SP);
			NxVec3  const DirY(SR * SP * CY - CR * SY,SR * SP * SY + CR * CY,- SR * CP);

			// convert to unreal rot units, to satisfy RotateAngleAxis
			//FLOAT const Rad2Unr = 65536.f/(2.f*PI);
			//FVector Result = Dir.RotateAngleAxis(appTrunc(Rad2Unr*Phi), DirY);
			//Result = Result.RotateAngleAxis(appTrunc(Rad2Unr*Theta), DirZ);

			NxVec3 Result=RotateAngleAxis(Phi,DirY,Dir);
			Result=RotateAngleAxis(Theta,DirZ,Result);

			//Result = Result.SafeNormal();
			// ensure it's a unit vector (might not have been passed in that way)
			Result.normalize();
			return Result;
		}
		return Dir;
	}

	bool TraceProjectile(PhysGameScene* PhysScene, PhysGameObject* Sponsor, ProjectileDesc& ProjDesc, float SimulatedTime, bool CheckVehicle, NxVec3& HitPos ,float& FlightTime)
	{
		if( !_TraceProjectile )
		{
			//	弹道模拟不需要判断记录点逻辑，故不用设置AttachId
			_TraceProjectile = CreateProjectile(PhysScene, Sponsor, 0, ProjDesc);
		}
		else
		{
			ComponentData* data = _TraceProjectile->GetComponentData();
			if ( !LoadComponentDesc(ProjDesc._CompId, data->_CompDesc) )		// 加载炮弹配置数据
			{
				gplDebugf(TEXT("CreateProjectile: LoadComponentDesc[%d]失败"), ProjDesc._CompId);
				return false;
			}

			ProjectileData* ProjData = (ProjectileData*)data;
			ProjData->_StartPos = ProjDesc._StartPos;
			ProjData->_Gravity = ProjDesc._Gravity;
			ProjData->_InitVelocity = ProjDesc._InitVelocity;
			ProjData->_TrajectoryMethod = ProjDesc._TrajectoryMethod;
			ProjData->_SafeTime = ProjDesc._SafeTime;
			ProjData->_BurstTime = ProjDesc._BurstTime;
			ProjData->_BurstRange = ProjDesc._BurstRange;
			ProjData->_BrakingVelocity = ProjDesc._BrakingVelocity;			
		}

		_TraceProjectile->EnableUtilityTest(true);
		_TraceProjectile->EnableVehicleTest(CheckVehicle);
		_TraceProjectile->SetTestSimulatedTime(SimulatedTime);
		_TraceProjectile->InitProData();
		float time = 10.0f;

		float delta = SimulatedTime;
		while(time > 0)
		{
			_TraceProjectile->Tick(delta);
			time -= delta;
			if (_TraceProjectile->GetHitState() != PhysProjectile::enHitIdle)
			{
				HitPos = _TraceProjectile->GetHitPos();
				bool bHit;
				_TraceProjectile->GetCurFlightTime(FlightTime, bHit);
				break;
			}
		}		

		return _TraceProjectile->GetHitState() == PhysProjectile::enHit;
	}

	NxVec3 EularToDir(float Roll, float Pitch, float Yaw)
	{
		NxQuat QYaw, QPitch, QRoll;
		QYaw.fromAngleAxisFast(Yaw, AXIS_VEC_Z);
		QPitch.fromAngleAxisFast(Pitch, AXIS_VEC_Y);
		QRoll.fromAngleAxisFast(Roll, AXIS_VEC_X);

		NxQuat Rst = QYaw;
		Rst *= QPitch;
		Rst *= QRoll;

		return Rst.rot(AXIS_VEC_X);
	}

	//------------------------------------------------------------------------
	//	正态分布辅助函数
	//------------------------------------------------------------------------
	float Frand()
	{
		return ((float) rand())/((float) RAND_MAX);
	}

	// gaussian random number, using box muller technique
	// m - mean, s - standard deviation
	float Gaussian(float m, float s)  
	{
		float x1, x2, w, y1;
		if (GGaussianUseLlast)  // use value from previous call
		{
			y1 = GGaussianY2;
			GGaussianUseLlast = 0;
		}
		else
		{
			do 
			{
				x1 = 2.0f * Frand() - 1.0f;
				x2 = 2.0f * Frand() - 1.0f;

				w = x1 * x1 + x2 * x2;
			}
			while (w >= 1.0f);

			w = sqrt((-2.0f * logf(w)) / w);
			y1 = x1 * w;
			GGaussianY2 = x2 * w;

			GGaussianUseLlast = 1;
		}

		return (m + y1 * s);
	}

	float RangeGaussian(float sigmarange)
	{
		float val = fabs(Gaussian(0, 1));
		if (val > sigmarange)
		{
			return Frand();
		}
		else
		{
			return val / sigmarange;
		}
	}

	void Trim(std::wstring& src)
	{
		std::wstring::size_type _offset;
		while (true)
		{
			_offset = src.find_first_of(TEXT(" "));
			if (_offset == std::wstring::npos)
				break;

			src.erase(_offset, 1);
		} 
	}
	
	TCHAR* PrintCommand(unsigned int cmd)
	{
		switch ( cmd )
		{
		case _TakeOff:			return TEXT("_TakeOff");
		case _Cruise:			return TEXT("_Cruise");
		case _Convoy:			return TEXT("_Convoy");
		case _Attack:			return TEXT("_Attack");
		case _Recovery:			return TEXT("_Recovery");
		case _LayerChange:		return TEXT("_LayerChange");
		}

		return TEXT("Invalid Command");
	}

	TCHAR* PrintAction(unsigned int act)
	{
		switch ( act )
		{
		case Ready:			return TEXT("Ready");
		case TakeOff:		return TEXT("TakeOff");
		case Landing:		return TEXT("Landing");
		case Transfer:		return TEXT("Transfer");
		case Circle:		return TEXT("Circle");
		case Attack:		return TEXT("Attack");
		case Crash:			return TEXT("Crash");
		}

		return TEXT("Invalid Action");
	}
};