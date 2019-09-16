//------------------------------------------------------------------------
//	@brief	舰载机状态机
//	@author	chenpu
//	@date	2013-2-3
//------------------------------------------------------------------------
#include "PhysXSupport.h"
#include "AircraftFSM.h"
#include "PhysGameObjectAircraft.h"

namespace GPL {

	bool AircraftFSM::Init(FSMActor* owner)
	{
		if ( !GameObjectFSM::Init(owner) )
			return false;

		_PrevState = 0xFF;

		_StateReady.PostBinding(this);
		_StateTakeOff.PostBinding(this);
		_StateLanding.PostBinding(this);
		_StateTransfer.PostBinding(this);
		_StateCircle.PostBinding(this);
		_StateAttack.PostBinding(this);
		_StateCrash.PostBinding(this);

		SetDefaultState(&_StateCircle);

		return true;
	}

	void AircraftFSM::Update(float DeltaTime)
	{
		GameObjectFSM::Update(DeltaTime);

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Actor);
		if (!pAircraft)
			return;

		if ( NULL == _ActiveState )
		{
			if ( !PopAction() )
			{
				pAircraft->NonCommandAction();
			}
		}

		if ( _ActiveState )
		{
			OrbitParams param;
			if ( ((AircraftState*)_ActiveState)->GetMotionParam(0, param) )
			{
				switch ( _ActiveState->GetEnumValue() )
				{
				case TakeOff:
				case Attack:
					return;

				default:
					if ( pAircraft->CheckFlag(Collimate) )
					{
						//	投弹精度计算
						if ( param._Wise != None )
						{
							//	转向修正
							pAircraft->ModifyPrecisionFactor(DeltaTime * pAircraft->GetTurningAffect());
						}
						else
						{
							//	直线运动修正
							pAircraft->ModifyPrecisionFactor(-DeltaTime * pAircraft->GetCollimation());
						}
					}
					break;
				}
			}
		}
	}

	void AircraftFSM::Coordinate(const NxVec3& Pos, const NxVec3& Dir)
	{
		if ( _ActiveState )
			((AircraftState*)_ActiveState)->Coordinate(Pos, Dir);

		ActionQueue::iterator iter;
		for ( iter = _ActionQueue.begin(); iter != _ActionQueue.end(); iter++ )
		{
			if ( Circle == iter->_action )
			{
				iter->_tar.z = Pos.z;
			}
			else if ( Transfer == iter->_action )
			{
				ActionData& data = *iter;
				ActionQueue::iterator next = iter;
				next++;
				if ( next != _ActionQueue.end() )
				{
					if ( Attack != next->_action && Landing != next->_action )
						data._tar.z = Pos.z;
				}
			}
		}
	}

	void AircraftFSM::ClearActions()
	{
		_ActionQueue.clear();
	}

	void AircraftFSM::PushAction(AircraftAction action, const NxVec3& tar, const NxVec3& dir, PhysGameObject* obj)
	{		
		ActionData data;
		data._action = action;
		data._tar = tar;
		data._dir = dir;
		data._obj = obj;

		_ActionQueue.push_back(data);
	}

	bool AircraftFSM::PopAction()
	{
		if ( _ActionQueue.empty() )
			return false;

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Actor);
		if ( !pAircraft )
			return false;

		ActionQueue::iterator iter = _ActionQueue.begin();
		pAircraft->_UStartPos = pAircraft->UGetGlobalPosition();
		pAircraft->_UStartDir = pAircraft->GetForward();
		pAircraft->_UTargetPos = iter->_tar * P2GScale;
		pAircraft->_UTargetDir = iter->_dir;
		pAircraft->_UTargetDir.z = 0;
		pAircraft->_UTargetDir.normalize();

		bool bExecutable = true;
		switch ( iter->_action )
		{
		case Ready:
			{
				AircraftEventReady eventArgs;
				eventArgs._State = &_StateReady;

				bExecutable = FlushEvent(eventArgs);
			}
			break;

		case Crash:
			if (NULL != iter->_obj)
				bExecutable = GotoCrash(false);		//	燃料耗尽
			else 
				bExecutable = GotoCrash(true);		//	被击坠
			break;

		case TakeOff:
			bExecutable = GotoTakeoff();
			break;

		case Landing:
			{
				bExecutable = GotoLanding(
#ifdef _CLIENT_RUNTIME
					iter->_tar, 
					iter->_dir
#endif // _CLIENT_RUNTIME
					);
			}
			break;

		case Circle:
			bExecutable = GotoCircle(iter->_tar);
			break;

		case Transfer:
			{
				if ( !iter->_obj )
					bExecutable = GotoTransfer(iter->_tar, iter->_dir);
				else
					bExecutable = GotoTransfer(iter->_obj);
			}
			break;

		case Attack:
			{
				if ( NULL != iter->_obj )
					bExecutable = GotoAttack(iter->_obj);
				else
				{
#ifdef _SERVER_RUNTIME
					if ( pAircraft->GetAircraftType() != EPT_Bomber )
						bExecutable = GotoAttack(iter->_tar, iter->_dir);
					else
					{
						float Height = pAircraft->GetGlobalHeight();
						float Range = Height * pAircraft->GetFinalPrecision();
						float Scale = NxMath::rand(0.f, 1.f);
						float Angle = NxMath::rand(0.f, 2 * NxPi);

						NxVec3 Target = CalcDiveTarget(
							iter->_tar, 
							pAircraft->GetForward(), 
							Range, 
							Range * pAircraft->GetDiffuseEccentricity(), 
							Scale, 
							Angle);

						NxVec3 Dir = Target - pAircraft->GetGlobalPosition();
						Dir.z = 0;
						Dir.normalize();
						if ( Dir.equals(ZERO_VEC, 0.01f) )
							Dir = pAircraft->GetForward();

						pAircraft->_UTargetPos = Target * P2GScale;
						pAircraft->_UTargetDir = Dir;

						bExecutable = GotoAttack(Target, Dir);
					}
#endif // _SERVER_RUNTIME

#ifdef _CLIENT_RUNTIME
					bExecutable = GotoAttack(iter->_tar, iter->_dir);
#endif // _CLIENT_RUNTIME
				}
			}
			break;
		}

		if ( bExecutable )
		{
#ifdef _SERVER_RUNTIME
			//	长机状态切换成功强制僚机执行
			if ( NULL == pAircraft->GetHeader() && NULL != _ActiveState )
			{
				UINT Num = pAircraft->GetWingNum();

				switch ( _ActiveState->GetEnumValue() )
				{
				case Landing:
					{
						for ( UINT idx = 0; idx < Num; idx++ )
						{
							PhysAircraft* Wing = pAircraft->GetWing(idx);
							if ( !Wing )
								continue;

							GameObjectState* pState = Wing->GetCurrentState();
							if ( pState && pState->GetEnumValue() == Ready )
								continue;

							Wing->SetGlobalPosition(Wing->GetFormationGlobalPosition());
							Wing->SetForward(pAircraft->GetForward());
							Wing->ChangeAction(Landing, ZERO_VEC, ZERO_VEC);
						}
					}
					break;

				case Attack:
					{
						if ( pAircraft->GetAircraftType() == EPT_TorpedoPlane )
						{
							for ( UINT idx = 0; idx < Num; idx++ )
							{
								PhysAircraft* Wing = pAircraft->GetWing(idx);
								if ( !Wing )
									continue;

								Wing->SetGlobalPosition(Wing->GetFormationGlobalPosition());
								Wing->SetForward(pAircraft->GetForward());
								Wing->ChangeAction(
									Attack, 
									pAircraft->_UTargetPos * G2PScale + Wing->GetFormationOffset(pAircraft->_UTargetDir), 
									pAircraft->_UTargetDir
									);
							}
						}
						else if ( pAircraft->GetAircraftType() == EPT_Bomber )
						{
							float fDelay = 0.f;
							for ( UINT idx = 0; idx < Num; idx++ )
							{
								PhysAircraft* Wing = pAircraft->GetWing(idx);
								if (!Wing)
									continue;

								NxVec3 Pos = Wing->GetGlobalPosition();
								NxVec3 Dir = Wing->GetForward();
								fDelay += NxMath::rand(
									GPhysGameSetting._GPL_Aircraft_DiveAttack_MinInterval,
									GPhysGameSetting._GPL_Aircraft_DiveAttack_MaxInterval);
								NxVec3 MoveTarget = Pos + Dir * Wing->GetCruiseSpeed() * 
									GPhysGameSetting._GPL_Aircraft_DiveAttack_SpeedFactor * 
									fDelay;

								Wing->ResetActionQueueInFSM();
								Wing->PushActionToFSM(Transfer, MoveTarget, ZERO_VEC, NULL);
								Wing->PushActionToFSM(Attack, iter->_tar, ZERO_VEC, NULL);
								Wing->PopActionInFSM();
								Wing->SetFlag(LockHeader, false);
								Wing->SetSpeedFactor(GPhysGameSetting._GPL_Aircraft_DiveAttack_SpeedFactor);
								//Wing->ChangeAction(Transfer, Wing->GetGlobalPosition() + Wing->GetForward() * 1024, Wing->GetForward());
								//Wing->SetFlag(LockHeader, false);
								//Wing->SetSpeedFactor(0.2f);
								//Wing->RestoreTargetPos(iter->_tar);
							}
						}
					}
					break;

				case Transfer:
					{
						if (pAircraft->GetAircraftType() == EPT_TorpedoPlane)
						{
							if (pAircraft->GetIntent() != Attack)
							{
								if (pAircraft->GetFlightLayer() != Higher)
								{
									pAircraft->OnCommand(_LayerChange, ZERO_VEC, AXIS_VEC_Z);
									BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
									if ( pBehaviorReport )
										pBehaviorReport->OnInnerCommand(pAircraft, ~Transfer, ZERO_VEC, AXIS_VEC_Z);
								}
							}							
							else
							{
								pAircraft->OnCommand(_LayerChange, ZERO_VEC, -AXIS_VEC_Z);
								BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
								if ( pBehaviorReport )
									pBehaviorReport->OnInnerCommand(pAircraft, ~Transfer, ZERO_VEC, -AXIS_VEC_Z);
							}
						}
						else if (pAircraft->GetAircraftType() == EPT_Bomber)
						{
							if (pAircraft->GetFlightLayer() != Higher)
							{
								pAircraft->OnCommand(_LayerChange, ZERO_VEC, AXIS_VEC_Z);
								BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
								if ( pBehaviorReport )
									pBehaviorReport->OnInnerCommand(pAircraft, ~Transfer, ZERO_VEC, AXIS_VEC_Z);
							}
						}
					}
				case Circle:
					{
						for ( UINT idx = 0; idx < Num; idx++ )
						{
							PhysAircraft* Wing = pAircraft->GetWing(idx);
							if ( !Wing )
								continue;

							Wing->SetGlobalPosition(Wing->GetFormationGlobalPosition());
							Wing->SetForward(pAircraft->GetForward());
							Wing->ChangeAction(
								(AircraftAction)_ActiveState->GetEnumValue(), 
								pAircraft->_UTargetPos * G2PScale, 
								pAircraft->_UTargetDir);
						}
					}
					break;
				}
			}
#endif // _SERVER_RUNTIME

			_ActionQueue.pop_front();
		}

		_PrevState = (NULL != _ActiveState) ? _ActiveState->GetEnumValue() : 0xFF;

		return true;
	}

	ActionData* AircraftFSM::GetPendingAction(UINT idx)
	{
		if ( idx < _ActionQueue.size() )
		{
			ActionQueue::iterator iter = _ActionQueue.begin();
			for ( UINT i = 0; i < idx; i++ )
				iter++;

			return &(*iter);
		}

		return NULL;
	}

	AircraftAction AircraftFSM::GetLastAction()
	{
		if ( _ActionQueue.empty() )
			return ActionNum;

		return _ActionQueue.back()._action;
	}

	void AircraftFSM::DropLastAction()
	{
		if ( _ActionQueue.empty() )
			return;

		_ActionQueue.pop_back();
	}

	UINT AircraftFSM::GetPendingActionNum()
	{
		return (UINT)_ActionQueue.size();
	}

	void AircraftFSM::CalcPendingMoveDir()
	{
		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Actor);
		if ( !pAircraft )
			return;

		if ( _ActionQueue.size() > 1 )
		{
			ActionQueue::iterator iter = _ActionQueue.begin();
			for ( UINT idx = 0; idx < _ActionQueue.size() - 2; idx++ )
			{
				iter++;
			}

			//	将倒数第二个移动任务设置为有向移动
			if ( iter != _ActionQueue.end() )
			{
				NxVec3 Dir = _ActionQueue.back()._tar - iter->_tar;
				Dir.z = 0;
				Dir.normalize();

				//	如果是完全重合的点就不搞了
				if ( Dir.magnitudeSquared() > 0 )
					iter->_dir = Dir;
			}
		}
		else if ( !_ActionQueue.empty() )
		{
			//	当前移动状态，重新规划移动路径使方向匹配
			if ( _ActiveState && _ActiveState->GetEnumValue() == Transfer )
			{
				NxVec3 Target = pAircraft->_UTargetPos * G2PScale;
				NxVec3 Dir = _ActionQueue.back()._tar - Target;
				Dir.z = 0;
				Dir.normalize();
				if ( Dir.magnitudeSquared() > 0 )
				{
					pAircraft->_UTargetDir = Dir;
					GotoTransfer(Target, Dir);
				}
			}
		}
	}

	bool AircraftFSM::GotoTakeoff()
	{
		AircraftEventTakeOff eventArgs;
		eventArgs._State = &_StateTakeOff;

		return FlushEvent(eventArgs);
	}

	bool AircraftFSM::GotoCircle()
	{
		AircraftEventCircle eventArgs;
		eventArgs._State = &_StateCircle;
		eventArgs._Center = _Actor->GetGlobalPosition();

		return FlushEvent(eventArgs);
	}

	bool AircraftFSM::GotoCircle(const NxVec3& center)
	{
		AircraftEventCircle eventArgs;
		eventArgs._State = &_StateCircle;
		eventArgs._Center = center;
		eventArgs._Center.z = _Actor->GetGlobalPosition().z;

		return FlushEvent(eventArgs);
	}

	bool AircraftFSM::GotoTransfer(const NxVec3& pos, const NxVec3& dir/* = ZERO_VEC*/)
	{
		AircraftEventTransfer eventArgs;
		eventArgs._State = &_StateTransfer;
		eventArgs._TargetObj = NULL;
		eventArgs._Target = pos;
		eventArgs._Target.z = _Actor->GetGlobalPosition().z;
		eventArgs._Direction = dir;

		return FlushEvent(eventArgs);
	}

	bool AircraftFSM::GotoTransfer(PhysGameObject* obj)
	{
		if ( NULL == obj )
			return false;

		AircraftEventTransfer eventArgs;
		eventArgs._State = &_StateTransfer;
		eventArgs._TargetObj = obj;
		eventArgs._Target = ZERO_VEC;
		eventArgs._Direction = ZERO_VEC;
		
		return FlushEvent(eventArgs);
	}

	bool AircraftFSM::GotoLanding(const NxVec3& pos/* = ZERO_VEC*/, const NxVec3& dir/* = ZERO_VEC*/)
	{
		AircraftEventLanding eventArgs;
		eventArgs._State = &_StateLanding;
		eventArgs._Target = pos;
		eventArgs._Direction = dir;

		return FlushEvent(eventArgs);
	}

	bool AircraftFSM::GotoAttack(const NxVec3& pos, const NxVec3& dir/* = ZERO_VEC*/)
	{
		AircraftEventAttack eventArgs;
		eventArgs._State = &_StateAttack;
		eventArgs._Target = pos;
		eventArgs._Direction = dir;
		eventArgs._Userdata = NULL;

		return FlushEvent(eventArgs);
	}

	bool AircraftFSM::GotoAttack(PhysGameObject* pObj)
	{
		AircraftEventAttack eventArgs;
		eventArgs._State = &_StateAttack;
		eventArgs._Userdata = pObj;

		return FlushEvent(eventArgs);
	}

	bool AircraftFSM::GotoCrash(bool bShoot)
	{
		AircraftEventCrash eventArgs;
		eventArgs._State = &_StateCrash;
		eventArgs._Shoot = bShoot;

		return FlushEvent(eventArgs);
	}
}