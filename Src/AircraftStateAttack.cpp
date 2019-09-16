//------------------------------------------------------------------------
//	@brief	舰载机攻击状态
//	@author	chenpu
//	@date	2013-2-4
//------------------------------------------------------------------------
#include "PhysXSupport.h"
#include "AircraftFSM.h"
#include "AircraftStateAttack.h"
#include "PhysGameObjectAircraft.h"
#include "DirectionalCircle.h"

namespace GPL {

	bool AircraftStateAttack::OnEvent(GameObjectEvent& args)
	{
		return true;
	}

	void AircraftStateAttack::OnEnter(GameObjectEvent& args)
	{
		__super::OnEnter(args);

		if (!_Host)
			return;

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if (!pAircraft)
			return;

		AircraftEventAttack& e = (AircraftEventAttack&)args;

#ifdef _SERVER_RUNTIME
		PhysGameObject* pTargetObject = (PhysGameObject*)e._Userdata;
		pAircraft->Lock(pTargetObject);
#endif // _SERVER_RUNTIME

		pAircraft->SetFlag(VerticalMove, false);
		_AttackFinished = false;

		switch ( pAircraft->GetAircraftType() )
		{
		case EPT_Scout:
		case EPT_Fight:
			{
#ifdef _SERVER_RUNTIME
				if ( NULL != pTargetObject )
				{
					RouteAssist::GetSingletonRef().ObjectTrace(pAircraft, pTargetObject, _Step);
					_AccumTime = 0;
				}
				else
				{
					gplDebugf(TEXT("Leave State Without Target Object"));
					GameObjectEvent args;
					OnLeave(args);
				}
#endif // _SERVER_RUNTIME

#ifdef _CLIENT_RUNTIME
				if (e._Direction.equals(ZERO_VEC, 0.01f))
					RouteAssist::GetSingletonRef().RouteWithoutDirection(pAircraft, e._Target, _Step);
				else
					RouteAssist::GetSingletonRef().Route(pAircraft, e._Target, e._Direction, _Step);

				RouteAssist::GetSingletonRef().ExtendLastMotion(pAircraft, _Step);
#endif // _CLIENT_RUNTIME
			}
			break;

		case EPT_Bomber:
			{
				NxVec3 vStepInPoint = pAircraft->GetGlobalPosition();
				NxVec3 vAttackPoint = e._Target;

				NxVec3 vDirection = vAttackPoint - vStepInPoint;
				vDirection.z = 0;
				float AclinicMovement = vDirection.normalize();	//	水平位移

				float DiveHeight = vStepInPoint.z - vAttackPoint.z;
				//float DiveTime = NxMath::sqrt(2 * DiveHeight / 10);
				float DiveTime = RouteAssist::GetSingletonRef().CalcDiveDuration(
									DiveHeight, 
									pAircraft->GetDiveAcceleration(), 
									GPhysGameSetting._GPL_Aircraft_DiveAttack_LimitSpeed);

				float SpeedFactor = (AclinicMovement / DiveTime) / pAircraft->GetCruiseSpeed();

				NxVec3 vStepOutPoint = RouteAssist::GetSingletonRef().CalcStepOut(
					pAircraft, 
					Attack, 
					vAttackPoint, 
					vDirection);

				//	攻击路径
				vAttackPoint.z = vStepInPoint.z;
				OrbitParams AttackPhase(vStepInPoint, vAttackPoint);
				_Step.push_back(AttackPhase);

				//	长机的脱离路径
				vAttackPoint.z = vStepInPoint.z = RouteAssist::GetSingletonRef().GetLayerHeight(Lower);
				OrbitParams LeavePhase(vAttackPoint, vStepOutPoint);
				_Step.push_back(LeavePhase);

				//	进入俯冲，关闭精度修正
				pAircraft->SetFlag(Collimate, false);
				pAircraft->SetFlag(LockHeader, false);
				pAircraft->SetFlag(VerticalMove, true);
				pAircraft->SetFlag(ExigencePull, false);
				pAircraft->SetFlag(DiveAttack, true);

				pAircraft->SetSpeedFactor(SpeedFactor);
			}
			break;

		case EPT_TorpedoPlane:
			{
				NxVec3 vEntryPoint = pAircraft->GetGlobalPosition();
				NxVec3 vStepOutPoint = RouteAssist::GetSingletonRef().CalcStepOut(pAircraft, Attack, vEntryPoint, e._Direction);

				OrbitParams LeavePhase(vEntryPoint, vStepOutPoint);
				_Step.push_back(LeavePhase);

				//	立即投弹
				PostMotion(pAircraft);
			}
			break;
		}

		BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
		if ( pBehaviorReport )
			pBehaviorReport->OnInnerCommand(pAircraft, Attack, pAircraft->_UTargetPos, pAircraft->_UTargetDir);
	}

	void AircraftStateAttack::OnLeave(GameObjectEvent& args)
	{
		_Step.clear();

		__super::OnLeave(args);

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if (NULL != pAircraft)
		{
			pAircraft->SetSpeedFactor(1.f);
			pAircraft->SetFlag(VerticalMove, false);
			pAircraft->SetFlag(DiveAttack, false);
			pAircraft->SetFlag(LockHeader, true);
			pAircraft->NonCommandAction();
			pAircraft->Lock(NULL);

			if (EPT_Bomber == pAircraft->GetAircraftType() && !_AttackFinished)
			{
				UINT Num = pAircraft->GetWingNum();
				for (UINT idx = 0; idx < Num; idx++)
				{
					PhysAircraft* pWing = pAircraft->GetWing(idx);
					if (pWing)
					{
						pWing->SetSpeedFactor(1.f);
						pWing->SetFlag(LockHeader, true);
					}
				}
			}
		}
	}

	void AircraftStateAttack::Update(float DeltaTime)
	{
		__super::Update(DeltaTime);

		if ( !_Host )
			return;

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if ( !pAircraft )
			return;

		switch (pAircraft->GetAircraftType())
		{
		case EPT_Fight:
		case EPT_Scout:
			{
#ifdef _SERVER_RUNTIME
				PhysGameObject* pTargetObject = pAircraft->GetLockTarget();
				if ( NULL != pTargetObject )
				{
					//	反应时间触发 路径再规划
					if ( _AccumTime > pAircraft->GetEchoInterval() )
					{
						RouteAssist::GetSingletonRef().ObjectTrace(pAircraft, pTargetObject, _Step);
						_AccumTime = 0;

						//gplDebugf(TEXT("反应时间触发 路径再规划"));
						BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
						if ( pBehaviorReport )
							pBehaviorReport->OnInnerCommand(pAircraft, Attack, pAircraft->_UTargetPos, pAircraft->_UTargetDir);
					}
				}
				else 
				{
					gplDebugf(TEXT("Leave State Without Target Object"));
					GameObjectEvent args;
					OnLeave(args);
				}
#endif // _SERVER_RUNTIME
			}
			break;

		case EPT_Bomber:
			{
				if (!_AttackFinished)
				{
//#ifdef _SERVER_RUNTIME
//					if (NxMath::abs(pAircraft->GetVerticalSpeed()) > pAircraft->GetCruiseSpeed() * 0.1f)
//					{
//						PhysAircraft* Next = PhysAircraft::GetAircraft(
//							(PhysVehicle*)pAircraft->GetOwner(), 
//							pAircraft->GetSquadID(), 
//							pAircraft->GetSquadIndex() + 1);
//
//						if (NULL != Next)
//						{
//							GameObjectState* CurrentState = Next->GetCurrentState();
//							if (CurrentState != NULL && CurrentState->GetEnumValue() != Attack)
//								Next->ChangeAction(Attack, Next->_UTargetPos * G2PScale, ZERO_VEC);
//						}
//					}
//#endif // _SERVER_RUNTIME
				}
				else if (!_Step.empty())
				{
					PhysAircraft* pHeader = pAircraft->GetHeader();
					if (NULL != pHeader)
					{
						NxVec3 HeaderPos = pHeader->GetGlobalPosition();
						NxQuat HeaderRot = pHeader->GetGlobalOritationQuat();
						NxVec3 Pos = HeaderPos + pAircraft->GetFormationOffset(pHeader->GetForward());
						Pos = VectorInterp(pAircraft->GetGlobalPosition(), Pos, DeltaTime, GPhysGameSetting._PhysAircraft_PositionInterp_Speed);

						NxQuat Rot;
						Rot.slerp(GPhysGameSetting._PhysAircraft_RotationInterp_Speed, pAircraft->GetGlobalOritationQuat(), HeaderRot);

						NxVec3 Dir = Rot.rot(AXIS_VEC_X);
						Dir.z = NxMath::max(Dir.z, 0.f);
						Dir.normalize();

						pAircraft->SetGlobalPosition(Pos);

						OrbitParams& param = _Step.front();
						param._Positon = Pos;
						param._Target = Pos + Dir * GPhysGameSetting._GPL_MAX_HALF_WORLD;
						param._PassedTime = 0;
					}
				}
			}
			break;
		}
	}

	void AircraftStateAttack::RefreshParam(OrbitParams& params, PhysAircraft* pAircraft)
	{
		if (!pAircraft)
			return;

		if (!pAircraft->CheckFlag(VerticalMove))
			return;

		float ZFix = pAircraft->GetGlobalHeight();
		 
		if ( params._Wise == None )
		{
			params._Positon.z = ZFix;
			params._Target.z = ZFix;
		}
		else
		{
			params._Positon.z = ZFix;
			params._Center.z = ZFix;
		}

		if (pAircraft->CheckFlag(ExigencePull))
		{
			if (ZFix < RouteAssist::GetSingletonRef().GetLayerHeight(Lower))
			{
				pAircraft->SetExigencePullAcceleration(pAircraft->GetPullAcceleration());
			}
			else
			{
				pAircraft->SetFlag(ExigencePull, false);
				pAircraft->SetFlag(Ascend, true);
			}
		}

		if (_AttackFinished && !pAircraft->CheckFlag(LockHeader))
		{
			PhysAircraft* Header = pAircraft->GetHeader();
			if (Header != NULL && Header->GetFlightLayer() > Lower)
			{
				NxVec3 HeaderPos2D = Header->GetGlobalPosition();
				NxVec3 Pos2D = pAircraft->GetGlobalPosition();
				HeaderPos2D.z = 0;
				Pos2D.z = 0;
				float Dis = HeaderPos2D.distance(Pos2D);
				float Standard = pAircraft->GetFormationOffset().magnitude();
				if ( Dis <= Standard )
					pAircraft->SetFlag(LockHeader, true);
			}
		}
	}

	void AircraftStateAttack::PostMotion(PhysAircraft* pAircraft)
	{
		if ( !pAircraft )
			return;

		if (_Step.empty())
		{
			GameObjectEvent args;
			OnLeave(args);
		}
		else if (!_AttackFinished)
		{
			//	触发投弹相关事件
			switch (pAircraft->GetAircraftType())
			{
			case EPT_Bomber:
				{
					pAircraft->SetSpeedFactor(1.f);
					pAircraft->SetFlag(DiveAttack, false);
					pAircraft->SetFlag(ExigencePull, true);
				}
			case EPT_TorpedoPlane:
				{
					BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
					if ( pBehaviorReport )
						pBehaviorReport->OnAction(pAircraft, *this);
				}
				break;
			}

			_AttackFinished = true;
		}
	}

}	// namespace GPL