//------------------------------------------------------------------------
//	@brief	舰载机物理对象
//	@author	chenpu
//	@date	2013-1-24
//------------------------------------------------------------------------
#include "PhysXSupport.h"
#include "PhysCommonDefine.h"
#include "PhysGameObject.h"
#include "PhysGameObjectAircraft.h"
#include "DirectionalCircle.h"

namespace GPL {

	IMPLEMENT_RTTI(PhysAircraft, PhysGameObject)

	PhysAircraft::~PhysAircraft()
	{
	}

	bool PhysAircraft::Init()
	{
		if( !PhysGameObject::Init() )
			return false;

		if ( !_FSM.Init(this) )
			return false;

		RESET_STATE(_Flags);

		_Forward = AXIS_VEC_Y;

		return true;
	}

	bool PhysAircraft::PushInitState(AircraftAction action, const NxVec3& pos, const NxVec3& dir, PhysGameObject* obj)
	{
		if ( NULL != _FSM.GetActiveState() )
		{
			gplDebugf(TEXT("Already had a running state"));
			return false;
		}

		_FSM.PushAction(action, pos, dir, obj);
		bool rst = _FSM.PopAction();

		return rst;
	}

	void PhysAircraft::Tick(float DeltaTime)
	{
		_FSM.Update(DeltaTime);

#ifdef _SERVER_RUNTIME
		if ( CHECK_STATE(_Flags, Collimate) )
		{
			//	轰炸机俯冲临界条件
			float Limit = _WorldTransform.t.z / GetSwoopSlope();
			if (_WorldTransform.t.distance(_UTargetPos * G2PScale) < Limit * 2)
				ExecuteAction();
		}
#endif // _SERVER_RUNTIME

		PostFix(DeltaTime);

		//	Use Own World-Matrix
		PhysGameObject::SetGlobalPose(_WorldTransform);
		_WorldTransform.M.multiply(AXIS_VEC_X, _Forward);
		_Forward.z = 0;
		_Forward.normalize();

		//	舰载机死亡状态下不进行攻击范围及避障检测
		if (_SquadIndex < 0)
			return;
		
		ScanAttackRange(DeltaTime);

		ScanBlocks(DeltaTime);
	}

	bool PhysAircraft::OnCommand(OperationCommand cmd, const NxVec3& tar, const NxVec3& dir, PhysGameObject* pObj /* = NULL */)
	{
		CommandParse(cmd, tar, dir, pObj);

		if ( _LayerChange != cmd )
			ExecuteAction();

#ifdef _SERVER_RUNTIME
		//	若指令未能立即执行，抛弃有目标对象的指令
		UINT PendingActionNum = _FSM.GetPendingActionNum();
		if (PendingActionNum > 0)
		{
			ActionData* pActionData = _FSM.GetPendingAction(PendingActionNum - 1);
			if (pActionData != NULL && pActionData->_obj != NULL)
				_FSM.ClearActions();
		}
#endif // _SERVER_RUNTIME

		switch ( GetAircraftType() )
		{
		case EPT_Bomber:
			{
				if ( _Attack != cmd )
				{
					if ( CHECK_STATE(_Flags, Collimate) )
					{
						if ( _LayerChange == cmd )
						{
							ModifyPrecisionFactor(GetLayerAffect());
						}
						else
						{
							DISABLE_STATE(_Flags, Collimate);
						}
					}
				}
#ifdef _SERVER_RUNTIME
				else if (!CHECK_STATE(_Flags, Collimate))
				{
					ENABLE_STATE(_Flags, Collimate);
					InitPrecisionFactor();
				}
				else
				{
					OnBombTargetChange(tar);
				}
#endif // _SERVER_RUNTIME
			}
			break;

		case EPT_TorpedoPlane:
			{
#ifdef _SERVER_RUNTIME
				//	鱼雷机收到拉升指令
				if ( _LayerChange == cmd && dir.equals(AXIS_VEC_Z, 0.01f) )
				{
					//	若正在向投弹点移动，则取消攻击指令
					if ( Attack == GetIntent() )
						_FSM.DropLastAction();
				}
#endif // _SERVER_RUNTIME
			}
			break;
		}

		return CHECK_STATE(_Flags, VerticalMove);
	}

	bool PhysAircraft::UOnCommand(OperationCommand cmd, const NxVec3& tar, const NxVec3& dir, PhysGameObject* pObj /* = NULL */)
	{
		NxVec3 FixedDir(dir.x, dir.y, 0);
		FixedDir.normalize();

		if ( cmd == _LayerChange )
			FixedDir = dir;

		return OnCommand(cmd, tar * G2PScale, FixedDir, pObj);
	}

	bool PhysAircraft::PendingCommand(OperationCommand cmd, const NxVec3& tar, const NxVec3& dir, PhysGameObject* pObj /* = NULL */)
	{
		bool Replaced = false;

		if ( _Cruise != cmd )
			return Replaced;

		//	立即执行条件
		bool ImmediateExec = false;
		GameObjectState* pState = _FSM.GetActiveState();
		if ( !pState )
			ImmediateExec = true;
		else if ( pState->GetEnumValue() == Circle && _FSM.GetPendingActionNum() == 0 )
			ImmediateExec = true;

		if ( (int)_FSM.GetPendingActionNum() == GPhysGameSetting._GPL_Aircraft_ActionCache_Length )
		{
			_FSM.DropLastAction();
			Replaced = true;
		}

		_FSM.PushAction(Transfer, tar, ZERO_VEC, NULL);

		AircraftList::iterator iter;
		for ( iter = _Wings.begin(); iter != _Wings.end(); iter++ )
		{
			PhysAircraft* Wing = *iter;
			if ( Wing )
				Wing->PendingCommand(cmd, tar, dir, pObj);
		}

		if ( ImmediateExec )
			ExecuteAction();

		_FSM.CalcPendingMoveDir();

		return Replaced;
	}

	bool PhysAircraft::UPendingCommand(OperationCommand cmd, const NxVec3& tar, const NxVec3& dir, PhysGameObject* pObj /* = NULL */)
	{
		return PendingCommand(cmd, tar * G2PScale, dir, pObj);
	}

	void PhysAircraft::ChangeAction(AircraftAction Act, const NxVec3& tar, const NxVec3& dir)
	{
		if (!IsAlive())
			return;

		GameObjectState* pState = _FSM.GetActiveState();
		if ( pState )
		{
			GPL::GameObjectEvent args;
			pState->OnLeave(args);

#ifdef _CLIENT_RUNTIME
			if (pState->GetEnumValue() == Ready && Act != TakeOff)
			{
				if ( _Header != NULL )
					ENABLE_STATE(_Flags, LockHeader);
			}
#endif // _CLIENT_RUNTIME
		}

		_FSM.ClearActions();
		_FSM.PushAction(Act, tar, dir, NULL);
		_FSM.PopAction();

		if (Act == TakeOff)
			return;

		if (Act == Attack && GetAircraftType() == EPT_Bomber)
		{
#ifdef _CLIENT_RUNTIME
			AircraftList::iterator iter = _Wings.begin();
			while (iter != _Wings.end())
			{
				PhysAircraft* Wing = *iter;
				if (NULL == Wing)
				{
					iter = _Wings.erase(iter);
					continue;
				}

				if (!Wing->IsAlive())
				{
					iter = _Wings.erase(iter);
					continue;
				}

				Wing->ChangeAction(Transfer, Wing->GetGlobalPosition() + Wing->GetForward() * 1024, Wing->GetForward());
				Wing->SetFlag(LockHeader, false);
				Wing->SetSpeedFactor(GPhysGameSetting._GPL_Aircraft_DiveAttack_SpeedFactor);
				iter++;
			}
#endif // _CLIENT_RUNTIME
			return;
		}

		//	动作变更下发僚机
		if ( !_Header )
		{
			AircraftList::iterator iter = _Wings.begin();
			while ( iter != _Wings.end() )
			{
				PhysAircraft* Wing = *iter;
				if ( NULL == Wing )
				{
					iter = _Wings.erase(iter);
					continue;
				}

				if ( !Wing->IsAlive() )
				{
					iter = _Wings.erase(iter);
					continue;
				}

				Wing->ChangeAction(Act, tar + Wing->GetFormationOffset(dir), dir);
				iter++;
			}
		}
	}

	void PhysAircraft::UChangeAction(AircraftAction Act, const NxVec3& tar, const NxVec3& dir)
	{
		NxVec3 FixedDir(dir.x, dir.y, 0);
		FixedDir.normalize();
		ChangeAction(Act, tar * G2PScale, FixedDir);
	}

	void PhysAircraft::ForceTransform(const NxVec3& Pos, const NxVec3& Dir)
	{
		AircraftState* pState = (AircraftState*)_FSM.GetActiveState();
		if ( !pState )
			return;

		pState->Coordinate(Pos, Dir);
	}

	NxMat34 PhysAircraft::TransformSimulate(float Time, bool ignoreDir /* = true */)
	{
		if ( NULL != _Header )
		{
			NxMat34 Trans = _Header->TransformSimulate(Time, false);
			NxQuat Rot;
			Trans.M.toQuat(Rot);
			NxVec3 Dir = Rot.rot(AXIS_VEC_X);
			Trans.t += GetFormationOffset(Dir);
			return Trans;
		}

		AircraftState* pState = (AircraftState*)_FSM.GetActiveState();
		if ( NULL == pState )
			return _WorldTransform;

		NxMat34 Trans;
		NxVec3 Dir = ZERO_VEC;
		pState->MovementForecast(Time, Trans.t, Dir);

		if ( !ignoreDir )
		{
			NxQuat Rot = Dir2Quat(Dir);
			Trans.M.fromQuat(Rot);
		}

		return Trans;
	}

	void PhysAircraft::OnCrash(bool bShoot/* = true*/)
	{
#ifdef _SERVER_RUNTIME
		//	起飞过程还处于滑行阶段就被击坠的情况，优先处理后续起飞通知
		if (bShoot)
		{
			AircraftState* State = (AircraftState*)GetCurrentState();
			if (NULL != State && TakeOff == State->GetEnumValue() && State->GetMotionNum() > 1)
				OnLeaveBoard();
		}
#endif // _SERVER_RUNTIME

		SetOwner(NULL);

		if ( _Header )
		{
			//gplDebugf(TEXT("PhysAircraft::OnCrash Wing"));

			RESET_STATE(_Flags);
			_Header->UnregisterWing(this);
			_Header = NULL;

			_SquadIndex = -1;
		}
		else
		{
			//gplDebugf(TEXT("PhysAircraft::OnCrash Header"));

			_SquadIndex = -1;

#ifdef _CLIENT_RUNTIME
			if ( NULL != _InterceptTrigger )
			{
				DestroyPhysGameObject(_Scene, _InterceptTrigger);
				_InterceptTrigger = NULL;
			}

			if ( NULL != _ChaseTrigger )
			{
				DestroyPhysGameObject(_Scene, _ChaseTrigger);
				_ChaseTrigger = NULL;
			}
#endif // _CLIENT_RUNTIME

			//	新长机
			PhysAircraft* pNewHeader = NULL;
			int index = 0;
			AircraftList::iterator iter = _Wings.begin();
			while ( iter != _Wings.end() )
			{
				PhysAircraft* pAircraft = *iter;
				if ( NULL == pAircraft )
				{
					iter = _Wings.erase(iter);
					continue;
				}

				if ( !pAircraft->IsAlive() )
				{
					iter = _Wings.erase(iter);
					continue;
				}

#ifdef _SERVER_RUNTIME
				if (pAircraft->CheckFlag(LockHeader))
				{
					AircraftState* State = (AircraftState*)GetCurrentState();
					if (NULL != State && Circle != State->GetEnumValue())
						pAircraft->SetGlobalPosition(pAircraft->GetFormationGlobalPosition());
					else
						pAircraft->SetGlobalPosition(GetGlobalPosition());

					pAircraft->SetForward(_Forward);
					pAircraft->_UTargetPos = _UTargetPos;
					pAircraft->_UTargetDir = _UTargetDir;
				}
#endif // _SERVER_RUNTIME
				pAircraft->SetSquadIndex(index);

				if ( index == 0 )
					pNewHeader = pAircraft;

				index++;
				iter++;
			}

			_Wings.clear();

			if ( NULL != pNewHeader )
			{
#ifdef _SERVER_RUNTIME
				//	新长机继承原锁定关系
				if ( _LockTarget )
					pNewHeader->Lock(_LockTarget);

				//	通知重新锁定
				while ( !_LockHosts.empty() )
					_LockHosts.front()->Lock(pNewHeader);

				AircraftState* State = (AircraftState*)GetCurrentState();
				if ( State != NULL )
				{
					if ( State->GetEnumValue() == Transfer )
					{
						pNewHeader->UChangeAction(Transfer, _UTargetPos, _UTargetDir);
						pNewHeader->PendingActionCopy(_FSM);
					}
					else if ( State->GetEnumValue() == Attack )
					{
						if ( GetAircraftType() == EPT_Fight && NULL != _LockTarget )
						{
							pNewHeader->OnCommand(_Attack, ZERO_VEC, ZERO_VEC, _LockTarget);
						}
					}
					else if (State->GetEnumValue() != TakeOff)
					{
						pNewHeader->UChangeAction((AircraftAction)State->GetEnumValue(), _UTargetPos, _UTargetDir);
						pNewHeader->PendingActionCopy(_FSM);
						//AircraftState* StateNew = (AircraftState*)pNewHeader->GetCurrentState();
						//if ( NULL != StateNew && State->GetEnumValue() == StateNew->GetEnumValue() )
						//{
						//	while (StateNew->GetMotionNum() > State->GetMotionNum())
						//	{
						//		StateNew->ForceStep();
						//	}

						//	OrbitParams p1, p2;
						//	if ( State->GetMotionParam(0, p1) && StateNew->GetMotionParam(0, p2) )
						//	{
						//		p2._PassedTime = p1._PassedTime;
						//		StateNew->ModifyMotionParam(0, p2);
						//	}
						//}
					}
				}
#endif // _SERVER_RUNTIME

				if ( CHECK_STATE(_Flags, Collimate) )
				{
					pNewHeader->SetFlag(Collimate, true);
					pNewHeader->SetPrecisionFactor(_PrecisionFactor);
				}
			}
			else
			{
				//	取消锁定
				if ( _LockTarget )
					_LockTarget->OnUnlock(this);

				//	通知目标丢失
				while ( !_LockHosts.empty() )
					_LockHosts.front()->OnTargetLost(this);
			}
		}
		
#ifdef _SERVER_RUNTIME
		_FSM.ClearActions();
		_FSM.PushAction(Crash, ZERO_VEC, ZERO_VEC, bShoot ? NULL : this);
		_FSM.PopAction();
#endif // _SERVER_RUNTIME
	}

	void PhysAircraft::OnActionFinished()
	{
		if ( _BehaviorReport )
			_BehaviorReport->OnAction(this, *_FSM.GetActiveState());

		ExecuteAction();
	}

	void PhysAircraft::NonCommandAction()
	{
		if ( _FSM.GetPendingActionNum() > 0 )
			return;

		NxVec3 Center;

		if ( NULL == _Header )
			Center = GetGlobalPosition();
		else
		{
			Center = _Header->GetConvoluteCenter();

			if ( Center.equals(ZERO_VEC, 0.01f) )
				Center = GetGlobalPosition();
		}

		_UTargetPos = Center * P2GScale;
		_UTargetDir = ZERO_VEC;

		_FSM.PushAction(Circle, Center, ZERO_VEC, NULL);
		_FSM.PopAction();
	}

	float PhysAircraft::Dive(float DeltaTime)
	{
		float PrevHeight = _WorldTransform.t.z;

		//	Attention : _VerticalSpeed & _GPL_Aircraft_DiveAttack_LimitSpeed Should be Negative
		if (NxMath::equals(_VerticalSpeed, GPhysGameSetting._GPL_Aircraft_DiveAttack_LimitSpeed, 0.001f))
		{
			_WorldTransform.t.z += GPhysGameSetting._GPL_Aircraft_DiveAttack_LimitSpeed * DeltaTime;
		}
		else if (_VerticalSpeed > GPhysGameSetting._GPL_Aircraft_DiveAttack_LimitSpeed)
		{
			_WorldTransform.t.z += _VerticalSpeed * DeltaTime + 0.5f * GetDiveAcceleration() * DeltaTime * DeltaTime;
			_VerticalSpeed = NxMath::max(_VerticalSpeed + GetDiveAcceleration() * DeltaTime
										, GPhysGameSetting._GPL_Aircraft_DiveAttack_LimitSpeed);
		}
		
		return _WorldTransform.t.z - PrevHeight;
	}

	float PhysAircraft::Pull(float DeltaTime)
	{
		float PrevHeight = _WorldTransform.t.z;
		_WorldTransform.t.z += _VerticalSpeed * DeltaTime + 0.5f * _ExigencePullAcceleration * DeltaTime * DeltaTime;
		_VerticalSpeed += _ExigencePullAcceleration * DeltaTime;

		return _WorldTransform.t.z - PrevHeight;
	}

	float PhysAircraft::Incline(float DeltaTime)
	{
		float LayerHeight = RouteAssist::GetSingletonRef().GetLayerHeight(Higher);
		float PrevHeight = _WorldTransform.t.z;
		_VerticalSpeed = GetLayerChangeSpeed();

		_WorldTransform.t.z += DeltaTime * _VerticalSpeed;
		if (_WorldTransform.t.z > LayerHeight)
		{
			_WorldTransform.t.z = LayerHeight;
			_VerticalSpeed = 0;
			SetFlag(VerticalMove, false);
			SetFlag(Ascend, false);
		}

		return _WorldTransform.t.z - PrevHeight;
	}

	float PhysAircraft::Decline(float DeltaTime)
	{
		float LayerHeight = RouteAssist::GetSingletonRef().GetLayerHeight(ExtremeLow);
		float PrevHeight = _WorldTransform.t.z;
		_VerticalSpeed = -GetLayerChangeSpeed();

		_WorldTransform.t.z += DeltaTime * _VerticalSpeed;
		if (_WorldTransform.t.z < LayerHeight)
		{
			_WorldTransform.t.z = LayerHeight;
			_VerticalSpeed = 0;
			SetFlag(VerticalMove, false);
			SetFlag(Descend, false);
		}

		return _WorldTransform.t.z - PrevHeight;
	}

	float PhysAircraft::FallingDown(float DeltaTime)
	{
		float PrevHeight = _WorldTransform.t.z;

#if 0
		_WorldTransform.t.z += _VerticalSpeed * DeltaTime - 5.f * DeltaTime * DeltaTime;
		_VerticalSpeed -= 10.f * DeltaTime;
		gplDebugf(TEXT("Crashed Plane Height[%f]"), _WorldTransform.t.z);
#else
		if (NxMath::equals(_VerticalSpeed, GPhysGameSetting._GPL_Aircraft_DiveAttack_LimitSpeed, 0.001f))
		{
			_WorldTransform.t.z += GPhysGameSetting._GPL_Aircraft_DiveAttack_LimitSpeed * DeltaTime;
		}
		else if (_VerticalSpeed > GPhysGameSetting._GPL_Aircraft_DiveAttack_LimitSpeed)
		{
			_WorldTransform.t.z += _VerticalSpeed * DeltaTime - 5.f * DeltaTime * DeltaTime;
			_VerticalSpeed = NxMath::max(_VerticalSpeed - 10.f * DeltaTime
				, GPhysGameSetting._GPL_Aircraft_DiveAttack_LimitSpeed);
		}
#endif

		return _WorldTransform.t.z - PrevHeight;
	}

	void PhysAircraft::OnLeaveBoard()
	{
		PhysAircraft* pNext = PhysAircraft::GetAircraft((PhysVehicle*)_Owner, _SquadID, _SquadIndex + 1);
		if (!pNext)
			return;

		GameObjectState* pState = pNext->GetCurrentState();
		if (NULL != pState && Ready == pState->GetEnumValue())
			pNext->OnCommand(_TakeOff, ZERO_VEC, ZERO_VEC);
	}

	void PhysAircraft::OnTargetLost(PhysGameObject* Target)
	{
		switch ( GetAircraftType() )
		{
		case EPT_Scout:
			{
				_FSM.ClearActions();
				NonCommandAction();
			}
			break;

		case EPT_Fight:
			{
				if ( _BehaviorReport )
					_BehaviorReport->OnChaseAreaEvent(this, Target, false);
			}
			break;
		}

		PhysGameObject::OnTargetLost(Target);
	}

	PhysAircraft* PhysAircraft::GetAircraft(PhysVehicle* pOwner, UINT squad, UINT index)
	{
		if ( !pOwner )
			return NULL;

		PhysGameObjectList vChildren = pOwner->GetChildren();
		PhysGameObjectList::iterator it = vChildren.begin();
		for ( ; it != vChildren.end(); it++ )
		{
			PhysAircraft* pAircraft = DynamicCast(PhysAircraft, (*it));
			if ( !pAircraft )
				continue;

			if ( pAircraft->GetSquadID() != squad )
				continue;
			
			if ( pAircraft->GetSquadIndex() == (int)index )
				return pAircraft;
		}

		return NULL;
	}

	NxVec3 PhysAircraft::GetFormationOffset()
	{
		//------------------------------------------------------------------------
		//	offset means vector from a point of any index to the 0-point
		//	their distance was also consisted
		//							0		
		//						   / \		
		//						  1   2		
		//						 /     \	
		//						3       4	
		//	degree(120) and distance(3) can be considered as parameters
		//------------------------------------------------------------------------
		if ( !_Header )
			return ZERO_VEC;

		AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
		if ( NULL == pData )
			return ZERO_VEC;

		//NxVec3 Offset;
		NxQuat Rot;
		Rot = Dir2Quat(_Header->GetForward());
		NxVec3 Offset = Rot.rot(pData->_FormationOffset[_SquadIndex].t);
		//_Header->GetGlobalOritation().multiply(pData->_FormationOffset[_SquadIndex].t, Offset);

		return Offset;
	}

	NxVec3 PhysAircraft::GetFormationOffset(const NxVec3& dir)
	{
		if ( dir.equals(ZERO_VEC, 0.01f) )
			return GetFormationOffset();

		if ( !_Header )
			return ZERO_VEC;

		AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
		if ( NULL == pData )
			return ZERO_VEC;

		NxQuat Rot = Dir2Quat(dir);
		//NxMat33 Trans;
		//Trans.fromQuat(Rot);
		NxVec3 Offset = Rot.rot(pData->_FormationOffset[_SquadIndex].t);
		//Trans.multiply(pData->_FormationOffset[_SquadIndex].t, Offset);

		return Offset;
	}

	NxVec3 PhysAircraft::GetFormationGlobalPosition()
	{
		if ( !_Header )
			return _WorldTransform.t;

		AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
		if ( NULL == pData )
			return _WorldTransform.t;

		NxVec3 Pos;
		_Header->GetGlobalPos().multiply(pData->_FormationOffset[_SquadIndex].t, Pos);

		return Pos;
	}

	void PhysAircraft::ApplyFormationTrans()
	{
		if ( !_Header )
			return;

		AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
		if ( NULL == pData )
			return;

		_Header->GetGlobalPos().multiply(pData->_FormationOffset[_SquadIndex], _WorldTransform);
	}

	void PhysAircraft::InitPhys()
	{
		AircraftData* data = (AircraftData*)_ComponentData;
		if ( data == NULL || data->_PhysModelDesc == NULL )
			return;

		AircraftCompDesc* CompDesc = (AircraftCompDesc*)data->_CompDesc;
		if ( CompDesc == NULL )
			return;

		NxActorDesc ActorDesc;
		std::vector<NxShapeDesc*> CachedShapeDescList;
		for(NxU32 ShapeIndex = 0; ShapeIndex < data->_PhysModelDesc->_ShapeDescList.size(); ++ShapeIndex)
		{
			NxShapeDesc* XShapeDesc = NULL;
			ShapeDesc* CustomShapeDesc = data->_PhysModelDesc->_ShapeDescList[ShapeIndex];

			// create NxShapeDesc data
			XShapeDesc = CreateComponentShape(CustomShapeDesc);
			if( XShapeDesc != NULL )
			{
				XShapeDesc->group = GPL_SHAPE_GROUP_AIRCRAFT;

				ActorDesc.shapes.push_back(XShapeDesc);

				CachedShapeDescList.push_back(XShapeDesc);
			}
			else
			{
				gplDebugf(TEXT("ERROR: Faied to CreateComponentShape CompId[%d] PhysId[%s] ShapeIndex[%d]"), data->_CompDesc->_CompId, data->_PhysModelDesc->_PhysId.c_str(), ShapeIndex);
			}
		}
		ActorDesc.globalPose = data->_InitGlobalPose;
		ActorDesc.density = 1.0f;
		//ActorDesc.dominanceGroup = ...;

		NxBodyDesc BodyDesc;
		BodyDesc.linearDamping = 0.f;
		BodyDesc.angularDamping = 0.f;
		BodyDesc.flags |= NX_BF_DISABLE_GRAVITY;
		BodyDesc.sleepEnergyThreshold = 0.5f;
		BodyDesc.sleepDamping = 0.2f;
		ActorDesc.body = &BodyDesc;

		NxU32 ValidCode = ActorDesc.checkValid();
		if( ValidCode == 0 )
		{
			_PhysActor = _Scene->_PhysScene->createActor(ActorDesc);
		}

		ReleaseShapeDescList(CachedShapeDescList);

		if( _PhysActor != NULL )
		{
			_PhysActor->setGroup(GPL_GROUP_DEFAULT);		// default generate contact information
			_PhysActor->userData = this;		// 绑定逻辑对象至NxActor上
		}
	}

	void PhysAircraft::PostInitPhys()
	{
		PhysGameObject::PostInitPhys();

		_PhysActor->wakeUp();

		//	限制转向半径
		AircraftData* data = (AircraftData*)_ComponentData;
		if ( data != NULL )
			data->_MinRadius = NxMath::max(1.f, data->_MinRadius);
	}

	void PhysAircraft::PrevTermPhys()
	{
		SetOwner(NULL);

		_Header = NULL;
		_Wings.clear();

#ifdef _CLIENT_RUNTIME
		if ( NULL != _InterceptTrigger )
		{
			DestroyPhysGameObject(_Scene, _InterceptTrigger);
			_InterceptTrigger = NULL;
		}

		if ( NULL != _ChaseTrigger )
		{
			DestroyPhysGameObject(_Scene, _ChaseTrigger);
			_ChaseTrigger = NULL;
		}
#endif // _CLIENT_RUNTIME

		PhysGameObject::PrevTermPhys();
	}

	void PhysAircraft::CommandParse(OperationCommand cmd, const NxVec3& tar, const NxVec3& dir, PhysGameObject* pObj)
	{
		if (cmd != _LayerChange)
			_FSM.ClearActions();

		switch ( cmd )
		{
		case _TakeOff:
			_FSM.PushAction(TakeOff, ZERO_VEC, ZERO_VEC, NULL);
			break;

		case _Cruise:
			_FSM.PushAction(Transfer, tar, dir, NULL);
			break;

		case _Convoy:
			_FSM.PushAction(Transfer, ZERO_VEC, ZERO_VEC, pObj);
			break;

		case _LayerChange:
			{
				GameObjectState* pState = _FSM.GetActiveState();
				if ( pState )
				{
					if (Transfer == pState->GetEnumValue() || Circle == pState->GetEnumValue())
					{
						bool bAscend = dir.sameDirection(AXIS_VEC_Z);
						SetFlag(VerticalMove, true);
						SetFlag(Ascend, bAscend);
						SetFlag(Descend, !bAscend);
					}
				}
			}
			break;

		case _Recovery:
			{
#ifdef _SERVER_RUNTIME
				NxVec3 _EndPoint = GetStartLocation();			//	终点
				NxVec3 _CatchPoint = GetCatchLocation();		//	着舰点
				NxVec3 _Dir = _EndPoint - _CatchPoint;
				_Dir.z = 0;
				_Dir.normalize();

				NxVec3 _TransPoint = RouteAssist::GetSingletonRef().CalcStepIn(this, Landing, Higher, _CatchPoint, _Dir);
				_TransPoint += GetFormationOffset(_Dir);		//	僚机偏移

				if ( !GetGlobalPosition().equals(_TransPoint, 0.1f) )
					_FSM.PushAction(Transfer, _TransPoint, _Dir, NULL);

				_FSM.PushAction(Landing, _EndPoint, _Dir, NULL);
#endif // _SERVER_RUNTIME

#ifdef _CLIENT_RUNTIME
				_FSM.PushAction(Landing, tar, dir, NULL);
#endif // _CLIENT_RUNTIME
			}
			break;

		case _Attack:
			{
				EPartType type = GetAircraftType();
				if ( EPT_Scout == type )
				{
					_FSM.PushAction(Transfer, tar, dir, pObj);
				}
				else if ( EPT_Fight == type )
				{
					_FSM.PushAction(Attack, tar, dir, pObj);
				}
				else if ( EPT_Bomber == type )
				{
#ifdef _SERVER_RUNTIME
					NxVec3 _AttackPoint = tar;
					_AttackPoint.z = RouteAssist::GetSingletonRef().GetLayerHeight(Lower);

					_FSM.PushAction(Transfer, tar, ZERO_VEC, NULL);
					_FSM.PushAction(Attack, _AttackPoint, ZERO_VEC, NULL);
#endif // _SERVER_RUNTIME

#ifdef _CLIENT_RUNTIME
					_FSM.PushAction(Attack, tar, dir, NULL);
#endif // _CLIENT_RUNTIME
				}
				else if ( EPT_TorpedoPlane == type )
				{
#if 0
					AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
					float fSpeed = (pData != NULL) ? pData->_Speed : DEFAULT_SPEED;

					NxVec3 _AttackPoint = tar;
					_AttackPoint.z = RouteAssist::GetSingletonRef().GetLayerHeight(ExtremeLow);

					NxVec3 _StepInPos = _AttackPoint - dir * GetExLowFlyingTime() * fSpeed;
					NxVec3 _EntryPos;
					GameObjectState* pState = _FSM.GetActiveState();
					if ( pState && pState->GetEnumValue() != TakeOff )
						_EntryPos = RouteAssist::GetSingletonRef().CalcStepIn(this, Attack, GetFlightLayer(), _StepInPos, dir);
					else 
						_EntryPos = RouteAssist::GetSingletonRef().CalcStepIn(this, Attack, Higher, _StepInPos, dir);

					NxVec3 vOffset = GetFormationOffset(dir);
					_FSM.PushAction(Transfer, _EntryPos + vOffset, dir, NULL);
					_FSM.PushAction(Attack, _AttackPoint + vOffset, dir, NULL);
#else
					NxVec3 Target = tar + GetFormationOffset(dir);
					NxReal Rad = NxMath::acos(dir.dot(GetForward()));
					NxReal Deg = NxMath::radToDeg(Rad);
					NxVec3 Delta = Target - GetGlobalPosition();
					Delta.normalize();

					if (!Delta.sameDirection(dir)
						&& Target.distance(GetGlobalPosition()) < GPhysGameSetting._GPL_TorpedoPlane_LaunchTorlerance * G2PScale
						&& Deg < GPhysGameSetting._GPL_TorpedoPlane_LaunchAngleTorlerance)
					{
						SetForward(dir);
						_FSM.PushAction(Attack, GetGlobalPosition(), dir, NULL);
					}
					else
					{
						_FSM.PushAction(Transfer, Target, dir, NULL);
						_FSM.PushAction(Attack, Target, dir, NULL);
					}
#endif
				}
			}
			break;
		}
	}

	DWORD PhysAircraft::ExecuteAction()
	{
		GameObjectState* Active = NULL;

		if (_FSM.PopAction())
		{
			Active = _FSM.GetActiveState();
			return (NULL != Active) ? Active->GetEnumValue() : 0xFF;
		}

		NonCommandAction();

		Active = _FSM.GetActiveState();
		return (NULL != Active) ? Active->GetEnumValue() : 0xFF;
	}

	void PhysAircraft::PendingActionCopy(AircraftFSM& fsm)
	{
		UINT num = fsm.GetPendingActionNum();
		for ( UINT idx = 0; idx < num; idx++ )
		{
			ActionData* pData = fsm.GetPendingAction(idx);
			if ( !pData )
				continue;

			_FSM.PushAction(pData->_action, pData->_tar, pData->_dir, pData->_obj);
		}
	}

	void PhysAircraft::PostFix(float DeltaTime)
	{
		if ( !_Header )
			return;

		if ( CHECK_STATE(_Flags, LockHeader) )
		{
			SetGlobalPosition(_Header->GetGlobalPosition() + GetFormationOffset());
			SetForward(_Header->GetForward());
#ifdef _CLIENT_RUNTIME
			SetGlobalOritation(_Header->GetGlobalOritation());
#endif // _CLIENT_RUNTIME
		}
	}

	void PhysAircraft::ScanAttackRange(float DeltaTime)
	{
		if (NULL != _Header)
			return;

		AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
		if ( NULL == pData )
			return;

		if ( EPT_Fight != pData->_AircraftType )
			return;

		float fThreshold = pData->_AttackExtent * pData->_AttackExtent;

		PhysGameObjectList::iterator it = _Scene->_PhysGameObjectList.begin();
		for ( ; it != _Scene->_PhysGameObjectList.end(); it++ )
		{
			PhysAircraft* pAircraft = DynamicCast(PhysAircraft, (*it));
			if ( !pAircraft )
				continue;

			AircraftList::iterator iter = _AttackableList.empty() ? 
				_AttackableList.end() :
				std::find(_AttackableList.begin(), _AttackableList.end(), pAircraft);

			float fDistance = GetGlobalPosition().distanceSquared(pAircraft->GetGlobalPosition());
			if ( fDistance > fThreshold )
			{
				//	可攻击距离
				if ( iter != _AttackableList.end() )
				{
					_AttackableList.erase(iter);

					if ( _BehaviorReport )
						_BehaviorReport->OnAttack(this, pAircraft, false);
				}
			} 
			else
			{
				//	可攻击角度
				NxVec3 vForward = GetForward();
				vForward.z = 0;
				vForward.normalize();

				NxVec3 vDir = pAircraft->GetGlobalPosition() - GetGlobalPosition();
				vDir.z = 0;
				vDir.normalize();

				float fRadian = NxMath::acos(vForward.dot(vDir));
				float fDegree = NxMath::radToDeg(fRadian);

				if ( fDegree > pData->_AttackScope )
				{
					if ( iter != _AttackableList.end() )
					{
						_AttackableList.erase(iter);

						if ( _BehaviorReport )
							_BehaviorReport->OnAttack(this, pAircraft, false);
					}
				}
				else
				{
					if ( iter == _AttackableList.end() )
					{
						_AttackableList.push_back(pAircraft);

						if ( _BehaviorReport )
							_BehaviorReport->OnAttack(this, pAircraft, true);
					}
				}
			}
		}
	}

	void PhysAircraft::ScanBlocks(float DeltaTime)
	{
		if (!GPhysGameSetting._GPL_Aircraft_Process_Blocks)
			return;
		
		if (NULL != _Header)
			return;
		
		GameObjectState* pState = _FSM.GetActiveState();
		if (!pState)
			return;

		if (EPT_Bomber == GetAircraftType() && Attack == pState->GetEnumValue())
			return;

		if (Landing != pState->GetEnumValue())
		{
			_AccumTime += DeltaTime;

			if (_AccumTime > _DetectInterval * 0.001f)
			{
				if (BlockTest())
				{
					if (!CHECK_STATE(_Flags, ExigencePull))
					{
						ENABLE_STATE(_Flags, VerticalMove);
						ENABLE_STATE(_Flags, ExigencePull);
					}
				}
				else
				{
					if (CHECK_STATE(_Flags, ExigencePull))
						DISABLE_STATE(_Flags, ExigencePull);
				}

				_AccumTime = 0;
			}
		}
	}

	bool PhysAircraft::BlockTest()
	{
		if (!_Scene)
			return false;

		if (!_ComponentData)
			return false;

		AircraftCompDesc* pCompDesc = DynamicCast(AircraftCompDesc, _ComponentData->_CompDesc);
		if (!pCompDesc)
			return false;

		NxRaycastHit HitInfo;
		NxU32 groups = MASK(GPL_SHAPE_GROUP_TERRAIN) | MASK(GPL_SHAPE_GROUP_STATICOBJECT);
		NxShape* HitShape = NULL;
		float Overlap = 0;
		float Range = CHECK_STATE(_Flags, ExigencePull) ? pCompDesc->_WarningDistance : pCompDesc->_WarningDistance * 2;
		//if ( NxMath::equals(Range, 0.f, 0.0001f) )
		//	gplDebugf(TEXT("PhysAircraft::BlockTest raycast maxDist warning"));

		//	水平方向检测
		NxVec3 Dir = _Forward;
		Dir.z = 0;
		if (Dir.normalize() > 0)
		{
			NxRay HRay(_WorldTransform.t, Dir);
			HRay.dir.normalize();
			HitShape = _Scene->_PhysScene->raycastClosestShape(HRay, NX_ALL_SHAPES, HitInfo, groups, Range, 0 );
			if ( HitShape != NULL )
				Overlap = NxMath::max(Overlap, pCompDesc->_WarningDistance - HitInfo.worldImpact.distance(_WorldTransform.t));
		}

		//	竖直方向检测
		NxRay VRay(_WorldTransform.t, -AXIS_VEC_Z);
		HitShape = _Scene->_PhysScene->raycastClosestShape(VRay, NX_ALL_SHAPES, HitInfo, groups, Range, 0 );
		if ( HitShape != NULL )
			Overlap = NxMath::max(Overlap, pCompDesc->_WarningDistance - HitInfo.worldImpact.distance(_WorldTransform.t));

		if ( HitShape != NULL )
		{
			_ExigencePullAcceleration = Overlap * 10;
			_DetectInterval = 10.f;//NxMath::max(_DetectInterval * 0.5f, 10.f);
			return true;
		}

		_ExigencePullAcceleration = 0;
		_VerticalSpeed = 0;
		_DetectInterval = NxMath::min(_DetectInterval * 2, GPhysGameSetting._GPL_Aircraft_Block_Detect_Interval);
		return false;
	}

	EPartType PhysAircraft::GetAircraftType()
	{
		AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
		if ( NULL == pData )
			return EPT_Invalid;

		return pData->_AircraftType;
	}

	FlightLayer PhysAircraft::GetFlightLayer()
	{
		float HLayerHeight = RouteAssist::GetSingletonRef().GetLayerHeight(Higher);
		float LLayerHeight = RouteAssist::GetSingletonRef().GetLayerHeight(ExtremeLow);

		float Height = GetGlobalHeight();
		if (NxMath::equals(Height, HLayerHeight, 0.1f) || Height > HLayerHeight)
			return Higher;
		
		if (NxMath::equals(Height, LLayerHeight, 0.1f) || Height < LLayerHeight)
			return ExtremeLow;
		
		return Lower;
	}

	void PhysAircraft::SetBaseShip(PhysGameObject* pOwner)
	{
		if ( !IsKind(PhysVehicle, pOwner) )
			return;

		SetOwner(pOwner);

		std::vector<Socket> vSockets;

		pOwner->_ComponentData->FindSocket(EST_Socket_StartPoint, vSockets);
		if ( !vSockets.empty() )
			_StartPoint = vSockets.front()._LocalPose;

		pOwner->_ComponentData->FindSocket(EST_Socket_LeavePoint, vSockets);
		if ( !vSockets.empty() )
			_LeavePoint = vSockets.front()._LocalPose;

		pOwner->_ComponentData->FindSocket(EST_Socket_CatchPoint, vSockets);
		if ( !vSockets.empty() )
			_CatchPoint = vSockets.front()._LocalPose;
	}

	PhysVehicle* PhysAircraft::GetBaseShip()
	{
		return DynamicCast(PhysVehicle, _Owner);
	}

	NxVec3 PhysAircraft::GetStartLocation()
	{
		if ( !_Owner )
			return ZERO_VEC;

		NxVec3 GlobalLocation;
		_Owner->GetGlobalPos().multiply(_StartPoint.t, GlobalLocation);

		return GlobalLocation;
	}

	NxVec3 PhysAircraft::GetLeaveLocation()
	{
		if ( !_Owner )
			return ZERO_VEC;

		NxVec3 GlobalLocation;
		_Owner->GetGlobalPos().multiply(_LeavePoint.t, GlobalLocation);

		return GlobalLocation;
	}

	NxVec3 PhysAircraft::GetCatchLocation()
	{
		if ( !_Owner )
			return ZERO_VEC;

		NxVec3 GlobalLocation;
		_Owner->GetGlobalPos().multiply(_CatchPoint.t, GlobalLocation);

		return GlobalLocation;
	}

	void PhysAircraft::SetSquadIndex(int index)
	{
		if ( _SquadIndex == index )
			return;

		_SquadIndex = index;
		
		//	长机处理
		if ( _SquadIndex == 0 )
		{
			_Header = NULL;

			RESET_STATE(_Flags);

#ifdef _CLIENT_RUNTIME
			AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
			if ( !pData )
				return;

			if (_LocalPlayer && GetAircraftType() == EPT_Fight)
			{
				UTriggerDesc Desc;
				Desc._InitGlobalPose.t = GetGlobalPosition();
				Desc._Extends = GPhysGameSetting._GPL_FlightLayer_CruiseHeight * G2PScale * 10;

				//	创建拦截范围碰撞体
				if (NULL == _InterceptTrigger)
				{
					//gplDebugf(TEXT("Create InterceptTrigger Radius=%f"), pData->_InterceptRange);
					Desc._Radius = pData->_InterceptRange;

					PhysTrigger* pTrigger = CreateTrigger(_Scene, Desc);
					_InterceptTrigger = DynamicCast(RangeTrigger, pTrigger);
					if (NULL != _InterceptTrigger)
					{
						if ( !_InterceptTrigger->Init() )
							DestroyPhysGameObject(_Scene, _InterceptTrigger);
						else
							_InterceptTrigger->SetOwner(this);
					}
				}

				//	创建追击范围碰撞体
				if (NULL == _ChaseTrigger)
				{
					//gplDebugf(TEXT("Create ChaseTrigger Radius=%f"), pData->_ChaseRange);
					Desc._Radius = pData->_ChaseRange;

					PhysTrigger* pTrigger = CreateTrigger(_Scene, Desc);
					_ChaseTrigger = DynamicCast(RangeTrigger, pTrigger);
					if (NULL != _ChaseTrigger)
					{
						if ( !_ChaseTrigger->Init() )
							DestroyPhysGameObject(_Scene, _ChaseTrigger);
						else
							_ChaseTrigger->SetOwner(this);
					}
				}
			}
#endif // _CLIENT_RUNTIME
		}
		else
		{
			PhysAircraft* pAircraft = GetAircraft((PhysVehicle*)_Owner, _SquadID, 0);
			SetHeader(pAircraft);
		}
	}

	void PhysAircraft::SetHeader(PhysAircraft* header)
	{
		if ( !header )
			return;

		_Header = header;
		_Header->RegisterWing(this);
	}

	void PhysAircraft::RegisterWing(PhysAircraft* wing)
	{
		if ( !wing )
			return;

		if (_Wings.empty())
		{
			_Wings.push_back(wing);
		}
		else
		{
			AircraftList::iterator iter = std::find(_Wings.begin(), _Wings.end(), wing);
			if ( iter == _Wings.end() )
				_Wings.push_back(wing);
		}
	}

	void PhysAircraft::UnregisterWing(PhysAircraft* wing, bool resort/* = true*/)
	{
		if (_Wings.empty())
			return;

		AircraftList::iterator iter = std::find(_Wings.begin(), _Wings.end(), wing);
		if ( iter != _Wings.end() )
			_Wings.erase(iter);

		if (!resort)
			return;

		int index = 1;
		for ( iter = _Wings.begin(); iter != _Wings.end(); iter++ )
		{
			PhysAircraft* pAircraft = *iter;
			if ( !pAircraft )
				continue;

			pAircraft->SetSquadIndex(index);
			index++;
		}
	}

	PhysAircraft* PhysAircraft::GetWing(UINT idx)
	{
		if ( idx < _Wings.size() )
			return _Wings[idx];

		return NULL;
	}

	bool PhysAircraft::IsAlive()
	{
		GameObjectState* State = _FSM.GetActiveState();

		if ( State && State->GetEnumValue() == Crash )
			return false;

		return true;
	}

	NxVec3 PhysAircraft::GetConvoluteCenter()
	{
		GameObjectState* pState = _FSM.GetActiveState();
		if ( !pState )
			return ZERO_VEC;

		if ( Circle != pState->GetEnumValue() )
			return ZERO_VEC;

		return ((AircraftStateCircle*)pState)->GetCenter();
	}

	NxVec3 PhysAircraft::GetGuardCenter()
	{
		if ( !_InterceptTrigger )
			return ZERO_VEC;

		NxActor* PhysActor = _InterceptTrigger->GetPhysActor();
		if ( !PhysActor )
			return ZERO_VEC;

		NxVec3 Center = PhysActor->getGlobalPosition();
		Center.z = GetGlobalPosition().z;
		return Center;
	}

	float PhysAircraft::GetMinRadius()
	{
		AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
		if ( !pData )
			return DEFAULT_RADIUS;

		return pData->_MinRadius; 
	}

	float PhysAircraft::GetConvoluteRadius()
	{
		float fBaseRadius = GetMinRadius();

		if ( !_Header )
			return fBaseRadius * 2;

		NxVec3 vOffset = GetFormationOffset();

		if ( _SquadIndex % 2 == 1 )
			return fBaseRadius * 2 + vOffset.magnitude() * NxMath::sin(60.f);
		else
			return fBaseRadius * 2 - vOffset.magnitude() * NxMath::sin(60.f);
	}

	float PhysAircraft::GetRadius(WiseType wise)
	{
		float fBaseRadius = GetMinRadius();

		if ( !_Header )
			return fBaseRadius;

		NxVec3 vOffset = GetFormationOffset();

		if ( CW == wise )
		{
			if ( _SquadIndex % 2 == 1 )
				return fBaseRadius + vOffset.magnitude() * NxMath::sin(60.f);
			else
				return fBaseRadius - vOffset.magnitude() * NxMath::sin(60.f);
		}
		else if ( CCW == wise )
		{
			if ( _SquadIndex % 2 == 1 )
				return fBaseRadius - vOffset.magnitude() * NxMath::sin(60.f);
			else
				return fBaseRadius + vOffset.magnitude() * NxMath::sin(60.f);
		}

		return fBaseRadius;
	}

	void PhysAircraft::SetSpeedFactor(float fFactor)
	{
		float fValideFactor = NxMath::min(fFactor, SPEED_FACTOR_UPPER_LIMIT);
		fValideFactor = NxMath::max(SPEED_FACTOR_LOWER_LIMIT, fValideFactor);
		_SpeedFactor = fValideFactor;
	}

	float PhysAircraft::GetSpeed()
	{
		AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
		if ( !pData )
			return _SpeedFactor * DEFAULT_SPEED;

		return _SpeedFactor * pData->_Speed;
	}

	NxVec3 PhysAircraft::GetVelocity()
	{
		AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
		if ( !pData )
			return GetForward() * DEFAULT_SPEED;

		return GetForward() * pData->_Speed;
	}

	ETrajectoryMethod PhysAircraft::GetTrajectoryMethod()
	{
		if ( !_ComponentData )
			return ETM_Parabola;

		AircraftCompDesc* pPhysCompDesc = DynamicCast(AircraftCompDesc, _ComponentData->_CompDesc);
		if ( !pPhysCompDesc )
			return ETM_Parabola;

		return pPhysCompDesc->_TrajectoryMethod;
	}

	void PhysAircraft::SetForward(const NxVec3& forward)	
	{
		if ( forward.equals(ZERO_VEC, 0.01f) )
			return;

		_Forward = forward;
		NxQuat Rot = Dir2Quat(_Forward);
		_WorldTransform.M.fromQuat(Rot);
	}

	NxVec3 PhysAircraft::GetForward()
	{
		return _Forward;
	}

	void PhysAircraft::ModifyPrecisionFactor(float Delta)
	{
		_PrecisionFactor += Delta;
		_PrecisionFactor = NxMath::max(_PrecisionFactor, GetPrecision());

		if ( _WorldTransform.t.z > 1.f )
		{
			float Range = _PrecisionFactor * _WorldTransform.t.z;
			float MinRange = GetPrecision() * RouteAssist::GetSingletonRef().GetLayerHeight(Higher);
			float MaxRange = 10000.f * RouteAssist::GetSingletonRef().GetLayerHeight(Higher);			//	10000.f for 1.0

			if ( Range < MinRange )
				_PrecisionFactor = MinRange / _WorldTransform.t.z;
			else if ( Range > MaxRange )
				_PrecisionFactor = MaxRange / _WorldTransform.t.z;
		}

		_PrecisionFactor = NxMath::clamp(_PrecisionFactor, 10000.f, GetPrecision());
		//gplDebugf(TEXT("ModifyPrecisionFactor::_PrecisionFactor[%f]"), _PrecisionFactor);
	}

	float PhysAircraft::GetFinalPrecision()
	{
		if ( NULL != _Header )
			return _Header->GetFinalPrecision();

		AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
		if ( !pData )
			return 0.f;

		return _PrecisionFactor * 0.0001f * pData->_InitPrecision * 0.0001f;
	}

	void PhysAircraft::OnBombTargetChange(const NxVec3& tar)
	{
		NxVec3 LastTarget = _UTargetPos * G2PScale;
		LastTarget.z = 0;
		NxVec3 NewTarget(tar.x, tar.y, 0);
		NxReal fDist = LastTarget.distance(NewTarget);
		NxReal fAffectRatio = GPhysGameSetting._GPL_Bomber_ReCollimate_PrecisionFactorA 
			* NxMath::pow(fDist, GPhysGameSetting._GPL_Bomber_ReCollimate_PrecisionFactorB);

		_PrecisionFactor *= 1 + fAffectRatio * 0.01f;

		_PrecisionFactor = NxMath::min(_PrecisionFactor, 10000.f);

		//gplDebugf(TEXT("Distance=%f PrecisionFactor=%f"), fDist, _PrecisionFactor);
	}

	void PhysAircraft::SetFlag(AircraftFlag flag, bool enable)
	{
		if (enable)
		{
			ENABLE_STATE(_Flags, flag);

			//gplDebugf(TEXT("PhysAircraft Open State[%d]"), flag);
		}
		else
		{
			DISABLE_STATE(_Flags, flag);

			//gplDebugf(TEXT("PhysAircraft Close State[%d]"), flag);
		}
	}

}	//	namespace GPL