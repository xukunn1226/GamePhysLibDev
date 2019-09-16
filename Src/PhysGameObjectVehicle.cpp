#include "..\Inc\PhysXSupport.h"
#include "..\inc\PhysGameObject.h"
namespace GPL
{
	IMPLEMENT_RTTI(PhysVehicle, PhysGameObject)
	
bool PhysVehicle::Init()
{
	if( !PhysGameObject::Init() )
	{
		return false;
	}
	
	if( GetPhysActor() != NULL )
	{
		_Velocity = GetPhysActor()->getLinearVelocity();
		_AngularVelocity = GetPhysActor()->getAngularVelocity();
	}
		
	_LastFireEscapedTime = 0.0f;

	_ActivePartType = EPT_Invalid;
	_FirePointLastTime = GPhysGameSetting._FirePointLastTime*GLOBAL_DATA_MULTIPLICATOR_TIME;
	_RecordAimingOccasion = GPhysGameSetting._RecordAimingOccasion;
	_AccuracyFormulaA = GPhysGameSetting._AccuracyFormulaA*GLOBAL_DATA_MULTIPLICATOR;
	_AccuracyFormulaB = GPhysGameSetting._AccuracyFormulaB*GLOBAL_DATA_MULTIPLICATOR;
	_AccuracyFormulaC = GPhysGameSetting._AccuracyFormulaC*GLOBAL_DATA_MULTIPLICATOR;
	_AccuracyFormulaD = GPhysGameSetting._AccuracyFormulaD*GLOBAL_DATA_MULTIPLICATOR;

	return true;
}

void PhysVehicle::InitPhys()
{
	VehicleData* CompData = (VehicleData*)_ComponentData;
	if( CompData == NULL || CompData->_PhysModelDesc == NULL )
	{
		return;
	}

	VehicleCompDesc* CompDesc = (VehicleCompDesc*)CompData->_CompDesc;
	if( CompDesc == NULL )
	{
		return;
	}

	NxActorDesc ActorDesc;
	std::vector<NxShapeDesc*> CachedShapeDescList;
	for(NxU32 ShapeIndex = 0; ShapeIndex < CompData->_PhysModelDesc->_ShapeDescList.size(); ++ShapeIndex)
	{
		NxShapeDesc* XShapeDesc = NULL;
		ShapeDesc* CustomShapeDesc = CompData->_PhysModelDesc->_ShapeDescList[ShapeIndex];
		
		// create NxShapeDesc data
		XShapeDesc = CreateComponentShape(CustomShapeDesc);
		if( XShapeDesc != NULL )
		{
			XShapeDesc->group = CustomShapeDesc->_bTriggerShape ? GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME : GPL_SHAPE_GROUP_VEHICLE_SHIP;
					
			ActorDesc.shapes.push_back(XShapeDesc);

#if 1
			if ( CustomShapeDesc->_bTriggerShape && CustomShapeDesc->_ShapeType == EST_SHAPE_BOX )
			{
				BoxShapeDesc* BShapeDesc = (BoxShapeDesc*)CustomShapeDesc;
				_EigenBox.extents = 0.5f * BShapeDesc->_Dimensions * G2PScale;
				_EigenBox.center = CustomShapeDesc->_LocalPose.t;
				_EigenBox.rot = CustomShapeDesc->_LocalPose.M;
			}
#endif
		}
		else
		{
			gplDebugf(TEXT("ERROR: Faied to CreateComponentShape CompId[%d] PhysId[%s] ShapeIndex[%d]"), CompData->_CompDesc->_CompId, CompData->_PhysModelDesc->_PhysId.c_str(), ShapeIndex);
		}
		CachedShapeDescList.push_back(XShapeDesc);
	}
	ActorDesc.globalPose = CompData->_InitGlobalPose;
	_OriginalLocZ = 0.f;		// 设定出生点Z值始终位于零点
	//ActorDesc.dominanceGroup = ...;

	NxBodyDesc BodyDesc;
	BodyDesc.mass = CompData->_Mass;
	BodyDesc.linearDamping = 0.f;
	BodyDesc.angularDamping = 0.f;
	BodyDesc.linearVelocity = CompData->_LinearVelocity;
	BodyDesc.flags |= NX_BF_DISABLE_GRAVITY;
	BodyDesc.flags |= NX_BF_FROZEN_POS_Z;
	BodyDesc.flags |= NX_BF_FROZEN_ROT_X;
	BodyDesc.flags |= NX_BF_FROZEN_ROT_Y;
	BodyDesc.sleepEnergyThreshold = 0.5f;
	BodyDesc.sleepDamping = 0.2f;
	ActorDesc.body = &BodyDesc;

	NxU32 ValidCode = ActorDesc.checkValid();
	if( ValidCode == 0 )
	{
		_PhysActor = _Scene->_PhysScene->createActor(ActorDesc);
	}

	UpdateMassProp(CompData->_Mass, CompData->_CMassOffsetX);
	
	if( _PhysActor != NULL )
	{
		if( CompData->_LinearVelocity.magnitudeSquared() > GPL_PHYS_SMALL_NUMBER )
		{
			_PhysActor->wakeUp();
		}
		else
		{
			_PhysActor->putToSleep();
		}

		// todo xukun: 默认设置产生contact information，客户端需要捕获舰船碰撞信息，服务端需要吗？有优化余地。。。
		_PhysActor->setGroup(GPL_GROUP_NOTIFYCOLLIDE);		// default generate contact information
		_PhysActor->userData = this;		// 绑定逻辑对象至NxActor上
	}

	ReleaseShapeDescList(CachedShapeDescList);

	//init ship components
	InitAllAttachment();
}

void PhysVehicle::PostInitPhys()
{
	PhysGameObject::PostInitPhys();

	VehicleData* Data = (VehicleData*)_ComponentData;
	VehicleCompDesc* CompDesc = (VehicleCompDesc*)Data->_CompDesc;
	
	if( CompDesc == NULL )
	{
		return;
	}
	switch(CompDesc->_SimType)
	{
	case EST_Normal:
		_SimObjWhenLive = new VehicleSimBase(EST_Normal);
		break;
	case EST_Normal_Cruise:
		_SimObjWhenLive = new VehicleSimShip(EST_Normal_Cruise);
		break;
	case EST_Normal_Cruise_Submarine:
		_SimObjWhenLive = new VehicleSimSubmarine(EST_Normal_Cruise_Submarine);
#ifdef _CLIENT_RUNTIME
		//CreateSubmarineSurfaceDummy();		// 策划改动，不需要此功能
#endif // _CLIENT_RUNTIME
		break;
	}

	_SimObjWhenDeath = new VehicleSimDeath(EST_Death);

	// 把死亡和非死亡状态下的Simulation都创建出来，初始赋予非死亡模拟器
	_SimObj = _SimObjWhenLive;
}

void PhysVehicle::PrevTermPhys()
{
#ifdef _CLIENT_RUNTIME
	_SubmarineSurfaceDummy = NULL;
#endif // _CLIENT_RUNTIME

	SAFE_DELETE(_SimObjWhenDeath);
	SAFE_DELETE(_SimObjWhenLive);
	_SimObj = NULL;

	ComponentList::iterator it = _AttachedComponents.begin();
	for( ; it != _AttachedComponents.end(); ++it )
	{
		if( *it != NULL )
		{
			(*it)->Term();
			delete *it;
			*it = NULL;
		}
	}
	_AttachedComponents.clear();

	PhysGameObject::PrevTermPhys();
}

void PhysVehicle::Tick(float DeltaTime)
{
	if( _SimObj )
	{
		_SimObj->Tick(DeltaTime, this);
		_SimObj->UpdateVehicle(this, DeltaTime);
#if 1
		_EigenBox.center = GetGlobalPosition();
		_EigenBox.rot = GetGlobalOritation();
#endif
	}

#ifdef _CLIENT_RUNTIME
	SyncSubmarineSurfaceDummy();
#endif // _CLIENT_RUNTIME

	ComponentList::iterator it;
	for ( it = _AttachedComponents.begin(); it != _AttachedComponents.end(); ++it )
	{
		if( *it != NULL )
			(*it)->Tick(DeltaTime);
	}

	_VelocitySize = _Velocity.magnitude();
	_AngularVelocityZSize = NxMath::abs(_AngularVelocity.z);

	CheckManchineGun(DeltaTime);
	
	_LastFireEscapedTime += DeltaTime;
	if( _bValidRecordPosition && _LastFireEscapedTime > _FirePointLastTime)
	{
		DisableRecordPoint();	
	}

	// 记录点有效，且当前PartType支持越打越准逻辑
	if (_bSupportContinuousFire)
	{
		if (_bValidRecordPosition)
		{
			_FinalAccuracyMultiplier = _FireAccuracyMultiplier * _DistAccuracyMultiplier;
			_FinalAccuracyMultiplier = NxMath::clamp(_FinalAccuracyMultiplier, 1.f, _FinalAccuracyMultiplier);
			if( _CurAccuracyMultiplier <= _FinalAccuracyMultiplier )
			{
				_CurAccuracyMultiplier = _FinalAccuracyMultiplier;
			}
			else
			{
				_CurAccuracyMultiplier -= _AccuracyRecover * DeltaTime;
				_CurAccuracyMultiplier = NxMath::max(_CurAccuracyMultiplier, _FinalAccuracyMultiplier);
			}
		}
		else
		{
			_CurAccuracyMultiplier = 1.f;
		}
	}
	
	//int AttachedId;
	//float AccuracyOfLinearVelocity,AccuracyOfAngularVelocity,AccuracyOfTurrentTurn,MaxAccuracyOfTurrent,CurAccuracyOfTurrent,CurAccuracyMultiplier,FireAccuracyMultiplier,DistAccuracyMultiplier,FinalAccuracyMultiplier;
	//GetAccuracyInfo(AttachedId,
	//				AccuracyOfLinearVelocity, 
	//				AccuracyOfAngularVelocity, 
	//				AccuracyOfTurrentTurn, 
	//				MaxAccuracyOfTurrent,
	//				CurAccuracyOfTurrent,
	//				CurAccuracyMultiplier, 
	//				FireAccuracyMultiplier,
	//				DistAccuracyMultiplier,
	//				FinalAccuracyMultiplier);
	//if( AttachedId == 22 )
	//{
	//	gplDebugf(TEXT("AccLV[%.4f] AccAngV[%.4f] AccTurn[%.4f] MaxAccTurrent[%.4f] CurAccTurrent[%.4f] CurAccMul[%.4f] FireAccMul[%.4f] DistAccMul[%.4f] FinalAccMul[%.4f]"),AccuracyOfLinearVelocity,AccuracyOfAngularVelocity,AccuracyOfTurrentTurn,MaxAccuracyOfTurrent,CurAccuracyOfTurrent,CurAccuracyMultiplier,FireAccuracyMultiplier,DistAccuracyMultiplier,FinalAccuracyMultiplier);
	//}
}

bool PhysVehicle::IsValidOp(EMoveFlag Flag)
{
	if( _SimObj )
	{
		return _SimObj->IsValidOp(Flag);
	}
	return false;
}

void PhysVehicle::Revive()
{
	// 恢复之前Simulation
	_SimObj = _SimObjWhenLive;

	NxActor* pActor = GetPhysActor();
	if ( pActor )
	{
		NxBodyFlag BodyFlag = (NxBodyFlag)(NX_BF_DISABLE_GRAVITY | NX_BF_FROZEN_POS_Z | NX_BF_FROZEN_ROT_X | NX_BF_FROZEN_ROT_Y);
		pActor->raiseBodyFlag(BodyFlag);
	}

	RecoverCollision();
}

void PhysVehicle::DeathSinkSimulate()
{
	Stop();

	_SimObj = _SimObjWhenDeath;

	NxActor* pActor = GetPhysActor();
	if ( pActor )
		pActor->clearBodyFlag(NX_BF_FROZEN);
}

void PhysVehicle::RandomSinkParams()
{
	VehicleSimDeath* pSimDeath = DynamicCast(VehicleSimDeath, _SimObj);
	if ( pSimDeath )
	{
		pSimDeath->RandomPose();

		SetSinkParams(pSimDeath->_Roll, pSimDeath->_Pitch, pSimDeath->_ForceFactor, pSimDeath->_TorqueFactor);
	}
}

void PhysVehicle::SetSinkParams(float fRoll, float fPitch, float fForceFactor, float fTorqueFactor)
{
	VehicleSimDeath* pSimDeath = DynamicCast(VehicleSimDeath, _SimObj);
	if ( pSimDeath )
	{
		pSimDeath->_Roll = fRoll;
		pSimDeath->_Pitch = fPitch;
		pSimDeath->_ForceFactor = fForceFactor;
		pSimDeath->_TorqueFactor = fTorqueFactor;
		pSimDeath->InitPhysParams(this);
	}
}

void PhysVehicle::Stop()
{
	ResetMoveFlag(MoveFlag_None);
}

void PhysVehicle::MoveForward(bool bForward)
{
	if( _SimObj && _SimObj->IsValidOp(MoveFlag_Forward) )
	{
		_SimObj->MoveForward(bForward);
	}
}

void PhysVehicle::MoveBackward(bool bBackward)
{
	if( _SimObj && _SimObj->IsValidOp(MoveFlag_Backward) )
	{
		_SimObj->MoveBackward(bBackward);
	}
}

void PhysVehicle::TurnRight(bool bRight)
{
	if( _SimObj && _SimObj->IsValidOp(MoveFlag_RightTurn) )
	{
		_SimObj->TurnRight(bRight);
	}
}

void PhysVehicle::TurnLeft(bool bLeft)
{
	if( _SimObj && _SimObj->IsValidOp(MoveFlag_LeftTurn) )
	{
		_SimObj->TurnLeft(bLeft);
	}
}

void PhysVehicle::ShiftDriving(bool bForward)
{
	if( _SimObj && _SimObj->IsValidOp(MoveFlag_CruiseDriving) )
	{
		VehicleData* Data = (VehicleData*)_ComponentData;
		VehicleCompDesc* CompDesc = (VehicleCompDesc*)Data->_CompDesc;
		if( CompDesc == NULL )
		{
			return;
		}

		int OldShiftIndexDriving = ((VehicleSimShip*)_SimObj)->GetDriveGear();
		int CurShiftIndexDriving = OldShiftIndexDriving;
		if(bForward)
		{
			// 限制挡位
			if( (OldShiftIndexDriving) < CompDesc->_MaxDriveGear )
				++CurShiftIndexDriving;
		}
		else
		{
			if( OldShiftIndexDriving > (CompDesc->_MaxReverseGear * (-1)) )
				--CurShiftIndexDriving;
		}
				
		((VehicleSimShip*)_SimObj)->SetShiftDriving(CurShiftIndexDriving);
	}
}

void PhysVehicle::ShiftSteering(bool bRight)
{
	if( _SimObj && _SimObj->IsValidOp(MoveFlag_CruiseSteering) )
	{
		VehicleData* Data = (VehicleData*)_ComponentData;
		VehicleCompDesc* CompDesc = (VehicleCompDesc*)Data->_CompDesc;
		if( CompDesc == NULL )
		{
			return;
		}

		int OldShiftIndexSteering = ((VehicleSimShip*)_SimObj)->GetSteerGear();
		int CurShiftIndexSteering = OldShiftIndexSteering;
		if(bRight)
		{
			// 限制挡位
			if( (OldShiftIndexSteering) < CompDesc->_MaxTurnGear )
				++CurShiftIndexSteering;
		}
		else
		{
			if( OldShiftIndexSteering > (CompDesc->_MaxTurnGear * (-1)) )
				--CurShiftIndexSteering;
		}

		((VehicleSimShip*)_SimObj)->SetShiftSteering(CurShiftIndexSteering);
	}
}

bool PhysVehicle::SetLevelInfo(int CurLevel, int PendingLevel)
{
	if( !IsSubmarine() )
	{
		return false;
	}

	_CurLevel = CurLevel;
	_PendingLevel = PendingLevel;

	float DesiredLocZ = GetLevelLocation(PendingLevel);
	float CurrentLocZ = GetLevelLocation(CurLevel);

	_DesiredLocZ = DesiredLocZ;
	_CurrLocZ = CurrentLocZ;
	_DeltaLocZ = CurrentLocZ - DesiredLocZ;

	return true;
}

void PhysVehicle::Riseup(int CurLevel,int PendingLevel)
{
	if( _SimObj && _SimObj->IsValidOp(MoveFlag_Riseup) )
	{
		VehicleData* Data = (VehicleData*)_ComponentData;
		VehicleCompDesc* CompDesc = (VehicleCompDesc*)Data->_CompDesc;
		if( CompDesc == NULL )
		{
			return;
		}
		_DeltaLocZ = 0.f;

		// 判断请求是否合理
		if( CurLevel >= PendingLevel || PendingLevel <= -2 || PendingLevel > 0 )
			return;

		if( SetLevelInfo(CurLevel, PendingLevel) )
		{
			_SimObj->Riseup(this);
		}
	}
}

void PhysVehicle::Sink(int CurLevel,int PendingLevel)
{
	if( _SimObj && _SimObj->IsValidOp(MoveFlag_Sink) )
	{
		VehicleData* Data = (VehicleData*)_ComponentData;
		VehicleCompDesc* CompDesc = (VehicleCompDesc*)Data->_CompDesc;
		if( CompDesc == NULL )
		{
			return;
		}
		_DeltaLocZ = 0.f;

		// 判断请求是否合理
		if( CurLevel <= PendingLevel || PendingLevel < -2 || PendingLevel >= 0 )
			return;

		if( SetLevelInfo(CurLevel, PendingLevel) )
		{
			_SimObj->Sink(this);
		}
	}
}

// 到达指定目标层
void PhysVehicle::OnNotifyStopRiseOrSink()
{
	NxVec3 Position = GetGlobalPosition();
	Position.z = _DesiredLocZ;
	SetGlobalPosition(Position);

	if( _PhysActor != NULL )
	{
		NxVec3 Velocity = _PhysActor->getLinearVelocity();
		Velocity.z = 0.f;
		_PhysActor->setLinearVelocity(Velocity);
	}

	_CurLevel = _PendingLevel;
}

void PhysVehicle::ForceReachDesiredLayer()
{
	if( _SimObj )
	{
		return _SimObj->ForceReachDesiredLayer(this);
	}
}

bool PhysVehicle::HaveAnyMoveOp()
{
	if( _SimObj )
	{
		return _SimObj->HaveAnyMoveOp();
	}
	return false;
}

bool PhysVehicle::HaveAnyTurnOp()
{
	if( _SimObj )
	{
		return _SimObj->HaveAnyTurnOp();
	}
	return false;
}

bool PhysVehicle::IsInCruiseDrive()
{
	if( _SimObj )
	{
		return _SimObj->IsInCruiseDrive();
	}
	return false;
}

bool PhysVehicle::IsInCruiseSteer()
{
	if( _SimObj )
	{
		return _SimObj->IsInCruiseSteer();
	}
	return false;
}

void PhysVehicle::ResetMoveFlag(EMoveFlag Flag)
{
	if( _SimObj )
	{
		_SimObj->ClearMoveFlag(MoveFlag_AllOp);
		_SimObj->SetMoveFlag(Flag);
	}
}

void PhysVehicle::UGetVehicleState(UVehicleState& NewState)
{
	if( _PhysActor != NULL )
	{
		NewState.Position = _PhysActor->getGlobalPosition() * P2GScale;
		NewState.Quaternion = _PhysActor->getGlobalOrientationQuat();
		NewState.LinVel = _PhysActor->getLinearVelocity() * P2GScale;
		NewState.AngVel = _PhysActor->getAngularVelocity();

		NxVec3 Axis;
		NxReal Angle;
		NewState.Quaternion.getAngleAxis(Angle, Axis);
		NewState.Angle = Axis.z > 0.f ? Angle : (360.f - Angle);
	}
}

void PhysVehicle::GetVehicleState(VehicleState& NewState)
{
	if( _PhysActor != NULL )
	{
		NewState.Position = _PhysActor->getGlobalPosition();
		NewState.Quaternion = _PhysActor->getGlobalOrientationQuat();
		NewState.LinVel = _PhysActor->getLinearVelocity();
		NewState.AngVel = _PhysActor->getAngularVelocity();

		NxVec3 Axis;
		NxReal Angle;
		NewState.Quaternion.getAngleAxis(Angle, Axis);
		NewState.Angle = Axis.z > 0.f ? Angle : (360.f - Angle);
	}
}

void PhysVehicle::UGetVehicleStateForServer(float& PosX, float& PosY, float& PosZ, unsigned char& AngleYaw, float& LinVelX, float& LinVelY, float& LinVelZ, signed char& AngleVelYaw)
{
	UVehicleState NewState;
	UGetVehicleState(NewState);

	PosX = NewState.Position.x;
	PosY = NewState.Position.y;
	PosZ = NewState.Position.z;

	AngleYaw = (unsigned char)NxMath::clamp(int(NewState.Angle * 0.7084f), 255, 0);		// 0.7083 = 255/360

	LinVelX = NewState.LinVel.x;
	LinVelY = NewState.LinVel.y;
	LinVelZ = NewState.LinVel.z;

	AngleVelYaw = (signed char)NxMath::clamp(int(NewState.AngVel.z * 57.2958f), 127, -128);		// 57.2958 = 180 / Pi
}

float PhysVehicle::GetMaxVelocityRatio()
{
	float MaxSpeed = GetInitializedMaxForwardSpeed();
	return MaxSpeed == 0.f ? 0.f : _Velocity.magnitude() / MaxSpeed;			//速度比率
}

bool PhysVehicle::IsSubmarine()
{
	if( _SimObj != NULL )
	{
		return IsKind(VehicleSimSubmarine, _SimObj);
	}
	return false;
}

int PhysVehicle::GetDriveGear()
{
	int iGear = 0;

	if( _SimObj && _SimObj->IsValidOp(MoveFlag_CruiseDriving)  )
	{
		iGear = ((VehicleSimShip*)_SimObj)->GetDriveGear();
	}

	return iGear;
}

int PhysVehicle::GetSteerGear()
{
	int iGear = 0;

	if( _SimObj && _SimObj->IsValidOp(MoveFlag_CruiseSteering) )
	{
		iGear = ((VehicleSimShip*)_SimObj)->GetSteerGear();
	}

	return iGear;
}

void PhysVehicle::SetDriveGear(int iGear)
{
	if( _SimObj && _SimObj->IsValidOp(MoveFlag_CruiseDriving) )
	{
		VehicleData* Data = (VehicleData*)_ComponentData;
		VehicleCompDesc* CompDesc = (VehicleCompDesc*)Data->_CompDesc;
		if( CompDesc == NULL )
		{
			return;
		}

		if( iGear >= (CompDesc->_MaxReverseGear * (-1)) 
			&& iGear <= CompDesc->_MaxDriveGear )
		{
			((VehicleSimShip*)_SimObj)->SetShiftDriving(iGear);
		}
	}
}

void PhysVehicle::SetSteerGear(int iGear)
{
	if( _SimObj && _SimObj->IsValidOp(MoveFlag_CruiseDriving) )
	{
		VehicleData* Data = (VehicleData*)_ComponentData;
		VehicleCompDesc* CompDesc = (VehicleCompDesc*)Data->_CompDesc;
		if( CompDesc == NULL )
		{
			return;
		}

		if( iGear >= (CompDesc->_MaxTurnGear * (-1)) 
			&& iGear <= CompDesc->_MaxTurnGear )
		{
			((VehicleSimShip*)_SimObj)->SetShiftSteering(iGear);
		}
	}
}

int PhysVehicle::ClampDriveGear(int iGear)
{
	VehicleData* Data = (VehicleData*)_ComponentData;
	VehicleCompDesc* CompDesc = (VehicleCompDesc*)Data->_CompDesc;
	if( CompDesc == NULL )
	{
		return 0;
	}

	return NxMath::clamp(iGear, CompDesc->_MaxDriveGear, (CompDesc->_MaxReverseGear * -1));
}

int PhysVehicle::ClampSteerGear(int iGear)
{
	VehicleData* Data = (VehicleData*)_ComponentData;
	VehicleCompDesc* CompDesc = (VehicleCompDesc*)Data->_CompDesc;
	if( CompDesc == NULL )
	{
		return 0;
	}

	return NxMath::clamp(iGear, CompDesc->_MaxTurnGear, (CompDesc->_MaxTurnGear * -1));
}

float PhysVehicle::GetMaxForwardSpeed()
{
	return UGetMaxForwardSpeed() * G2PScale;
}

float PhysVehicle::UGetMaxForwardSpeed()
{
	VehicleData* ObjectData = (VehicleData*)GetComponentData();
	if( ObjectData == NULL )
	{
		return 0.f;
	}
	return ObjectData->_MaxForwardSpeed;
}

float PhysVehicle::GetInitializedMaxForwardSpeed()
{
	return UGetInitializedMaxForwardSpeed() * G2PScale;
}

float PhysVehicle::UGetInitializedMaxForwardSpeed()
{
	VehicleData* ObjectData = (VehicleData*)GetComponentData();
	if( ObjectData == NULL )
	{
		return 0.f;
	}
	return ObjectData->_MaxForwardSpeedCopy;
}

float PhysVehicle::GetMaxReverseSpeed()
{
	return UGetMaxReverseSpeed() * G2PScale;
}

float PhysVehicle::UGetMaxReverseSpeed()
{
	VehicleData* ObjectData = (VehicleData*)GetComponentData();
	if( ObjectData == NULL )
	{
		return 0.f;
	}
	return ObjectData->_MaxForwardSpeed * GPhysGameSetting._GPL_ReverseSpeed_Rate;
}

float PhysVehicle::GetInitializedMaxReverseSpeed()
{
	return UGetInitializedMaxReverseSpeed() * G2PScale;
}

float PhysVehicle::UGetInitializedMaxReverseSpeed()
{
	VehicleData* ObjectData = (VehicleData*)GetComponentData();
	if( ObjectData == NULL )
	{
		return 0.f;
	}
	return ObjectData->_MaxForwardSpeedCopy * GPhysGameSetting._GPL_ReverseSpeed_Rate;
}

float PhysVehicle::GetPendingTurnRate()
{
	VehicleData* ObjectDesc = (VehicleData*)GetComponentData();
	if( ObjectDesc == NULL )
	{
		return 0.f;
	}
	VehicleCompDesc* CompDesc = (VehicleCompDesc*)ObjectDesc->_CompDesc;
	if( CompDesc == NULL )
	{
		return 0.f;
	}

	NxVec3 VelocityXY = _Velocity;
	VelocityXY.z = 0;
	float LinearVelXYMag = VelocityXY.magnitude();
	float MaxForwardSpeed = GetInitializedMaxForwardSpeed();

	float RealFactor = NxMath::clamp(GPhysGameSetting._GPL_FactorOfAngularVelocity, 1.f, 0.f);
	float RealMaxForwardSpeed = (MaxForwardSpeed == 0.f ? LinearVelXYMag : MaxForwardSpeed);
	float Ratio = NxMath::clamp(LinearVelXYMag / RealMaxForwardSpeed, 1.f, 0.f);

	// 根据当前速度与最大速度比值计算当前角速度
	float PendingTurnRate = ((Ratio * RealFactor) + (1 - RealFactor)) * CompDesc->_MaxTurnRate;

	return PendingTurnRate;
}

float PhysVehicle::GetMaxTurnRate()
{
	VehicleData* ObjectDesc = (VehicleData*)GetComponentData();
	if( ObjectDesc == NULL )
	{
		return 0.f;
	}
	VehicleCompDesc* CompDesc = (VehicleCompDesc*)ObjectDesc->_CompDesc;
	if( CompDesc == NULL )
	{
		return 0.f;
	}
	return CompDesc->_MaxTurnRate;
}

NxVec3 PhysVehicle::UGetCurVelocity()
{
	return _Velocity * P2GScale;
}

float  PhysVehicle::UGetCurVelocityMag()
{
	return UGetCurVelocity().magnitude();
}

float PhysVehicle::GetMaxReverseForce()
{
	VehicleData* ObjectData = (VehicleData*)GetComponentData();
	if( ObjectData == NULL )
	{
		return 0.f;
	}
	return ObjectData->_MaxForwardForce * GPhysGameSetting._GPL_ReverseForce_Rate;
}

float PhysVehicle::GetCruiseSpeed(int ShiftIndex, bool bForward)
{
	VehicleData* ObjectDesc = (VehicleData*)GetComponentData();
	if( ObjectDesc == NULL )
	{
		return 0.f;
	}
	VehicleCompDesc* CompDesc = (VehicleCompDesc*)ObjectDesc->_CompDesc;
	if( CompDesc == NULL )
	{
		return 0.f;
	}

	float MaxSpeed = bForward ? GetMaxForwardSpeed() : GetMaxReverseSpeed();
	int MaxGear = bForward ? CompDesc->_MaxDriveGear : CompDesc->_MaxReverseGear;
	return NxMath::clamp(NxMath::abs(ShiftIndex) * MaxSpeed / MaxGear, MaxSpeed, 0.f);
}

float PhysVehicle::GetCruiseTurnRate(int ShiftIndex, float MaxTurnRate)
{
	VehicleData* ObjectDesc = (VehicleData*)GetComponentData();
	if( ObjectDesc == NULL )
	{
		return 0.f;
	}
	VehicleCompDesc* CompDesc = (VehicleCompDesc*)ObjectDesc->_CompDesc;
	if( CompDesc == NULL )
	{
		return 0.f;
	}
	return NxMath::abs(ShiftIndex) * (MaxTurnRate / CompDesc->_MaxTurnGear);
}

float PhysVehicle::GetSpeedWhenNoThrottle()
{
	return GetCruiseSpeed(1, true);
}

void PhysVehicle::USetMaxForwardSpeed(float MaxSpeed)
{
	VehicleData* CompData = DynamicCast(VehicleData, _ComponentData);
	if( CompData != NULL )
	{
		CompData->_MaxForwardSpeed = MaxSpeed;
	}
}

void PhysVehicle::USetMaxForwardForce(float MaxForce)
{
	VehicleData* CompData = DynamicCast(VehicleData, _ComponentData);
	if( CompData != NULL )
	{
		CompData->_MaxForwardForce = MaxForce;
	}
}

void PhysVehicle::USetMaxRiseSinkForce(float MaxForce)
{
	VehicleData* CompData = DynamicCast(VehicleData, _ComponentData);
	if( CompData != NULL )
	{
		CompData->_MaxRiseSinkForce = MaxForce;
	}
}

void PhysVehicle::USetMaxTorque(float MaxTorque)
{
	VehicleData* CompData = DynamicCast(VehicleData, _ComponentData);
	if( CompData != NULL )
	{
		CompData->_TurnTorque = MaxTorque;
	}
}

void PhysVehicle::USetMass(float Mass)
{
	VehicleData* CompData = DynamicCast(VehicleData, _ComponentData);
	if( CompData != NULL )
	{
		CompData->_Mass = Mass;
	}

	UpdateMassProp(Mass, CompData->_CMassOffsetX);
}

void PhysVehicle::USetTurretAngleVelocity(int AttachedID, float TurretAngleVelocity)
{
	ShipComponent* ShipComp = GetShipComponent(AttachedID);
	if( ShipComp != NULL && ShipComp->_ComponentData != NULL && IsKind(WeaponData, ShipComp->_ComponentData) )
	{
		((WeaponData*)ShipComp->_ComponentData)->_TurretAngleVelocity = TurretAngleVelocity;
	}
}

void PhysVehicle::USetInitVelocity(int AttachedID, float InitVelocity)
{
	ShipComponent* ShipComp = GetShipComponent(AttachedID);
	if( ShipComp != NULL && ShipComp->_ComponentData != NULL && IsKind(WeaponData, ShipComp->_ComponentData) )
	{
		((WeaponData*)ShipComp->_ComponentData)->_InitVelocity = InitVelocity * G2PScale;
	}
}

void PhysVehicle::USetGravity(int AttachedID, float Gravity)
{
	ShipComponent* ShipComp = GetShipComponent(AttachedID);
	if( ShipComp != NULL && ShipComp->_ComponentData != NULL && IsKind(WeaponData, ShipComp->_ComponentData) )
	{
		((WeaponData*)ShipComp->_ComponentData)->_Gravity = Gravity * G2PScale;
	}
}

void PhysVehicle::USetAccuracyDefault(int AttachedID, float AccuracyDefault)
{
	ShipComponent* ShipComp = GetShipComponent(AttachedID);
	if( ShipComp != NULL && ShipComp->_ComponentData != NULL && IsKind(WeaponData, ShipComp->_ComponentData) )
	{
		((WeaponData*)ShipComp->_ComponentData)->_AccuracyDefault = AccuracyDefault;
	}
}

void PhysVehicle::USetAccuracyTurrent(int AttachedID, float AccuracyTurrent)
{
	ShipComponent* ShipComp = GetShipComponent(AttachedID);
	if( ShipComp != NULL && ShipComp->_ComponentData != NULL && IsKind(WeaponData, ShipComp->_ComponentData) )
	{
		((WeaponData*)ShipComp->_ComponentData)->_AccuracyTurrent = AccuracyTurrent;
	}
}

void PhysVehicle::USetAccuracyShipDrive(int AttachedID, float AccuracyShipDrive)
{
	ShipComponent* ShipComp = GetShipComponent(AttachedID);
	if( ShipComp != NULL && ShipComp->_ComponentData != NULL && IsKind(WeaponData, ShipComp->_ComponentData) )
	{
		((WeaponData*)ShipComp->_ComponentData)->_AccuracyShipDrive = AccuracyShipDrive;
	}
}

void PhysVehicle::USetAccuracyShipSteer(int AttachedID, float AccuracyShipSteer)
{
	ShipComponent* ShipComp = GetShipComponent(AttachedID);
	if( ShipComp != NULL && ShipComp->_ComponentData != NULL && IsKind(WeaponData, ShipComp->_ComponentData) )
	{
		((WeaponData*)ShipComp->_ComponentData)->_AccuracyShipSteer = AccuracyShipSteer;
	}
}

void PhysVehicle::USetAccuracyRecover(int AttachedID, float AccuracyRecover)
{
	ShipComponent* ShipComp = GetShipComponent(AttachedID);
	if( ShipComp != NULL && ShipComp->_ComponentData != NULL && IsKind(WeaponData, ShipComp->_ComponentData) )
	{
		((WeaponData*)ShipComp->_ComponentData)->_AccuracyRecover = AccuracyRecover;
	}
}

void PhysVehicle::USetAccuracyFire(int AttachedID, float AccuracyFire)
{
	ShipComponent* ShipComp = GetShipComponent(AttachedID);
	if( ShipComp != NULL && ShipComp->_ComponentData != NULL && IsKind(WeaponData, ShipComp->_ComponentData) )
	{
		((WeaponData*)ShipComp->_ComponentData)->_AccuracyFire = AccuracyFire;
	}
}

// 装载所有部件（武器、舰桥等）
void PhysVehicle::InitAllAttachment()
{
	VehicleData* CompData = (VehicleData*)_ComponentData;
	if( CompData == NULL || CompData->_PhysModelDesc == NULL )
	{
		return;
	}

	std::vector<VehicleAttachedCompData*>::iterator it = CompData->_AttachedCompList.begin();
	for ( ; it != CompData->_AttachedCompList.end(); ++it )
	{
		Socket AttachedSocket;
		if( !CompData->FindSocket((*it)->_AttachedId, AttachedSocket) )
		{
			gplDebugf(TEXT("PhysVehicle::InitAllAttachment 没有找到SOCKET[%d]"), (*it)->_AttachedId);
			continue;
		}
		ShipComponent* ShipComp = AttachAttachment(*it, AttachedSocket);
		if( ShipComp == NULL )
		{
			continue;
		}
		
		int ZeroedCount = int( (*it)->_AttachedId - (_AttachedComponents.size() - 1));
		if( ZeroedCount > 0 )
		{
			_AttachedComponents.insert(_AttachedComponents.begin() + _AttachedComponents.size(), ZeroedCount, NULL);
		}
		_AttachedComponents[(*it)->_AttachedId] = ShipComp;
	}
}

ShipComponent* PhysVehicle::AttachAttachment( ComponentData* CompData, Socket AttachedTo )
{
	ShipComponent* ShipComp = NULL;

	switch ( CompData->_CompDesc->_ObjectType )
	{
	case EGOT_Attached_Weapon:
		{
			ShipComp = new Turrent();
		}		
		break;
	case EGOT_Attached_Comp:
		{
			ShipComp = new ShipComponent();
		}
		break;
	}
	
	if( ShipComp == NULL )
		return NULL;

	if( !ShipComp->InitShipComponent(CompData) )
	{
		ShipComp->Term();
		delete ShipComp;
		return NULL;
	}
	ShipComp->AttachTo(this, AttachedTo);
	return ShipComp;
}

ShipComponent* PhysVehicle::GetShipComponent( int AttachedID )
{
	if( AttachedID > 0 && AttachedID < (int)_AttachedComponents.size() )
	{
		return _AttachedComponents.at(AttachedID);
	}
	return NULL;
}

void PhysVehicle::SetAimTarget( EPartType PartType, NxVec3 TargetLoc )
{
	_ActivePartType = PartType;
	_TargetPos = TargetLoc;
	for (NxU32 i = 0; i < _AttachedComponents.size(); i++)
	{
		ShipComponent* ShipComp = _AttachedComponents[i];
		if (!IsKind(Turrent, ShipComp))
			continue;

		EPartType ePartType = ShipComp->GetSocketAttachedTo()._PartType;
		if (ePartType == PartType)
		{
			ShipComp->SetAimTarget(TargetLoc);
			WeaponData* pWeaponData = DynamicCast(WeaponData, ShipComp->_ComponentData);
			if (NULL != pWeaponData)
				_AccuracyRecover = pWeaponData->_AccuracyRecover;
		}
		else if (ePartType == EPT_SearchlightLauncher)
		{
			GameObject* pAppendentObject = ((Turrent*)ShipComp)->GetAppendentObject();
			AssociatedTrigger* pTrigger = DynamicCast(AssociatedTrigger, pAppendentObject);
			if (NULL != pTrigger && pTrigger->IsActivated())
				ShipComp->SetAimTarget(TargetLoc);
		}
	}

	_bSupportContinuousFire = SupportContinuousFire(PartType);

	// 越打越准逻辑
	if (_bSupportContinuousFire && _bValidRecordPosition)
		_DistAccuracyMultiplier = CalcDistAccuracyMultiplier(TargetLoc);
}

void PhysVehicle::USetAimTarget( EPartType PartType, NxVec3 TargetLoc )
{
	SetAimTarget(PartType, TargetLoc * G2PScale);
}

void PhysVehicle::CheckManchineGun( float DeltaTime )
{
	_MachineGunCheckTimeDelta+=DeltaTime;
	while (_MachineGunCheckTimeDelta>_MachineGunCheckTime)
	{
		_MachineGunCheckTimeDelta-=_MachineGunCheckTime;
		std::vector<PhysGameObject*> Aircrafts;
		std::vector<PhysGameObject*>::iterator iter=_Scene->_PhysGameObjectList.begin();
		for (;iter!=_Scene->_PhysGameObjectList.end();iter++)
		{
			if ((*iter)->GetObjectType()==EGOT_Vehicle_Aircraft)
			{
				PhysAircraft *Aircraft = (PhysAircraft*)(*iter);
				NxVec3 pos1=Aircraft->GetGlobalPosition();
				NxVec3 pos2=GetGlobalPosition();
				if ((pos2-pos1).magnitude()<_MachineGunAttackRange)
				{
					Aircrafts.push_back(*iter);
				}
			}
		}
		if (!Aircrafts.empty()&&_BehaviorReport)
		{
			_BehaviorReport->OnAircraftEnter(Aircrafts);
		}
	}
}

void PhysVehicle::UpdateMassProp(float Mass, float CMassOffsetX)
{
	if( _PhysActor == NULL )
		return;

	if( Mass == 0.f )
	{
		_PhysActor->updateMassFromShapes(1.f, Mass);
	}
	else
	{
		_PhysActor->updateMassFromShapes(0.f, Mass);
	}

	// reset mass position
	NxVec3 MassLocalPos = _PhysActor->getCMassLocalPosition();
	MassLocalPos.x += CMassOffsetX;
	_PhysActor->setCMassOffsetLocalPosition(MassLocalPos);
}

bool PhysVehicle::IsVelocityFoward()
{
	NxF32 angle=NxAngle(GetGlobalPos().M.getColumn(0),_Velocity);
	return angle<NxPiF32/2;
}

bool PhysVehicle::SupportContinuousFire(EPartType PartType)
{
#if 0
	bool bSupportContinuousFire = false;
	for(NxU32 i = 0; i < GPhysGameSetting._PartTypeListOfSupportContinuousFire.size(); ++i)
	{
		if( GPhysGameSetting._PartTypeListOfSupportContinuousFire[i] == PartType )
		{
			bSupportContinuousFire = true;
			break;
		}
	}

	return bSupportContinuousFire;
#else
	ComponentList::iterator it;
	for (it = _AttachedComponents.begin(); it != _AttachedComponents.end(); it++)
	{
		Turrent* WeaponComp = DynamicCast(Turrent, (*it));
		if (!WeaponComp)
			continue;

		WeaponData* Data = DynamicCast(WeaponData, WeaponComp->_ComponentData);
		if (!Data)
			continue;

		if (Data->_PartType == PartType)
		{
			if (1.f - Data->_AccuracyContinuousFire > 0.0001f)
				return true;
			else
				return false;
		}
	}

	return false;
#endif
}

void PhysVehicle::OnSetFired(int AttachID, int FirePartID, NxVec3 TargetPos, EPartType FireFromPartType)
{	
	if (_RecordAimingOccasion && SupportContinuousFire(FireFromPartType) )
	{ // 开炮后记录_RecordPos
		EnableRecordPoint(TargetPos * G2PScale);
	}

	ShipComponent* ShipComp = GetShipComponent(AttachID);
	if ( !ShipComp )
		return;

	Turrent* TurrentComp = DynamicCast(Turrent, ShipComp);
	if ( !TurrentComp )
		return;

	TurrentComp->SetFired(FirePartID);
}

void PhysVehicle::OnProjectileHit( PhysProjectile * Projectile, float AccuracyContinuousFire, EPartType FireFromPartType )
{
	bool ValidHit = false;
	float DistAccuracyMultiplier = 1.f;
	if ( !_RecordAimingOccasion && SupportContinuousFire(FireFromPartType) )
	{ // 命中后记录_RecordPos
		ShipComponent* pComponent = GetShipComponent(Projectile->GetSponsorAttachId());
		if ( NULL != pComponent && NULL != pComponent->_ComponentData )
		{
			WeaponCompDesc* pDesc = DynamicCast(WeaponCompDesc, pComponent->_ComponentData->_CompDesc);
			if ( NULL != pDesc )
			{
				ValidHit = Projectile->IsTargetInCoverage(pDesc->_GunTubeLimitAngleLower, pDesc->_GunTubeLimitAngleUpper);
				//	只有当目标点被当前武器射击范围覆盖，才认为是有效的射击记录
				if (ValidHit)
				{
					DistAccuracyMultiplier = CalcDistAccuracyMultiplier(Projectile->GetTargetPos());
					EnableRecordPoint(Projectile->GetTargetPos());
				}
			}
		}
	}

	if (_bValidRecordPosition)
	{
		_FireAccuracyMultiplier = NxMath::clamp(_FireAccuracyMultiplier * DistAccuracyMultiplier, 1.f, GPhysGameSetting._AccuracyLimitMin * GLOBAL_DATA_MULTIPLICATOR);
		_FireAccuracyMultiplier *= AccuracyContinuousFire;
		_FireAccuracyMultiplier = NxMath::clamp(_FireAccuracyMultiplier, 1.f, GPhysGameSetting._AccuracyLimitMin * GLOBAL_DATA_MULTIPLICATOR);

		_FinalAccuracyMultiplier = _FireAccuracyMultiplier * _DistAccuracyMultiplier;
	}
}

void PhysVehicle::EnableRecordPoint(NxVec3 RecordPos)
{
	_bValidRecordPosition = true;
	_RecordPos = RecordPos;
	_LastFireEscapedTime = 0.f;
	_DistAccuracyMultiplier = CalcDistAccuracyMultiplier(_TargetPos);
}

void PhysVehicle::DisableRecordPoint()
{
	_FinalAccuracyMultiplier = 1.f;
	_FireAccuracyMultiplier = 1.f;
	_CurAccuracyMultiplier = 1.f;
	_DistAccuracyMultiplier = 1.f;
	_bValidRecordPosition = false;
}

bool PhysVehicle::HasFireAimingAffect()
{
	if( _bValidRecordPosition && _bSupportContinuousFire )
	{
		float fFinalAccuracyMultiplier = _FireAccuracyMultiplier * _DistAccuracyMultiplier;
		fFinalAccuracyMultiplier = NxMath::clamp(fFinalAccuracyMultiplier, 1.f, fFinalAccuracyMultiplier);
		if( _CurAccuracyMultiplier <= fFinalAccuracyMultiplier )
			return false;
		else
			return true;
	}

	return false;
}

float PhysVehicle::CalcDistAccuracyMultiplier(NxVec3 AimingPos)
{
	if( !_bValidRecordPosition )
	{
		return 1.f;
	}
	
	NxVec3 Delta = AimingPos - _RecordPos;
	Delta.z = 0;

	float DistAccuracyMultiplier = NxMath::pow(_AccuracyFormulaA, Delta.magnitude()*P2GScale*_AccuracyFormulaB) + _AccuracyFormulaD;
	return 1.f + DistAccuracyMultiplier * 0.01f;
}

float PhysVehicle::GetLimitAccuracy()
{
	if (!_bSupportContinuousFire)
		return 1.f;

	return _CurAccuracyMultiplier;
}

float PhysVehicle::GetEvaluateAccuracy(const NxVec3& AimingPoint)
{
	if (!_bValidRecordPosition)
		return 0;

	NxReal _DistFactor = CalcDistAccuracyMultiplier(AimingPoint);
	_DistFactor *= 0.99f;

	return NxMath::clamp(_FireAccuracyMultiplier * _DistFactor, 1.f, 0.5f);
}

NxVec3 PhysVehicle::GetLastAimingPos()
{
	return _TargetPos;
}

float PhysVehicle::GetEscapedTime()
{
	return _LastFireEscapedTime;
}

float PhysVehicle::GetLevelLocation( int level )
{
	if( level == 0 )
	{
		return  _OriginalLocZ;
	}
	else if( level == -1 )
	{
		return _OriginalLocZ + GPhysGameSetting._GPL_WaterLayerOneHeight * G2PScale * -1.f;
	}
	else if( level == -2 )
	{
		return _OriginalLocZ + GPhysGameSetting._GPL_WaterLayerTwoHeight * G2PScale * -1.f;
	}
	return 0;
}

#ifdef _CLIENT_RUNTIME
bool PhysVehicle::CreateSubmarineSurfaceDummy()
{
	if ( NULL != _SubmarineSurfaceDummy )
		return false;

	if ( !_PhysActor )
		return false;

	VehicleData* CompData = DynamicCast(VehicleData, _ComponentData);
	if (!CompData)
		return false;

	NxCapsuleShapeDesc ShapeDesc;
	ShapeDesc.height = CompData->_Length;	//	Along Axis-Y
	ShapeDesc.radius = NxMath::sqrt(CompData->_Width * CompData->_Width + CompData->_Height * CompData->_Height) * 0.5f;
	ShapeDesc.localPose.t = ZERO_VEC;
	ShapeDesc.localPose.M.rotZ(NxHalfPi);
	ShapeDesc.group = GPL_SHAPE_GROUP_SUBMARINE_SURFACE_VOLUME;

	if ( 0 == ShapeDesc.checkValid() )
	{
		_SubmarineSurfaceDummy = _PhysActor->createShape(ShapeDesc);
		if ( _SubmarineSurfaceDummy )
			return true;
	}
	else
	{
		gplDebugf(TEXT("CreateSubmarineSurfaceDummy Faild"));
	}

#if 0
	NxBodyDesc BodyDesc;
	BodyDesc.linearDamping = 0.f;
	BodyDesc.angularDamping = 0.f;
	BodyDesc.flags = NX_BF_DISABLE_GRAVITY | NX_BF_FROZEN_POS_Z;
	BodyDesc.sleepEnergyThreshold = 0.5f;
	BodyDesc.sleepDamping = 0.2f;

	NxActorDesc ActorDesc;
	ActorDesc.shapes.push_back(&ShapeDesc);
	ActorDesc.body = &BodyDesc;
	ActorDesc.density = 1.0f;
	ActorDesc.globalPose.zero();
	ActorDesc.globalPose.M.rotY(90.f);

	NxU32 ValidCode = ActorDesc.checkValid();
	if( ValidCode == 0 )
	{
		_SubmarineSurfaceDummy = _Scene->_PhysScene->createActor(ActorDesc);
		if( _SubmarineSurfaceDummy != NULL )
		{
			_SubmarineSurfaceDummy->setGroup(GPL_GROUP_DEFAULT);		// default generate contact information
			_SubmarineSurfaceDummy->userData = this;					// 绑定逻辑对象至NxActor上
			return true;
		}
	}
#endif

	return false;
}

void PhysVehicle::SyncSubmarineSurfaceDummy()
{
	if ( !_SubmarineSurfaceDummy )
		return;

	NxVec3 RelativePosition = ZERO_VEC;
	RelativePosition.z = -GetGlobalPosition().z;

	_SubmarineSurfaceDummy->setLocalPosition(RelativePosition);
}
#endif // _CLIENT_RUNTIME

bool PhysVehicle::GetMaxDriveGear(int &drive,int &reverse)
{
	drive=0;
	reverse=0;
	VehicleData* Data = (VehicleData*)_ComponentData;
	VehicleCompDesc* CompDesc = (VehicleCompDesc*)Data->_CompDesc;
	if( CompDesc == NULL )
	{
		return false;
	}

	drive=CompDesc->_MaxDriveGear;
	reverse=CompDesc->_MaxReverseGear;
	return true;
}

bool PhysVehicle::GetMaxSteerGear(int &steer)
{
	VehicleData* Data = (VehicleData*)_ComponentData;
	VehicleCompDesc* CompDesc = (VehicleCompDesc*)Data->_CompDesc;
	if( CompDesc == NULL )
	{
		return false;
	}

	steer=CompDesc->_MaxTurnGear;
	return true;
}

NxU32 PhysVehicle::UGetAIManchineGunFiringRange(NxF32 Radius, PhysGameObject** PhysGameObjs, NxU32 PhysGameObjsNum, bool accurateCollision)
{
	NxSphere worldSphere(_PhysActor->getGlobalPosition(), Radius/P2GScale);
	NxShape* shapes[32];
	NxU32 nbShapes = _Scene->_PhysScene->overlapSphereShapes(worldSphere, NX_DYNAMIC_SHAPES,32, &(shapes[0]),NULL,(1<<GPL_SHAPE_GROUP_AIRCRAFT),NULL,accurateCollision);
	std::map<PhysGameObject*, NxShape*>	SelectedTable;
	NxU32 nbObjs=0;
	for (NxU32 i=0;i<nbShapes;i++)
	{
		if(i>=PhysGameObjsNum) return nbShapes;
		PhysGameObject* pObj = (PhysGameObject*)(shapes[i]->getActor().userData);
		if(!pObj) return nbShapes;
		if ( SelectedTable.find(pObj) == SelectedTable.end() )
		{
			PhysGameObjs[i] = pObj;
			SelectedTable[pObj] = shapes[i];
			nbObjs++;
		}
	}
	return nbObjs;
}

void PhysVehicle::GetAccuracyInfo(int& AttachedId,
								  float& AccuracyOfLinearVelocity, 
								  float& AccuracyOfAngularVelocity, 
								  float& AccuracyOfTurrentTurn,
								  float& MaxAccuracyOfTurrent,
								  float& CurAccuracyOfTurrent,
								  float& CurAccuracyMultiplier, 
								  float& FireAccuracyMultiplier,
								  float& DistAccuracyMultiplier,
								  float& FinalAccuracyMultiplier)
{
	for ( NxU32 i = 0; i < _AttachedComponents.size(); i++ )
	{
		ShipComponent* ShipComp = _AttachedComponents[i];
		if( ShipComp != NULL && IsKind(Turrent, ShipComp) && ShipComp->GetSocketAttachedTo()._PartType == _ActivePartType )
		{
			Turrent* TurrentComp = (Turrent*)ShipComp;

			AttachedId = TurrentComp->GetAttachedID();
			AccuracyOfLinearVelocity = TurrentComp->GetAccuracyOfLinearVelocity();
			AccuracyOfAngularVelocity = TurrentComp->GetAccuracyOfAngularVelocity();
			AccuracyOfTurrentTurn = TurrentComp->GetAccuracyOfTurrentTurn();
			MaxAccuracyOfTurrent = TurrentComp->GetMaxAccuracy();
			CurAccuracyOfTurrent = TurrentComp->GetCurAccuracy(0);

			CurAccuracyMultiplier = GetLimitAccuracy();
			FireAccuracyMultiplier = _FireAccuracyMultiplier;
			DistAccuracyMultiplier = _DistAccuracyMultiplier;
			FinalAccuracyMultiplier = _FinalAccuracyMultiplier;
			break;
		}
	}
}

void PhysVehicle::GetTorpedoFireParam(TorpedoFireInfo& FireInfo, bool bSubmarine)
{
	if ( !bSubmarine )
	{
		//	水面舰艇获取鱼雷发射参数
		TorpedoFireInfo::iterator it;
		for ( it = FireInfo.begin(); it != FireInfo.end(); it++ )
		{
			TorpedoFireParam& Param = *it;
			Turrent* TorpedoTube = DynamicCast(Turrent, GetShipComponent(Param.AttachId));
			if ( !TorpedoTube )
				continue;

			WeaponData* pData = DynamicCast(WeaponData, TorpedoTube->_ComponentData);
			if ( !pData )
				continue;

			size_t PortNum = Param.FiredParts.size();
			float FanAngle = GPhysGameSetting._GPL_TorpedoSalvo_DeltaAngle * (PortNum - 1);
			float DisVelocity = GPhysGameSetting._GPL_TorpedoSalvo_DeltaTangentVelocity * G2PScale * (PortNum - 1);
			float Angle = FanAngle / 2;
			float TangentVelocity = DisVelocity / 2;

			FiringParam TubeParam;
			TorpedoTube->GetFireParam(TubeParam);

			FirePartInfo::iterator it_part;
			for ( it_part = Param.FiredParts.begin(); it_part != Param.FiredParts.end(); it_part++ )
			{
				NxQuat Rot;
				Rot.fromAngleAxis(Angle, AXIS_VEC_Z);
				it_part->StartPos = TubeParam.StartPos;
				it_part->StartDir = Rot.rot(TubeParam.StartDir);

				Rot;
				Rot.fromAngleAxisFast(NxHalfPi, AXIS_VEC_Z);
				NxVec3 DisV = Rot.rot(it_part->StartDir);
				it_part->InitVelocity = it_part->StartDir * pData->_InitVelocity;
				it_part->InitVelocity += DisV * TangentVelocity;
				it_part->InitVelocity += _PhysActor->getLinearVelocity();

				Angle -= GPhysGameSetting._GPL_TorpedoSalvo_DeltaAngle;
				TangentVelocity -= GPhysGameSetting._GPL_TorpedoSalvo_DeltaTangentVelocity * G2PScale;
			}
		}
	} 
	else
	{
		//	潜水艇获取鱼雷发射参数
		size_t PortNum = FireInfo.size();
		float FanAngle = GPhysGameSetting._GPL_TorpedoSalvo_DeltaAngle * (PortNum - 1);
		float DisVelocity = GPhysGameSetting._GPL_TorpedoSalvo_DeltaTangentVelocity * G2PScale * (PortNum - 1);
		float Angle = FanAngle / 2;
		float TangentVelocity = DisVelocity / 2;

		TorpedoFireInfo::iterator it;
		for ( it = FireInfo.begin(); it != FireInfo.end(); it++ )
		{
			TorpedoFireParam& Param = *it;
			Turrent* TorpedoTube = DynamicCast(Turrent, GetShipComponent(Param.AttachId));
			if ( !TorpedoTube )
				continue;

			WeaponData* pData = DynamicCast(WeaponData, TorpedoTube->_ComponentData);
			if ( !pData )
				continue;

			FiringParam TubeParam;
			TorpedoTube->GetFireParam(TubeParam);

			FirePartInfo::iterator it_part = Param.FiredParts.begin();
			if ( it_part != Param.FiredParts.end() )
			{
				NxQuat Rot;
				Rot.fromAngleAxis(Angle, AXIS_VEC_Z);
				it_part->StartPos = TubeParam.StartPos;
				it_part->StartDir = Rot.rot(TubeParam.StartDir);

				Rot;
				Rot.fromAngleAxisFast(NxHalfPi, AXIS_VEC_Z);
				NxVec3 DisV = Rot.rot(it_part->StartDir);
				it_part->InitVelocity = it_part->StartDir * pData->_InitVelocity;
				it_part->InitVelocity += DisV * TangentVelocity;
				it_part->InitVelocity += _PhysActor->getLinearVelocity();

				Angle -= GPhysGameSetting._GPL_TorpedoSalvo_DeltaAngle;
				TangentVelocity -= GPhysGameSetting._GPL_TorpedoSalvo_DeltaTangentVelocity * G2PScale;
			}
		}
	}
}

void PhysVehicle::UGetTorpedoFireParam(TorpedoFireInfo& FireInfo, bool bSubmarine)
{
	GetTorpedoFireParam(FireInfo, bSubmarine);

	TorpedoFireInfo::iterator it;
	for ( it = FireInfo.begin(); it != FireInfo.end(); it++ )
	{
		TorpedoFireParam& Param = *it;

		FirePartInfo::iterator it_part;
		for ( it_part = Param.FiredParts.begin(); it_part != Param.FiredParts.end(); it_part++ )
		{
			it_part->StartPos *= P2GScale;
			it_part->InitVelocity *= P2GScale;
		}
	}
}

float PhysVehicle::GetCapsizalRoll()
{
	VehicleSimDeath* pSimObj = DynamicCast(VehicleSimDeath, _SimObj);
	if ( !pSimObj )
		return 0.f;

	return pSimObj->_Roll;
}

float PhysVehicle::GetCapsizalPitch()
{
	VehicleSimDeath* pSimObj = DynamicCast(VehicleSimDeath, _SimObj);
	if ( !pSimObj )
		return 0.f;

	return pSimObj->_Pitch;
}

float PhysVehicle::GetCapsizalTorqueFactor()
{
	VehicleSimDeath* pSimObj = DynamicCast(VehicleSimDeath, _SimObj);
	if ( !pSimObj )
		return 0.f;

	return pSimObj->_TorqueFactor;
}

float PhysVehicle::GetSinkForceFactor()
{
	VehicleSimDeath* pSimObj = DynamicCast(VehicleSimDeath, _SimObj);
	if ( !pSimObj )
		return 0.f;

	return pSimObj->_ForceFactor;
}

bool PhysVehicle::IsSinking()
{
	VehicleSimDeath* pSimObj = DynamicCast(VehicleSimDeath, _SimObj);
	if ( !pSimObj )
		return false;

	return true;
}

bool PhysVehicle::CheckUnderSurface()
{	
	UINT groups = 0;
	ENABLE_STATE(groups, GPL_SHAPE_GROUP_OCEAN);

	//NxQuat quat;
	//_EigenBox.rot.toQuat(quat);
	//gplDebugf(TEXT("CheckUnderSurface whit EigenBox"));
	//gplDebugf(TEXT("Center [%f, %f, %f]"), _EigenBox.center.x, _EigenBox.center.y, _EigenBox.center.z);
	//gplDebugf(TEXT("Extends [%f, %f, %f]"), _EigenBox.extents.x, _EigenBox.extents.y, _EigenBox.extents.z);
	//gplDebugf(TEXT("Rotation [%f, %f, %f, %f]"), quat.x, quat.y, quat.z, quat.w);

	if (_Scene->_PhysScene->checkOverlapOBB(_EigenBox, NX_ALL_SHAPES, groups))
		return false;

	if (GetGlobalPosition().z < 0)
		return true;

	return false;
}

void PhysVehicle::RemoveCollision()
{
	if ( !_PhysActor )
		return;

	UINT Num = _PhysActor->getNbShapes();
	NxShape* const* shapelist = _PhysActor->getShapes();
	for ( UINT idx = 0; idx < Num; idx++ )
	{
		NxShape* pShape = shapelist[idx];
		if ( !pShape )
			continue;

		if ( GPL_SHAPE_GROUP_VEHICLE_SHIP != pShape->getGroup() )
			continue;

		pShape->setGroup(GPL_SHAPE_GROUP_SINK_VEHICLE);
	}

	VehicleSimDeath* pSimObj = DynamicCast(VehicleSimDeath, _SimObj);
	if ( pSimObj )
	{
		pSimObj->ResetForce();
		pSimObj->ResetTorque();
	}

	_PhysActor->putToSleep();
}

void PhysVehicle::RecoverCollision()
{
	if ( !_PhysActor )
		return;

	UINT Num = _PhysActor->getNbShapes();
	NxShape* const* shapelist = _PhysActor->getShapes();
	for ( UINT idx = 0; idx < Num; idx++ )
	{
		NxShape* pShape = shapelist[idx];
		if ( !pShape )
			continue;
		
		NxCollisionGroup GroupFlag = pShape->getGroup();
		if ( GPL_SHAPE_GROUP_SINK_VEHICLE != GroupFlag )
			continue;
		pShape->setGroup(GPL_SHAPE_GROUP_VEHICLE_SHIP);
	}
}

void PhysVehicle::SetDetectable(bool bDetectable)
{
	_bDetectable = bDetectable;

	if (_bDetectable)
		_PhysActor->clearActorFlag(NX_AF_DISABLE_COLLISION);
	else
		_PhysActor->raiseActorFlag(NX_AF_DISABLE_COLLISION);
}

void PhysVehicle::InitCriticalFireData(int PartType, int MinTimes, int MaxTimes, float Accuracy)
{
	if (PartType == EPT_MainArtillery)
	{
		_MainArtilleryFireData.NecessaryTimesMin = MinTimes;
		_MainArtilleryFireData.NecessaryTimesMax = MaxTimes;
		_MainArtilleryFireData.AccuracyMultiplier = Accuracy;
		ResetCriticalFireData(_MainArtilleryFireData);
	}
	else if (PartType == EPT_SubArtillery)
	{
		_SubArtilleryFireData.NecessaryTimesMin = MinTimes;
		_SubArtilleryFireData.NecessaryTimesMax = MaxTimes;
		_SubArtilleryFireData.AccuracyMultiplier = Accuracy;
		ResetCriticalFireData(_SubArtilleryFireData);
	}

}

void PhysVehicle::ClearCriticalFireRecord(int PartType)
{
	if (PartType == EPT_MainArtillery)
		_MainArtilleryFireData.AccumTimes = 0;
	else if (PartType == EPT_SubArtillery)
		_SubArtilleryFireData.AccumTimes = 0;
}

void PhysVehicle::ResetCriticalFireData(int PartType)
{
	if (PartType == EPT_MainArtillery)
		ResetCriticalFireData(_MainArtilleryFireData);
	else if (PartType == EPT_SubArtillery)
		ResetCriticalFireData(_SubArtilleryFireData);
}

float PhysVehicle::GetCriticalFireAccuracyMultiplier(int PartType)
{
	CriticalFireData* pData = NULL;

	if (PartType == EPT_MainArtillery)
		pData = &_MainArtilleryFireData;
	else if (PartType == EPT_SubArtillery)
		pData = &_SubArtilleryFireData;

	if (!pData)
		return 1.f;

	if (pData->NecessaryTimesMax == 0)
		return 1.f;

	if (pData->NecessaryTimes == pData->AccumTimes)
	{
		ResetCriticalFireData(*pData);
		return 1.f - pData->AccuracyMultiplier;
	}

	pData->AccumTimes++;
	return 1.f;
}

void PhysVehicle::ResetCriticalFireData(CriticalFireData& InData)
{
	InData.NecessaryTimes = (InData.NecessaryTimesMax != InData.NecessaryTimesMin)
							? NxMath::rand(InData.NecessaryTimesMin, InData.NecessaryTimesMax)
							: InData.NecessaryTimesMin;
	InData.AccumTimes = 0;
}

}