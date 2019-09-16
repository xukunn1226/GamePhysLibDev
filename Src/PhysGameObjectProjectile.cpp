#include "..\Inc\PhysXSupport.h"

namespace GPL
{
	bool MagneticTargetSort(const PhysProjectile::MagneticTargetStatus& RefA, const PhysProjectile::MagneticTargetStatus& RefB)
	{
		if (RefA._Distance < RefB._Distance)
			return true;

		return false;
	}

	bool PhysProjectile::Init()
	{
		PhysGameObject::Init();
		_bUtilityTest = false;
		_bVehicleTest = true;
		_TestSimulatedTime = 0;
		InitProData();
		return true;
	}

	void PhysProjectile::InitProData()
	{	
		ProjectileData* Data = DynamicCast(ProjectileData, _ComponentData);
		ProjectileCompDesc* Desc = DynamicCast(ProjectileCompDesc, _ComponentData->_CompDesc);

		_ElapsedSimulatedTime = 0.f;
		_TotalFlightSimulatedTime = 0.f;
		_ElapsedFlightTime = 0.f;
		_HitState = enHitIdle;
		_Step = enIdle;
		_OnWaterTime = 0;
		_FirstValidHitActor = NULL;

		_IntervalSimulatedTime = GetAdaptiveSimulatedTime();

		_CurFlightLoc = (Data != NULL) ? Data->_StartPos : ZERO_VEC;
		_TargetPos = Data->_TargetPos;

		_ValidMagneticTarget.clear();
		_MagneticTestTime = 0.f;
		_MagneticTestInterval = Desc->_MagneticSensorInterval * GLOBAL_DATA_MULTIPLICATOR_TIME;
	}

	void PhysProjectile::InitPhys()
	{
		_PhysActor = NULL;		// 炮弹对象并不创建真实的物理对象
	}

	void PhysProjectile::SetSponsor(PhysGameObject* Sponsor)
	{
		_Sponsor = Sponsor;
	}

	PhysGameObject *PhysProjectile::GetSponsor()
	{
		return _Sponsor;
	}

	NxVec3 PhysProjectile::GetGlobalPosition() const
	{
		return _CurFlightLoc * P2GScale;
	}

	float PhysProjectile::GetPartFlightTime( ETrajectoryMethod method, NxVec3 Gravity, NxVec3 StartPos,NxVec3 InitVelocity,NxVec3 TargetPos )
	{
		ProjectileData* Data = (ProjectileData*)_ComponentData;
		if( !Data )
		{
			return 0.f;
		}

		switch(method)
		{
		case ETM_Parabola:
			{
				float GravityZ = Gravity.z;
				if ( GravityZ == 0.f )
					return 0;

				//	TargetZ = StartZ + VelZ * DeltaTime + 0.5 * GravityZ * DeltaTime * DeltaTime
				//	GravityZ * DeltaTime * DeltaTime + 2 * VelZ * DeltaTime + 2 * (StartZ - TargetZ) = 0
				//	a = GravityZ; b = 2 * VelZ; c = 2 * (StartZ - TargetZ);
				//	DeltaTime1 = (-b + sqrt(b * b - 4 * a * c)) / 2a
				//	DeltaTime2 = (-b - sqrt(b * b - 4 * a * c)) / 2a
				float InitVelZ = InitVelocity.z;
				float Delta = 4 * InitVelZ * InitVelZ - 4 * GravityZ * 2 * (StartPos.z - TargetPos.z);
				if ( Delta < 0.f )
					return 0;		// 无解

				Delta = NxMath::sqrt(Delta);
				float Ret1 = (-2 * InitVelZ + Delta) / (2 * GravityZ);
				float Ret2 = (-2 * InitVelZ - Delta) / (2 * GravityZ);
				float Ret = NxMath::max(Ret1, Ret2);
				return NxMath::max(Ret, 0.f);

				//NxVec3 DeltaPos = TargetPos - StartPos;
				//DeltaPos.z = 0;

				//NxVec3 VelXY = InitVelocity;
				//VelXY.z = 0;
				//VelXY.normalize();

				//return DeltaPos.magnitude() / InitVelocity.dot(VelXY);
			}
			break;
		case ETM_Straight:
			{
				NxVec3 DeltaPos = TargetPos - StartPos;
				float VelSize = InitVelocity.magnitude();
				if( VelSize == 0.f )
					return 0.f;

				return DeltaPos.magnitude() / VelSize;
			}
			break;
		default: 
			break;
		}
		return 0.f;
	}

	NxVec3 PhysProjectile::GetPartFlightLocation( ETrajectoryMethod method,NxVec3 StartPos,NxVec3 InitVelocity,float ElapsedTime )
	{
		NxVec3 Result = StartPos;
		ProjectileData* Data = (ProjectileData*)_ComponentData;
		if( !Data )
		{
			return Result;
		}

		if( method == ETM_Parabola )
		{
			// formula: X(t) = X(0) + V * t + 0.5 * G * t*t;
			Result = StartPos + InitVelocity * ElapsedTime + 0.5f * Data->_Gravity * ElapsedTime * ElapsedTime;
		}
		else if( method == ETM_Straight )
		{
			Result = StartPos + InitVelocity * ElapsedTime;
		}
		return Result;
	}

	NxVec3 PhysProjectile::GetPartFlightVelocity( ETrajectoryMethod method,NxVec3 InitVelocity,float ElapsedTime )
	{
		NxVec3 Velocity = InitVelocity;
		ProjectileData* Data = (ProjectileData*)_ComponentData;
		if( !Data )
		{
			return Velocity;
		}

		if( method == ETM_Parabola )
		{
			//NxVec3 VelZ = InitVelocity.dot(NxVec3(0,0,1)) * NxVec3(0,0,1);
			NxVec3 VelZ = InitVelocity;
			VelZ.x = 0;
			VelZ.y = 0;
			VelZ = VelZ + 0.5f * Data->_Gravity * ElapsedTime * ElapsedTime;

			NxVec3 VelXY = InitVelocity;
			VelXY.z = 0;
			//VelXY.normalize();
			//VelXY = InitVelocity.dot(VelXY) * VelXY;

			Velocity = VelXY + VelZ;
		}
		else if( method == ETM_Straight )
		{
			Velocity = InitVelocity;
		}
		return Velocity;
	}

	float PhysProjectile::GetAdaptiveSimulatedTime()
	{
		if (_bUtilityTest)
		{
			return _TestSimulatedTime;
		}

#ifdef _SERVER_RUNTIME
		return GPhysGameSetting._GPL_PHYS_SERVER_MAX_TIMESTEP;

		/*ProjectileData* Data = (ProjectileData*)_ComponentData;
		if( !Data || Data->_TrajectoryMethod != ETM_Parabola)
		{
			return GPL_PHYS_SERVER_MAX_TIMESTEP;
		}

		float FlightTime = GetFlightTime(Data->_EndPos);
		float MinSimulatedTime = GPL_PHYS_SERVER_MAX_TIMESTEP;
		return NxMath::clamp(FlightTime / GPL_PROJECTILE_TICK_COUNT, GPL_PROJECTILE_SIMULATED_TIME_MAX, MinSimulatedTime);*/
#endif // _SERVER_RUNTIME

#ifdef _CLIENT_RUNTIME
		return GPhysGameSetting._GPL_PHYS_CLIENT_MAX_TIMESTEP;
#endif // _CLIENT_RUNTIME
	}
	
	float PhysProjectile::GetFlightTime( NxVec3 TargetPos )
	{
		ProjectileData* Data = (ProjectileData*)_ComponentData;
		if( !Data )
			return -1.f;
		
		ETrajectoryMethod method = Data->_TrajectoryMethod;
		if (method == ETM_Parabola || method == ETM_Straight)
		{
			return GetPartFlightTime(method, Data->_Gravity, Data->_StartPos, Data->_InitVelocity, TargetPos);
		}
		else if (method == ETM_SectionalV || method == ETM_SectionalH)
		{
			if (_Step == enIdle)
			{
				return GetPartFlightTime(ETM_Parabola, Data->_Gravity, Data->_StartPos, Data->_InitVelocity, TargetPos);
			}
			else if (_Step == enOnWater)
			{
				float fTime = GetPartFlightTime(ETM_Parabola, Data->_Gravity, Data->_StartPos, Data->_InitVelocity, _HitWaterPose);
				NxVec3 velocity = GetBrakingVelocity(method);
				return fTime + GetPartFlightTime(ETM_Straight, Data->_Gravity, _HitWaterPose, velocity, TargetPos);
			}
		}
		return -1.f;
	}

	NxVec3 PhysProjectile::GetFlightLocation( float ElapsedTime )
	{
		NxVec3 Result(0.f);
		ProjectileData* Data = (ProjectileData*)_ComponentData;
		ProjectileCompDesc* CompDesc = (ProjectileCompDesc*)Data->_CompDesc;

		if( !Data ||!CompDesc)
			return Result;

		ETrajectoryMethod method = Data->_TrajectoryMethod;
		if (method == ETM_Parabola || method == ETM_Straight)
		{
			return GetPartFlightLocation(method, Data->_StartPos, Data->_InitVelocity, ElapsedTime);
		}
		else if (method == ETM_SectionalV || method == ETM_SectionalH)
		{
			if (_Step == enIdle)
			{
				return GetPartFlightLocation(ETM_Parabola, Data->_StartPos, Data->_InitVelocity, ElapsedTime);
			}
			else if (_Step == enOnWater)
			{
				float fTime = GetPartFlightTime(ETM_Parabola, Data->_Gravity, Data->_StartPos,Data->_InitVelocity,_HitWaterPose);
				NxVec3 velocity = GetBrakingVelocity(method);
				return GetPartFlightLocation(ETM_Straight, _HitWaterPose, velocity, ElapsedTime-fTime);
			}
		}
		return Result;
	}

	NxVec3 PhysProjectile::GetFlightVelocity( float ElapsedTime )
	{
		NxVec3 Result(0.f);
		ProjectileData* Data = (ProjectileData*)_ComponentData;
		ProjectileCompDesc* CompDesc = (ProjectileCompDesc*)Data->_CompDesc;

		if( !Data ||!CompDesc)
		{
			return Result;
		}
		ETrajectoryMethod method = Data->_TrajectoryMethod;
		if (method==ETM_Parabola||method==ETM_Straight)
		{
			return GetPartFlightVelocity(method,Data->_InitVelocity,ElapsedTime);
		}
		else if (method==ETM_SectionalV||method==ETM_SectionalH)
		{
			if (_Step==enIdle)
			{
				return GetPartFlightVelocity(ETM_Parabola,Data->_InitVelocity,ElapsedTime);
			}
			else if (_Step==enOnWater)
			{
				NxVec3 velocity =GetBrakingVelocity(method);
				return velocity;
			}
		}
		return Result;
	}

	NxVec3 PhysProjectile::GetBrakingVelocity( ETrajectoryMethod method )
	{
		ProjectileData* Data = (ProjectileData*)_ComponentData;
		ProjectileCompDesc* CompDesc = (ProjectileCompDesc*)Data->_CompDesc;

		if( !Data ||!CompDesc)
		{
			return NxVec3(0.f);
		}

		return Data->_BrakingVelocity;
	}

	ETrajectoryMethod PhysProjectile::GetTrajectoryMethod()
	{
		ProjectileData* pData = DynamicCast(ProjectileData, _ComponentData);
		if ( !pData )
			return ETM_Invalid;

		return pData->_TrajectoryMethod;
	}

	NxVec3 PhysProjectile::GetTargetPos()
	{
		return _TargetPos;
	}

	NxVec3 PhysProjectile::GetTowards()
	{
		ProjectileData* pData = DynamicCast(ProjectileData, _ComponentData);
		if ( !pData )
			return ZERO_VEC;

		NxVec3 Towards;
		switch (pData->_TrajectoryMethod)
		{
		case ETM_Parabola:
			Towards = GetFlightVelocity(_ElapsedFlightTime);
			break;
		case ETM_Straight:
			Towards = pData->_InitVelocity;
			break;
		case ETM_SectionalH:
			Towards = pData->_BrakingVelocity;
			break;
		case ETM_SectionalV:
			Towards = pData->_BrakingVelocity;
			break;
		default:
			return ZERO_VEC;
		}

		Towards.normalize();
		return Towards;
	}

	float PhysProjectile::GetBurstDistance( const NxVec3 &BurstPos,PhysVehicle* ship )
	{		
		ProjectileData* Data = (ProjectileData*)_ComponentData;
		float Distance=Data->_BurstRange*G2PScale;
		if(!ship)
		{
			return Distance;
		}
		Socket BurstSocketList[VEHICLE_BURST_POINT_MAX];
		int BurstPointNum = ship->GetBurstPointNum(BurstSocketList);

		for (int n=0;n<BurstPointNum;n++)
		{
			NxMat34 LocalPos=BurstSocketList[n]._LocalPose;
			NxMat34 GlobalPos = ship->GetGlobalPos() * LocalPos;

			NxU32 groups = (1 << GPL_SHAPE_GROUP_VEHICLE_SHIP);
			NxRay WorldRay;
			WorldRay.orig = BurstPos;
			WorldRay.dir = GlobalPos.t - BurstPos;
			WorldRay.dir.normalize();

			//if (!NxMath::equals(WorldRay.dir.magnitude(), 1.f, 0.0001f))
			//	gplDebugf(TEXT("PhysProjectile::GetBurstDistance NxRay dir warning"));

			NxRaycastHit Hit;
			if(_Scene->_PhysScene->raycastClosestShape(WorldRay, NX_DYNAMIC_SHAPES, Hit, groups, (GlobalPos.t - BurstPos).normalize() ))
			{
				float dis=(Hit.worldImpact-BurstPos).normalize();
				if (dis<Distance)
				{
					Distance=dis;
				}
			}
		}
		return Distance;
	}

	// 检测到任何shape（_Sponsor除外）即停止继续检测
	bool PhysProjectile::ProjectileRaycastReport::onHit(const NxRaycastHit& hits)
	{
		if (!_ValidHit)
		{
			*_Hit = hits;
			if ( hits.shape != NULL )
			{
				PhysGameObject* HitActor = (PhysGameObject*)(hits.shape->getActor().userData);
				if (HitActor != _Sponsor)
				{
					_ValidHit=true;
					return false;
				}
			}
			return true;
		}

		return false;
	}

	void PhysProjectile::Tick(float DeltaTime)
	{
		if( _bForceSelfDestruction )
			return;

		ProjectileData* CompData = (ProjectileData*)_ComponentData;
		if ( !CompData )
			return ;

		ProjectileCompDesc* CompDesc = (ProjectileCompDesc*)CompData->_CompDesc;
		if ( CompDesc == NULL )
			return;

		float CurTotalFlightSimulatedTime = _TotalFlightSimulatedTime;
		if ( _HitState == enHitIdle )
		{ 
			if (CurTotalFlightSimulatedTime > CompDesc->_MaxFlightTime)
			{
				// 超最长飞行，destroy it
				HitNotifyInfo info;
				info._HitNormal = AXIS_VEC_Z;
				info._HitPos = GetFlightLocation(CurTotalFlightSimulatedTime);
				info._HitDirection = GetFlightVelocity(CurTotalFlightSimulatedTime);
				info._HitDirection.normalize();
				_HitState = enSelfDestory;
				PostBurstMsg(info._HitPos, info._HitNormal, info._HitDirection);			
				return;
			}
			
			if( CompData->_BurstTime > 0.f && CurTotalFlightSimulatedTime > CompData->_BurstTime)
			{
				// 超爆炸时间，destroy it
				HitNotifyInfo info;
				info._HitNormal = AXIS_VEC_Z;
				info._HitPos = GetFlightLocation(CompData->_BurstTime);
				info._HitDirection = GetFlightVelocity(CompData->_BurstTime);
				info._HitDirection.normalize();

				_HitState = enSelfDestory;
				CheckBurstHit(info._HitPos, info._HitNormal, info._HitDirection, NULL, NULL);			
				return;
			}		
		}

		_TotalFlightSimulatedTime += DeltaTime;
		_ElapsedSimulatedTime += DeltaTime;

		if( _ElapsedSimulatedTime > _IntervalSimulatedTime )
		{
			int CallCount = NxMath::trunc(_ElapsedSimulatedTime / _IntervalSimulatedTime);
			while(CallCount-- && _HitState == enHitIdle)
			{
				int AlignedCount = NxMath::trunc(CurTotalFlightSimulatedTime / _IntervalSimulatedTime);
				if( CallCount == 0 )
				{
					CurTotalFlightSimulatedTime = AlignedCount * _IntervalSimulatedTime + _ElapsedSimulatedTime;
				}
				else
				{
					CurTotalFlightSimulatedTime = (1 + AlignedCount + 0.001f) * _IntervalSimulatedTime;
				}
				_ElapsedSimulatedTime -= _IntervalSimulatedTime;

				NxVec3 PendingFlightLoc = GetFlightLocation(CurTotalFlightSimulatedTime);
				
				// 累计水面飞行时间
				if ( _Step == enOnWater )
					_OnWaterTime += _IntervalSimulatedTime;

				if( _HitState == enHitIdle )
				{
					// 没有命中
					NxVec3 HitLoc, HitNormal;
					bool bHit = IsHitObject(_CurFlightLoc, PendingFlightLoc);
					// 若命中，记录命中点；否则记录当前点
					if( bHit )
					{
						_HitState = enHit;
					}
					else
					{
						NxVec3 FlightDir = PendingFlightLoc - _CurFlightLoc;
						FlightDir.normalize();

						_CurFlightLoc = PendingFlightLoc;
						_ElapsedFlightTime = CurTotalFlightSimulatedTime;

						//	未直接命中的情况下执行磁感应引信检测
						if (MagneticBurstTest(DeltaTime))
						{
							_HitState = enHit;
							PostBurstMsg(_CurFlightLoc, AXIS_VEC_Z, FlightDir);
						}
					}

					//if ( !_bUtilityTest )
					//	gplDebugf(TEXT("[%.4f] PendingFlightLoc: %.2f %.2f %.2f"), _TotalFlightSimulatedTime, _CurFlightLoc.x, _CurFlightLoc.y, _CurFlightLoc.z);
				}
			}		
		}
	}

	bool PhysProjectile::CheckChangeTrajectory( PhysGameObject * HitObj ,const NxRaycastHit &HitInfo)
	{
		ProjectileData* CompData = (ProjectileData*)_ComponentData;
		if ( !CompData )
			return false;

		ETrajectoryMethod	method=CompData->_TrajectoryMethod;
		if ( method == ETM_SectionalV || method == ETM_SectionalH )
		{
			if ( _Step == enIdle )
			{
				PhysOcean* pOcean = DynamicCast(PhysOcean, HitObj);
				if ( pOcean != NULL )
				{
					_Step = enOnWater;
					_HitWaterPose = HitInfo.worldImpact;

					//gplDebugf(TEXT("炮弹落水，_ElapsedFlightTime[%f] _HitWaterPose[%f %f %f]"), _ElapsedFlightTime, _HitWaterPose.x, _HitWaterPose.y, _HitWaterPose.z);
					if ( _BehaviorReport )
						_BehaviorReport->OnProjectileHitWater(this,HitInfo.worldImpact,HitInfo.worldNormal);

					return true;
				}
			}		
		}
		return false;
	}

	bool PhysProjectile::SweepObject(const NxVec3 & StartPos,const NxVec3 &EndPos )
	{
		ProjectileData* CompData = DynamicCast(ProjectileData, _ComponentData);
		ProjectileCompDesc* CompDesc = DynamicCast(ProjectileCompDesc, _ComponentData->_CompDesc);
		if (NULL == CompData || NULL == CompDesc)
			return false;

#ifdef _CLIENT_RUNTIME
		//	[8/3/2015 chenpu]	试验客户端鱼雷不检测船，结果以服务器为准
		if (CompData->_TrajectoryMethod == ETM_SectionalH)
			return false;
#endif // _CLIENT_RUNTIME

		//  [8/20/2014 chenpu]	海水半透明化以后空气墙范围扩大的对应策略，SweepObject只检测船
		NxU32 groups = 0;
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_VEHICLE_SHIP);

		NxU32 flags = NX_SF_STATICS|NX_SF_DYNAMICS|NX_SF_ALL_HITS;
		NxSweepQueryHit result[20];		// 效率考虑，仅检测最多20个
		float HitRadius = CompDesc->_HitRange * G2PScale;
		NxVec3 TestStart = StartPos;
		NxVec3 TestEnd = EndPos;

		NxVec3 HitDirection = EndPos - StartPos;
		float MoveDelta = HitDirection.normalize();
		if (CompData->_TrajectoryMethod == ETM_SectionalH)//鱼雷
		{
			NxVec3 dir = EndPos - StartPos;
			dir.z = 0;
			dir.normalize();
			NxMat33 rot;
			rot.rotZ(atan2(dir.y,dir.x));

			float CheckHeight = GPhysGameSetting._GPL_WaterLayerOneHeight * G2PScale + 2 * HitRadius;
			NxVec3 TestCenter = (EndPos + StartPos) / 2;
			TestCenter.z -= CheckHeight / 2;

			NxBox TestBox(TestCenter, NxVec3(MoveDelta / 2 + HitRadius, HitRadius, CheckHeight / 2), rot);
			
			if (_Scene->_PhysScene->checkOverlapOBB(TestBox, NX_ALL_SHAPES, groups))
			{
				TestStart.z = TestCenter.z;
				TestEnd.z = TestCenter.z;
				NxBox SweepBox(TestStart, NxVec3(HitRadius, HitRadius, CheckHeight / 2), rot);
				NxU32 NumShapes = _Scene->_PhysScene->linearOBBSweep(SweepBox, TestEnd-TestStart, flags, NULL, 20, &result[0], NULL,groups);

				NxShape *hitShape = NULL;
				ShipComponent* HitComp = NULL;
				PhysGameObject* HitActor = NULL;
				bool bret = false;
				NxU32 i = 0;
				for(i = 0; i < NumShapes; ++i)
				{
					hitShape = result[i].hitShape;
					HitActor=(PhysGameObject *)hitShape->getActor().userData;
					HitComp = (ShipComponent*)hitShape->userData;
					bret = (HitActor != NULL && HitActor != _Sponsor);
					if( bret )
						break;		// 一旦发现可命中目标，即跳出
				}

				if (bret)
				{
					// 直接命中通知,仅命中舰船或舰载机时发送通知
					NxVec3 CheckPoint=TestStart+result[i].t*(TestEnd-TestStart);
					CheckPoint.z=result[i].point.z;		// 计算炮弹爆炸点
					if( HitComp != NULL )
					{
						SendStrikeMsg(HitComp, result[i].normal, result[i].point, NULL, HitRadius, true);
					}
					else
					{
						SendStrikeMsg(HitActor, result[i].normal, result[i].point, NULL, HitRadius, true);
					}
					
					CheckBurstHit(CheckPoint,result[i].normal, HitDirection, HitComp, HitActor);
					
					return true;
				}
			}
		}
		else
		{ // 除鱼雷外的其他炮弹
			groups |= (1 << GPL_SHAPE_GROUP_AIRCRAFT);
			NxCapsule TestCapsule = NxCapsule(NxSegment(TestStart,TestEnd),HitRadius);

			if (_Scene->_PhysScene->checkOverlapCapsule(TestCapsule,NX_ALL_SHAPES,groups))
			{
				NxCapsule SweepCapsule = NxCapsule(NxSegment(TestStart,TestStart),HitRadius);
				NxU32 NumShapes = _Scene->_PhysScene->linearCapsuleSweep(SweepCapsule, TestEnd-TestStart, flags, NULL, 20, &result[0], NULL,groups);

				NxShape *hitShape = NULL;
				ShipComponent* HitComp = NULL;
				PhysGameObject *HitActor = NULL;
				bool bret = false;
				NxU32 i = 0;
				for(i = 0; i < NumShapes; ++i)
				{
					hitShape = result[i].hitShape;
					HitActor=(PhysGameObject *)hitShape->getActor().userData;
					HitComp = (ShipComponent*)hitShape->userData;
					
					bret = (HitActor != NULL);			
					if( bret )
					{
						PhysVehicle* HitVehicle = DynamicCast(PhysVehicle, HitActor);
						if( HitVehicle != NULL )
						{
							if ( HitVehicle == _Sponsor )
							{
								//	碰撞Sponsor，判定未命中
								bret = false;
							}
							else if ( CompData->_TrajectoryMethod == ETM_SectionalV )
							{
								//	深水炸弹碰撞非水下单位时，判定未命中
								if ( !HitVehicle->IsSubmarine() )
									bret = false;
								else if ( HitVehicle->_CurLevel == 0 )
									bret = false;
							}
						}
					}

					if( bret )
						break;			// 一旦发现可命中目标，即跳出
				}				

				if (bret)
				{
					// 直接命中通知,仅命中舰船或舰载机时发送通知
					NxVec3 CheckPoint=TestStart+result[i].t*(TestEnd-TestStart);
					CheckPoint.z=result[i].point.z;		// 计算炮弹爆炸点
					if( HitComp != NULL )
					{
						SendStrikeMsg(HitComp, result[i].normal, result[i].point, NULL, HitRadius, true);
					}
					else
					{
						SendStrikeMsg(HitActor, result[i].normal, result[i].point, NULL, HitRadius, true);
					}
					
					CheckBurstHit(CheckPoint,result[i].normal, HitDirection, HitComp, HitActor);
					
					return true;
				}		
			}
		}
	
		return false;
	}

	bool PhysProjectile::IsHitObject(NxVec3 StartPos, NxVec3 EndPos)
	{
		ProjectileData* CompData = DynamicCast(ProjectileData, _ComponentData);
		if ( !CompData )
			return false;

		ProjectileCompDesc* CompDesc = DynamicCast(ProjectileCompDesc, CompData->_CompDesc);
		if( !CompDesc )
			return false;

		NxVec3 HitDirection = EndPos - StartPos;
		HitDirection.normalize();

		bool bRet = false;
#if 0
		switch ( CompData->_TrajectoryMethod )
		{
		case ETM_SectionalV:
		case ETM_SectionalH:
			{
				if ( _Step == enIdle )
				{
					//	鱼雷、深水炸弹抛射阶段，不检查是否与舰船碰撞
					NxRaycastHit Hit;
					PhysGameObject* HitActor = NULL;
					ShipComponent* HitComp = NULL;
					raycastObject(StartPos, EndPos, Hit, HitComp, HitActor, false, false, false);

					CheckChangeTrajectory(HitActor,Hit);
				}
				else if ( _Step == enOnWater )
				{
					//	鱼雷、深水炸弹在落入水面后使用SWEEP检测碰撞
					bRet = SweepObject(StartPos, EndPos);
				} 
			}
			break;

		default:
			{
				if ( CompDesc->_HitRange > 0.f && !_bUtilityTest )
				{
					bRet = SweepObject(StartPos, EndPos);
				}
				else
				{
					NxRaycastHit Hit;
					PhysGameObject* HitActor = NULL;
					ShipComponent* HitComp = NULL;
					raycastObject(StartPos, EndPos, Hit, HitComp, HitActor, false, true, (CompData->_TrajectoryMethod == ETM_Straight));
					if ( HitActor )
					{
						//	命中动态物体
						if( _Owner != NULL && HitActor == _Owner )
						{
							raycastObject(StartPos, EndPos, Hit, HitComp, HitActor, true, true, (CompData->_TrajectoryMethod == ETM_Straight));

							//	仍命中舰船本身
							if( HitActor != NULL && HitActor == _Owner )
								return false;
						}

						bRet = true;
						if( HitComp != NULL || IsKind(PhysVehicle, HitActor) || IsKind(PhysAircraft, HitActor) )
						{
							//	命中舰船
							GameObject* StrikeComp = (HitComp != NULL ? HitComp : (GameObject*)HitActor);
							if ( SendStrikeMsg(StrikeComp, Hit.worldNormal, Hit.worldImpact, NULL, 0.f, true) )
							{
								// 命中舰船继续检测穿透
								NxRaycastHit Hit2;
								PhysGameObject* HitActor2 = NULL;
								ShipComponent* HitComp2 = NULL;

								StartPos = Hit.worldImpact;
								NxVec3 Dir = (EndPos - StartPos);
								Dir.normalize();
								StartPos += Dir * 0.1f;

								raycastObject(StartPos,EndPos,Hit2,HitComp2,HitActor2, false, true, (method==ETM_Straight));
								if (HitComp2 != HitComp && HitActor==HitActor2 && HitActor2!=_Owner)
								{
									GameObject* StrikeComp2 = (HitComp2 != NULL ? HitComp2 : (GameObject*)HitActor2);
									SendStrikeMsg(StrikeComp2,Hit2.worldNormal, Hit2.worldImpact, NULL, 0.f, true);
								}						
							}
						}

						// 计算范围爆炸伤害
						CheckBurstHit(Hit.worldImpact, Hit.worldNormal, HitDirection, HitComp, HitActor);
					}
				}
			}
			break;
		}
#else
		ETrajectoryMethod method = CompData->_TrajectoryMethod;
		if( ((method==ETM_SectionalV||method==ETM_SectionalH)&&_Step==enOnWater) 
			|| (method != ETM_SectionalV && method != ETM_SectionalH && CompDesc->_HitRange > 0.f && !_bUtilityTest) )
		{
			//	鱼雷、深水炸弹在落入水面后使用SWEEP检测碰撞
			bRet = SweepObject(StartPos, EndPos);

			//  [8/20/2014 chenpu]	海水半透明化以后空气墙范围扩大的对应策略，追加raycastObject检测地形
			if (!bRet)
			{
				NxRaycastHit Hit;
				PhysGameObject* HitActor = NULL;
				ShipComponent* HitComp = NULL;
				raycastObject(StartPos, EndPos, Hit, HitComp, HitActor, false, false, false);
				if (NULL != HitActor && !IsKind(PhysOcean, HitActor))
				{
					SendStrikeMsg(HitComp != NULL ? HitComp : (GameObject*)HitActor, Hit.worldNormal, Hit.worldImpact, NULL, 0.f, true);
					PostBurstMsg(Hit.worldImpact, Hit.worldNormal, HitDirection);
					return true;
				}
			}
		}
		else if( (method==ETM_SectionalV||method==ETM_SectionalH) && _Step==enIdle )
		{
			//	鱼雷、深水炸弹抛射阶段
			NxRaycastHit Hit;
			PhysGameObject* HitActor = NULL;
			ShipComponent* HitComp = NULL;
			raycastObject(StartPos, EndPos, Hit, HitComp, HitActor, false, true, false);

			if (NULL != HitActor && !IsKind(PhysOcean, HitActor))
			{
				if (HitActor != _Sponsor)
				{
					SendStrikeMsg(HitComp != NULL ? HitComp : (GameObject*)HitActor, Hit.worldNormal, Hit.worldImpact, NULL, 0.f, true);
					PostBurstMsg(Hit.worldImpact, Hit.worldNormal, HitDirection);
					return true;
				}
				
				//	抛射阶段检测到自身舰船，需要重新检测与海面的碰撞
				raycastObject(StartPos, EndPos, Hit, HitComp, HitActor, true, false, false);
			}

			CheckChangeTrajectory(HitActor,Hit);
		}
		else
		{
			NxRaycastHit Hit;
			PhysGameObject* HitActor = NULL;
			ShipComponent* HitComp = NULL;
			if ( !_bUtilityTest )
				raycastObject(StartPos, EndPos, Hit, HitComp, HitActor, false, true, (method == ETM_Straight));
			else
			{
				raycastObject(StartPos, EndPos, Hit, HitComp, HitActor, false, _bVehicleTest, (method == ETM_Straight));

				PhysVehicle* HitVehicle = DynamicCast(PhysVehicle, HitActor);
				if ( HitVehicle != NULL && !HitVehicle->IsDetectable() )
					raycastObject(StartPos, EndPos, Hit, HitComp, HitActor, false, false, (method == ETM_Straight));
			}

			if ( NULL != HitActor )
			{
				//	命中动态物体
				if ( NULL != _Sponsor && HitActor == _Sponsor )
				{
					raycastObject(StartPos, EndPos, Hit, HitComp, HitActor, true, true, (method == ETM_Straight));

					//	仍命中舰船本身
					if ( HitActor != NULL && HitActor == _Sponsor )
						return false;
				}

				bRet = true;
				PhysGameObject* FinalHitActor = HitActor;
				ShipComponent* FinalHitComp = HitComp;
				if( HitComp != NULL || IsKind(PhysVehicle, HitActor) || IsKind(PhysAircraft, HitActor) )
				{
					////////////
					GameObject* StrikeComp = (HitComp != NULL ? HitComp : (GameObject*)HitActor);
					bool bContinueStrike = SendStrikeMsg(StrikeComp, Hit.worldNormal, Hit.worldImpact, NULL, 0.f, true);
					//gplDebugf(TEXT("命中结果[%d] 部件TYPE[%d]"), bContinueStrike, 
					//	HitComp == NULL ? 0 : ((VehicleAttachedCompData*)HitComp->_ComponentData)->_PartType);

					NxRaycastHit Hit2 = Hit;
					PhysGameObject* HitActor2 = NULL;
					ShipComponent* HitComp2 = NULL;
					int MaxIteratorCount = 5;
					while( bContinueStrike && MaxIteratorCount > 0 )
					{
						--MaxIteratorCount;

						NxVec3 StartPos2 = Hit2.worldImpact;
						StartPos2 += HitDirection * 0.1f;
						NxVec3 EndPos2 = StartPos2 + HitDirection * 10.f;

						raycastObject(StartPos2, EndPos2, Hit2, HitComp2, HitActor2, false, true, (method==ETM_Straight));

						// 若继续穿透的命中结果与之前不同，直接跳出
						if( HitActor != HitActor2 )
						{
							break;
						}

						// 此次命中的部件与上次不同
						if (HitComp2 != HitComp && HitActor2!=_Sponsor)
						{
							GameObject* StrikeComp2 = (HitComp2 != NULL ? HitComp2 : (GameObject*)HitActor2);
							bContinueStrike = SendStrikeMsg(StrikeComp2,Hit2.worldNormal, Hit2.worldImpact, NULL, 0.f, true);

							HitComp = HitComp2;
							HitActor = HitActor2;
							//gplDebugf(TEXT("穿透命中结果 部件TYPE[%d]"),
							//	HitComp == NULL ? 0 : ((VehicleAttachedCompData*)HitComp->_ComponentData)->_PartType);
						}
					}




					//	命中舰船
					//GameObject* StrikeComp = (HitComp != NULL ? HitComp : (GameObject*)HitActor);
					//if ( SendStrikeMsg(StrikeComp, Hit.worldNormal, Hit.worldImpact, NULL, 0.f, true) )
					//{
					//	// 命中舰船继续检测穿透
					//	NxRaycastHit Hit2;
					//	PhysGameObject* HitActor2 = NULL;
					//	ShipComponent* HitComp2 = NULL;

					//	StartPos = Hit.worldImpact;
					//	NxVec3 Dir = (EndPos - StartPos);
					//	Dir.normalize();
					//	StartPos += Dir * 0.1f;

					//	raycastObject(StartPos,EndPos,Hit2,HitComp2,HitActor2, false, true, (method==ETM_Straight));
					//	if (HitComp2 != HitComp && HitActor==HitActor2 && HitActor2!=_Sponsor)
					//	{
					//		GameObject* StrikeComp2 = (HitComp2 != NULL ? HitComp2 : (GameObject*)HitActor2);
					//		SendStrikeMsg(StrikeComp2,Hit2.worldNormal, Hit2.worldImpact, NULL, 0.f, true);
					//	}						
					//}
				}
				// 计算范围爆炸伤害
				CheckBurstHit(Hit.worldImpact, Hit.worldNormal, HitDirection, FinalHitComp, FinalHitActor);
			}
		}
#endif
		return bRet;
	}

	bool PhysProjectile::MagneticBurstTest(float DeltaTime)
	{
		ProjectileData* ProjData = DynamicCast(ProjectileData, _ComponentData);
		if (!ProjData)
			return false;

		ProjectileCompDesc* CompDesc = DynamicCast(ProjectileCompDesc, _ComponentData->_CompDesc);
		if (!CompDesc)
			return false;

		if (CompDesc->_MagneticSensorInterval <= 0)
			return false;

		if (!_BehaviorReport)
			return false;

		_MagneticTestTime += DeltaTime;
		if (_MagneticTestTime >= _MagneticTestInterval)
		{
			_MagneticTestTime -= _MagneticTestInterval;

			const int NumOverlapShapes = 20;
			NxShape* shapes [NumOverlapShapes];
			NxSphere TriggerBall(_CurFlightLoc, ProjData->_MagneticSensorRange);
			NxU32 groups = 0;
			ENABLE_STATE(groups, GPL_SHAPE_GROUP_VEHICLE_SHIP);

			//	检测前将已跟踪目标全部置为丢失状态
			MagneticTestResult::iterator iter;
			for (iter = _ValidMagneticTarget.begin(); iter != _ValidMagneticTarget.end(); iter++)
				iter->_Lost = true;

			if (_Scene->_PhysScene->checkOverlapSphere(TriggerBall, NX_DYNAMIC_SHAPES, groups))
			{
				NxU32 nbShapes = _Scene->_PhysScene->overlapSphereShapes(TriggerBall, NX_DYNAMIC_SHAPES, NumOverlapShapes, &shapes[0], NULL, groups);
				for (NxU32 i = 0; i < nbShapes; i++)
				{
					NxShape* shape = shapes[i];
					//ShipComponent* PhysComp = (ShipComponent*)shape->userData;
					PhysGameObject* PhysObject = (PhysGameObject*)shape->getActor().userData;
					if (PhysObject == _Sponsor)
						continue;

					float fDist = shape->getGlobalPosition().distance(TriggerBall.center);

					bool HasTraced = false;
					for (iter = _ValidMagneticTarget.begin(); iter != _ValidMagneticTarget.end(); iter++)
					{
						//	距离减小，更新与追踪目标的距离
						if (iter->_Target == PhysObject)
						{
							if (fDist < iter->_Distance)
							{
								iter->_Distance = fDist;
								iter->_Lost = false;
							}
							HasTraced = true;
							break;
						}
					}
					
					//	新增目标
					if (!HasTraced)
					{
						MagneticTargetStatus MTS;
						MTS._Target = PhysObject;
						MTS._Distance = fDist;
						MTS._Lost = false;
						_ValidMagneticTarget.push_back(MTS);
					}
				}
			}

			//	安全时间内检测，但不引爆
			if (ProjData->_SafeTime > _OnWaterTime)
				return false;

			//	由近及远排序
			if (_ValidMagneticTarget.size() > 1)
				std::stable_sort(_ValidMagneticTarget.begin(), _ValidMagneticTarget.end(), MagneticTargetSort);

			//	对符合条件的跟踪目标进行回调，由逻辑层决定是否引爆（视配置而定，或需排除队友）
			for (iter = _ValidMagneticTarget.begin(); iter != _ValidMagneticTarget.end(); iter++)
			{
				if (!iter->_Lost)
					continue;
				
				if (_BehaviorReport->OnProjectileMagneticTestReport(this, iter->_Target))
					return true;
			}
		}

		return false;
	}
	
	// 范围伤害计算
	// 若IngoreComponent!=NULL,表明命中舰船部件，此时忽略IngoreActor
	// 若IngoreComponent==NULL，且IgnoreActor!=NULL,表明命中舰船本身、舰载机、静态物体等
	void PhysProjectile::CheckBurstHit(NxVec3 pos,NxVec3 normal, NxVec3 HitDirection, ShipComponent* IngoreComponent, PhysGameObject* IngoreActor)
	{
		if (_bUtilityTest)
		{
			PostBurstMsg(pos,normal, HitDirection);
			return ;
		}
		
		float BurstRange=((ProjectileData*)_ComponentData)->_BurstRange * G2PScale;
		if (BurstRange > 0)
		{
			NxSphere worldSphere(pos,BurstRange );
			const int NumOverlapShapes = 20;
			NxShape* shapes [NumOverlapShapes];
			NxU32 nbShapes = _Scene->_PhysScene->overlapSphereShapes(worldSphere, NX_DYNAMIC_SHAPES, NumOverlapShapes, &shapes[0], NULL, 1<<GPL_SHAPE_GROUP_VEHICLE_SHIP | 1<<GPL_SHAPE_GROUP_AIRCRAFT);

			std::set<GameObject*> Components;	// 范围爆炸造成伤害的GameObject
			if( ((ProjectileData*)_ComponentData)->_TrajectoryMethod == ETM_SectionalV )
			{ // 深水炸弹
				for (NxU32 i=0;i<nbShapes;i++)
				{
					NxShape *shape=shapes[i];
					ShipComponent *PhysComp = (ShipComponent*)shape->userData;		// 仅检测舰船及部件
					PhysGameObject* PhysObject = (PhysGameObject*)shape->getActor().userData;
					PhysVehicle* HitVehicle = DynamicCast(PhysVehicle, PhysObject);

					if( HitVehicle != NULL && HitVehicle->IsSubmarine() && HitVehicle->_CurLevel < 0 )
					{ // 深水炸弹特殊处理，仅能对下潜的潜艇造成伤害
						if( PhysComp != NULL && PhysComp != IngoreComponent )
						{
							Components.insert(PhysComp);
						}
						if( PhysObject != NULL && PhysObject != IngoreActor )
						{
							Components.insert(PhysObject);
						}
					}
				}
			}
			else
			{
				for (NxU32 i=0;i<nbShapes;i++)
				{
					NxShape *shape=shapes[i];
					ShipComponent* PhysComp = (ShipComponent*)shape->userData;		// 仅检测舰船及部件
					PhysGameObject* PhysObject = (PhysGameObject*)shape->getActor().userData;
					
					if ( PhysComp != NULL && PhysComp != IngoreComponent )
					{
						Components.insert(PhysComp);
					}
					if( PhysObject != NULL && PhysObject != IngoreActor )
					{
						Components.insert(PhysObject);
					}
				}
			}

			std::set<GameObject*>::iterator iter = Components.begin();
			for (;iter!=Components.end();iter++)
			{				
				PhysGameObject* Vehicle = NULL;
				if( IsKind(ShipComponent, (*iter)) )
				{
					Vehicle = ((ShipComponent*)(*iter))->GetOwner();
				}
				if( IsKind(PhysVehicle, (*iter)) )
				{
					Vehicle = (PhysGameObject*)(*iter);
				}
				if( IsKind(PhysAircraft, (*iter)) )
				{
					Vehicle = (PhysAircraft*)(*iter);
				}
				SendStrikeMsg(*iter, normal, pos, Vehicle, 0.f, false);		// 非直接命中
			}
			Components.clear();
		}
		
		// 告知最后爆炸点
		PostBurstMsg(pos,normal, HitDirection);	
	}

	void PhysProjectile::raycastObject(const NxVec3& StartPos, const NxVec3& EndPos, NxRaycastHit& Hit, ShipComponent*& HitComp, PhysGameObject*& HitActor, bool RaycastAll/* =false */, bool bCheckVehicle /* = true */, bool bCheckAircraft /* = true */)
	{
		HitComp = NULL;
		HitActor = NULL;

		NxShape*HitShape=NULL;
		NxRay WorldRay;
		WorldRay.orig = StartPos;
		WorldRay.dir = EndPos - StartPos;
		NxReal MaxDis = WorldRay.dir.normalize();

		//if (!NxMath::equals(WorldRay.dir.magnitude(), 1.f, 0.0001f))
		//	gplDebugf(TEXT("PhysProjectile::raycastObject NxRay dir warning"));

		NxU32 groups = 0;
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_OCEAN);
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_TERRAIN);
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_STATICOBJECT);

		if( bCheckVehicle )
			ENABLE_STATE(groups, GPL_SHAPE_GROUP_VEHICLE_SHIP);

		if( bCheckAircraft )
			ENABLE_STATE(groups, GPL_SHAPE_GROUP_AIRCRAFT);

		if( _Scene->_PhysScene->raycastAnyShape(WorldRay, NX_ALL_SHAPES, groups, MaxDis) )
		{	
			if (RaycastAll)
			{
				ProjectileRaycastReport report(_Sponsor, &Hit);
				_Scene->_PhysScene->raycastAllShapes(
					WorldRay, report,
					NX_ALL_SHAPES, 
					groups, 
					MaxDis, 0xffffffff );
				HitShape = Hit.shape;
			}
			else
			{
				HitShape = _Scene->_PhysScene->raycastClosestShape(
					WorldRay, 
					NX_ALL_SHAPES, 
					Hit, 
					groups, 
					MaxDis, 0xffffffff );
			}			
		}

		if ( HitShape != NULL )
		{
			HitActor = (PhysGameObject*)(HitShape->getActor().userData);
			HitComp = (ShipComponent *)HitShape->userData;
		}
	}

	// 发送命中通知
	// HitComp最小命中单位（部件，舰船，舰载机等）
	bool  PhysProjectile::SendStrikeMsg(GameObject* HitComp, 
										NxVec3 StrikeNormal, 
										NxVec3 StrikePos, 
										PhysGameObject* Vehicle, 
										float Distance, 
										bool bDirectHit)
	{
		if (!_FirstValidHitActor)
		{
			_FirstValidHitActor = IsKind(ShipComponent, HitComp) 
								? ((ShipComponent*)HitComp)->GetOwner() 
								: DynamicCast(PhysGameObject, HitComp);
		}

		if (_bUtilityTest)
			return false;
		
		ProjectileData* Data = (ProjectileData*)_ComponentData;
		if ( !Data )
			return false;

		if ( _BehaviorReport )
		{
			DamageInfo info;

			float SafeTime;
			if (Data->_TrajectoryMethod == ETM_SectionalV || Data->_TrajectoryMethod == ETM_SectionalH)
			{
				if (_Step == enIdle)
				{
					//	鱼雷、深水炸弹抛射阶段视作安全时间内处理
					info._SafeTime = true;
				}
				else
				{
					//	入水后正常计算安全时间
					SafeTime = _OnWaterTime;
					info._SafeTime = Data->_SafeTime > SafeTime;
				}
			}
			else
			{
				SafeTime = _ElapsedFlightTime;
				info._SafeTime = Data->_SafeTime > SafeTime;
			}
			
			info._HitPos = StrikePos;

			// 直接命中取Distance，否则根据炮弹爆炸点与舰船上预设爆炸点连线计算Distance
			info._Distance = Distance;
			if( !bDirectHit )
			{
				PhysVehicle* Ship = DynamicCast(PhysVehicle,Vehicle);
				if( Ship != NULL )
				{
					info._Distance = GetBurstDistance(StrikePos,Ship);
				}

				PhysAircraft* Aircraft = DynamicCast(PhysAircraft, Vehicle);
				if( Aircraft != NULL )
				{
					info._Distance = NxMath::min(Data->_BurstRange * G2PScale, (StrikePos - Aircraft->GetGlobalPosition()).normalize());
				}
			}

			info._Obj = HitComp;
			info._DirectHit = bDirectHit;
			//gplDebugf(TEXT("SendStrikeMsg	爆炸点离舰船的距离Distance[%f]  直接命中[%d]"), info._Distance*P2GScale, bDirectHit);

			NxVec3 vel = GetFlightVelocity(_ElapsedFlightTime);
			info._Angle = NxAngle(vel,StrikeNormal)-NxPiF32;
			return _BehaviorReport->OnProjectileStrike(this,info);
		}
		
		return false;
	}

	void PhysProjectile::PostBurstMsg( NxVec3 HitPos, NxVec3 HitNormal, NxVec3 HitDirection)
	{
		if (_bUtilityTest)
		{
			_HitPos = HitPos;
			return;
		}

		if (_BehaviorReport)
		{
			HitNotifyInfo info;
			info._HitPos = HitPos;
			info._HitNormal = HitNormal;
			info._HitDirection = HitDirection;
			_BehaviorReport->OnProjectileBurstHit(this, info);
		}
	}

	void PhysProjectile::ForceSelfDestruction()
	{
		HitNotifyInfo info;
		info._HitNormal=NxVec3(0.0f,0.0f,1.0f);
		info._HitPos=GetFlightLocation(_TotalFlightSimulatedTime);
		info._HitDirection = GetFlightVelocity(_TotalFlightSimulatedTime);
		info._HitDirection.normalize();
		_HitState= enSelfDestory;
		CheckBurstHit(info._HitPos,info._HitNormal, info._HitDirection, NULL,NULL);

		_bForceSelfDestruction = true;
	}

	bool PhysProjectile::IsTargetInCoverage(float LowerLimitAngle, float UpperLimitAngle)
	{
		ProjectileData* pData = DynamicCast(ProjectileData, _ComponentData);
		if ( !pData )
			return false;

		float PitchAngle;
		if ( !GPL::CalcProjectileAngular(
					pData->_StartPos, 
					pData->_TargetPos, 
					pData->_InitVelocity.magnitude(), 
					pData->_Gravity.magnitude(), 
					PitchAngle) )
			return false;
		
		if ( PitchAngle > UpperLimitAngle )
			return false;

		if( PitchAngle < LowerLimitAngle )
			return false;

		return true;
	}
}