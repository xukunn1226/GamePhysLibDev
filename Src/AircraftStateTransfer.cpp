//------------------------------------------------------------------------
//	@brief	���ػ��ƶ�״̬
//	@author	chenpu
//	@date	2013-2-4
//------------------------------------------------------------------------
#include "PhysXSupport.h"
#include "AircraftFSM.h"
#include "AircraftStateTransfer.h"
#include "DirectionalCircle.h"

#define LANDING_ALLOW	(400)

namespace GPL {

	bool AircraftStateTransfer::OnEvent(GameObjectEvent& args)
	{
		return true;
	}

	void AircraftStateTransfer::OnEnter(GameObjectEvent& args)
	{
		__super::OnEnter(args);

		if ( !_Host )
			return;

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if ( !pAircraft )
			return;

		AircraftEventTransfer& e = (AircraftEventTransfer&)args;
		NxVec3 Target(e._Target.x, e._Target.y, pAircraft->GetGlobalPosition().z);
		NxVec3 Direction = e._Direction;

		pAircraft->Lock(e._TargetObj);

		if ( NULL != e._TargetObj )
		{
			//	����׷��
			RouteAssist::GetSingletonRef().ObjectTrace(pAircraft, e._TargetObj, _Step);
		}
		else if ( Target.equals(pAircraft->GetGlobalPosition(), 0.1f) )
		{
			//	�Ѵ�Ŀ��㣬ֱ�ӽ���
			OnLeave(args);
		}
		else if ( Direction.equals(ZERO_VEC, 0.01f) )
		{
			//	���Է����·���滮
			RouteAssist::GetSingletonRef().RouteWithoutDirection(pAircraft, Target, _Step);
#ifdef _CLIENT_RUNTIME
			RouteAssist::GetSingletonRef().ExtendLastMotion(pAircraft, _Step);
#endif // _CLIENT_RUNTIME
		}
		else
		{
			//	��ע�����·���滮
			RouteAssist::GetSingletonRef().Route(pAircraft, Target, Direction, _Step);
#ifdef _CLIENT_RUNTIME
			RouteAssist::GetSingletonRef().ExtendLastMotion(pAircraft, _Step);
#endif // _CLIENT_RUNTIME
		}

		BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
		if ( pBehaviorReport )
			pBehaviorReport->OnInnerCommand(pAircraft, Transfer, pAircraft->_UTargetPos, pAircraft->_UTargetDir);
	}

	void AircraftStateTransfer::OnLeave(GameObjectEvent& args)
	{
		__super::OnLeave(args);

		if ( !_Host )
			return;

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if ( !pAircraft )
			return;

		pAircraft->Lock(NULL);
	}

	void AircraftStateTransfer::Update(float DeltaTime)
	{
		__super::Update(DeltaTime);

		if ( !_Host )
			return;

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if (!pAircraft)
			return;

		if (!_Step.empty())
		{
			float fFlightHeight = pAircraft->GetGlobalHeight();

			OrbitParams& param = _Step.front();
			if (param._Wise != None)
			{
				param._Center.z = fFlightHeight;
				param._Positon.z = fFlightHeight;
			}
			else
			{
				param._Positon.z = fFlightHeight;
				param._Target.z = fFlightHeight;
			}
		}
		else if (NULL == pAircraft->GetLockTarget())
		{
			GameObjectEvent args;
			OnLeave(args);
		}

		PhysGameObject* pTarget = pAircraft->GetLockTarget();
		if ( NULL != pTarget )
		{
			//	��Ӧʱ�䴥�� ·���ٹ滮
			if ( _AccumTime > pAircraft->GetEchoInterval() )
			{
				if ( pAircraft->GetAircraftType() == EPT_Scout )
				{
					NxVec3 MyPos = pAircraft->GetGlobalPosition();
					NxVec3 TargetPos = pTarget->GetGlobalPosition();
					float SensorRange = pAircraft->GetSensorRange();

					if ( TargetPos.distanceSquared(MyPos) < SensorRange * SensorRange * 0.8f )
					{
						//	����������
						NxVec3 Dir = MyPos - TargetPos;
						Dir.z = 0;
						Dir.normalize();

						NxVec3 Target = MyPos + Dir * 100;

						RouteAssist::GetSingletonRef().RouteWithoutDirection(pAircraft, Target, _Step);

						BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
						if ( pBehaviorReport )
							pBehaviorReport->OnInnerCommand(pAircraft, Transfer, pAircraft->_UTargetPos, pAircraft->_UTargetDir);

						_AccumTime = 0;

						return;
					}
				}

				RouteAssist::GetSingletonRef().ObjectTrace(pAircraft, pTarget, _Step);
				_AccumTime = 0;

				BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
				if ( pBehaviorReport )
					pBehaviorReport->OnInnerCommand(pAircraft, Transfer, pAircraft->_UTargetPos, pAircraft->_UTargetDir);
			}
		}
#ifdef _SERVER_RUNTIME
		else if ( Landing == pAircraft->GetIntent() )
		{
			NxVec3 _EndPoint = pAircraft->GetStartLocation();		//	�յ�
			NxVec3 _CatchPoint = pAircraft->GetCatchLocation();		//	�Ž���
			NxVec3 _Dir = _EndPoint - _CatchPoint;
			_Dir.z = 0;
			_Dir.normalize();

			NxVec3 _TransPoint = RouteAssist::GetSingletonRef().CalcStepIn(
				pAircraft, Landing, Higher, _CatchPoint, _Dir);

			NxVec3 PrevTarget = pAircraft->_UTargetPos * G2PScale;
			PrevTarget.z = _TransPoint.z;
			NxVec3 PrevDir = pAircraft->_UTargetDir;
			PrevDir.z = 0;
			PrevDir.normalize();

			//	Ŀ��λ��δ�������
			if (PrevTarget.equals(_TransPoint, 10.f) && PrevDir.equals(_Dir, 0.01f))
			{
				PhysVehicle* pBaseShip = DynamicCast(PhysVehicle, pAircraft->GetOwner());
				if (NULL != pBaseShip && pBaseShip->IsSubmarine() && pBaseShip->_CurLevel != 0)
				{
					if ( _AccumTime > pAircraft->GetEchoInterval() )
					{
						RouteAssist::GetSingletonRef().Route(pAircraft, _TransPoint, _Dir, _Step);
						_AccumTime = 0;

						BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
						if ( pBehaviorReport )
							pBehaviorReport->OnInnerCommand(pAircraft, Transfer, pAircraft->_UTargetPos, pAircraft->_UTargetDir);
					}
				}
				return;
			}

			if ( _TransPoint.distanceSquared(pAircraft->GetGlobalPosition()) < LANDING_ALLOW )
			{
				BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
				if (NULL != pBehaviorReport && pBehaviorReport->IsLandingPermitted(pAircraft))
				{
					if ( _Dir.sameDirection(pAircraft->GetForward()) )
					{
						GameObjectEvent args;
						OnLeave(args);
						return;
					}
				}
			}

			//	���������ж�̬���㽵����ʼλ��
			if ( _AccumTime > pAircraft->GetEchoInterval() )
			{
				RouteAssist::GetSingletonRef().Route(pAircraft, _TransPoint, _Dir, _Step);
				_AccumTime = 0;

				BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
				if ( pBehaviorReport )
					pBehaviorReport->OnInnerCommand(pAircraft, Transfer, pAircraft->_UTargetPos, pAircraft->_UTargetDir);
			}
		}
#endif // _SERVER_RUNTIME
	}

	void AircraftStateTransfer::RefreshParam(OrbitParams& params, PhysAircraft* pAircraft)
	{
		//if (!pAircraft)
		//	return;

		//if (!pAircraft->CheckFlag(VerticalMove))
		//	return;

		//float ZFix = pAircraft->GetGlobalHeight();

		//if ( params._Wise == None )
		//{
		//	params._Positon.z = ZFix;
		//	params._Target.z = ZFix;
		//}
		//else
		//{
		//	params._Positon.z = ZFix;
		//	params._Center.z = ZFix;
		//}
	}

	void AircraftStateTransfer::PostMotion(PhysAircraft* pAircraft)
	{
		if ( !pAircraft )
			return;		

		if ( _Step.empty() )
		{
#if 0
			if ( pAircraft->GetAircraftType() == EPT_TorpedoPlane && pAircraft->GetIntent() == Attack )
			{
				if ( pAircraft->GetFlightLayer() != ExtremeLow )
				{
					//	���׻����ƶ�������Ŀ���ʱ��û���½�����Ͷ���߶���׷������һ��
					float fRadius = pAircraft->GetMinRadius();
					NxVec3 Dir = pAircraft->GetForward().cross(-AXIS_VEC_Z);
					Dir.z = 0;
					Dir.normalize();
					NxVec3 Center = pAircraft->GetGlobalPosition() + Dir * fRadius;
					OrbitParams circle(CW, pAircraft->GetGlobalPosition(), Center, fRadius);
					_Step.push_back(circle);
				}
				else
				{
					pAircraft->OnActionFinished();
				}
			}
			else
			{
				pAircraft->OnActionFinished();
			}
#else

#ifdef _SERVER_RUNTIME
			if (Landing != pAircraft->GetIntent())
			{
				pAircraft->OnActionFinished();
			}
			else
			{
				BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
				if (!pBehaviorReport)
				{
					pAircraft->OnActionFinished();
				}
				else if (pBehaviorReport->IsLandingPermitted(pAircraft))
				{
					pAircraft->OnActionFinished();
				}
				else
				{
					//	���ϲ��߼��޷�����ʱ�����½���·���滮��β��ĸ��
					NxVec3 _EndPoint = pAircraft->GetStartLocation();		//	�յ�
					NxVec3 _CatchPoint = pAircraft->GetCatchLocation();		//	�Ž���
					NxVec3 _Dir = _EndPoint - _CatchPoint;
					_Dir.z = 0;
					_Dir.normalize();

					NxVec3 _TransPoint = RouteAssist::GetSingletonRef().CalcStepIn(
						pAircraft, Landing, Higher, _CatchPoint, _Dir);

					if (pAircraft->GetGlobalPosition().equals(_TransPoint, 0.01f))
						_TransPoint -= _Dir * 2;
					RouteAssist::GetSingletonRef().Route(pAircraft, _TransPoint, _Dir, _Step);
					RouteAssist::GetSingletonRef().ExtendLastMotion(pAircraft, _Step);

					BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
					if ( pBehaviorReport )
						pBehaviorReport->OnInnerCommand(pAircraft, Transfer, pAircraft->_UTargetPos, pAircraft->_UTargetDir);
				}				
			}
#endif // _SERVER_RUNTIME
#endif
		}
		else
		{
			BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
			if ( pBehaviorReport )
				pBehaviorReport->OnAction(pAircraft, *this);
		}
	}

}	// namespace GPL