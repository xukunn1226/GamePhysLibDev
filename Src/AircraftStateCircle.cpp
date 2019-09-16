//------------------------------------------------------------------------
//	@brief	舰载机盘旋状态
//	@author	chenpu
//	@date	2013-2-4
//------------------------------------------------------------------------
#include "PhysXSupport.h"
#include "AircraftFSM.h"
#include "AircraftStateCircle.h"
#include "DirectionalCircle.h"

namespace GPL {

	bool AircraftStateCircle::OnEvent(GameObjectEvent& args)
	{
		return true;
	}

	void AircraftStateCircle::OnEnter(GameObjectEvent& args)
	{
		__super::OnEnter(args);

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if ( !pAircraft )
			return;

		NxVec3 vEntryPos = pAircraft->GetGlobalPosition();	//	进入点
		NxVec3 vRelayPoint;									//	中继点

		AircraftEventCircle& circleArgs = (AircraftEventCircle&)args;
		_Center = circleArgs._Center;
		_StableRadius = pAircraft->GetConvoluteRadius();

		float fRadius = pAircraft->GetMinRadius();
		float fAngle = 180.f;
		if ( _Center.equals(vEntryPos, 0.1f) )
		{
			//	由圆心切入盘旋，切入轨迹中心自定
			NxVec3 vDir = pAircraft->GetForward().cross(-AXIS_VEC_Z);
			vDir.z = 0;
			vDir.normalize();

			_StepInCenter = _Center + vDir * fRadius;
			vRelayPoint = _StepInCenter + vDir * fRadius;

			OrbitParams stepinPhase(CW, pAircraft->GetGlobalPosition(), _StepInCenter, fRadius);
			stepinPhase._Critical = fAngle;
			_Step.push_back(stepinPhase);

			OrbitParams circlePhase(CW, vRelayPoint, _Center, fRadius * 2);
			_Step.push_back(circlePhase);
		} 
		else if (NxMath::equals(vEntryPos.distance(_Center), fRadius * 2, 0.1f))
		{
			//	已在圆周轨迹上
			OrbitParams circlePhase(CW, vEntryPos, _Center, fRadius * 2);
			_Step.push_back(circlePhase);
		}
		else //if ( pAircraft->GetSquadIndex() != 0 )
		{
			//	默认值（仅作保护用）
			{
				NxVec3 vDir = pAircraft->GetForward().cross(-AXIS_VEC_Z);
				vDir.z = 0;
				vDir.normalize();

				_StepInCenter = _Center + vDir * fRadius;
				vRelayPoint = _StepInCenter + vDir * fRadius;
			}

			//	非圆心切入，切入轨迹中心随长机
			PhysAircraft* pHeader = pAircraft->GetHeader();
			if ( pHeader )
			{
				GameObjectState* pHeaderState = pHeader->GetCurrentState();
				if ( pHeaderState && pHeaderState->GetEnumValue() == Circle )
				{
					AircraftStateCircle* pState = (AircraftStateCircle*)pHeaderState;
					_Center = pState->_Center;
					_StepInCenter = pState->_StepInCenter;

					float fActualRadius = pAircraft->GetRadius(CW);

					NxVec3 vDir = _StepInCenter - _Center;
					vDir.z = 0;
					vDir.normalize();
					vRelayPoint = _StepInCenter + vDir * fActualRadius;

					//	将位置近似修正到圆周上，并计算临界角度
					NxVec3 vFrom = vEntryPos - _Center;
					vFrom.z = 0;
					vFrom.normalize();
					vFrom = _Center + vFrom * fRadius;

					DirectionalCircle stepInCircle(_Center, fActualRadius, DirectionalCircle::ECW);
					stepInCircle.CalcArc(vFrom, vRelayPoint, fAngle);
				}
			}

			OrbitParams stepinPhase(CW, pAircraft->GetGlobalPosition(), _StepInCenter, fRadius);
			stepinPhase._Critical = fAngle;
			_Step.push_back(stepinPhase);

			OrbitParams circlePhase(CW, vRelayPoint, _Center, fRadius * 2);
			_Step.push_back(circlePhase);
		}

		BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
		if ( pBehaviorReport )
			pBehaviorReport->OnInnerCommand(pAircraft, Circle, pAircraft->_UTargetPos, pAircraft->_UTargetDir);
	}

	void AircraftStateCircle::OnLeave(GameObjectEvent& args)
	{
		__super::OnLeave(args);

		if ( !_Host )
			return;

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if ( !pAircraft )
			return;

		pAircraft->SetSpeedFactor(1.f);
	}

	void AircraftStateCircle::Update(float DeltaTime)
	{
		__super::Update(DeltaTime);
	}

	void AircraftStateCircle::RefreshParam(OrbitParams& param, PhysAircraft* pAircraft)
	{
		if ( !pAircraft )
			return;

		NxVec3 Pos = pAircraft->GetGlobalPosition();
		if ( pAircraft->CheckFlag(VerticalMove) || pAircraft->CheckFlag(ExigencePull) )
		{
			param._Center.z = Pos.z;
			param._Positon.z = Pos.z;
		}

		if ( _Step.size() > 1 )
			return;

		//	for state sync
		if ( Pos.distanceSquared(param._Positon) < 0.1f )
		{
			BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
			if ( pBehaviorReport )
				pBehaviorReport->OnAction(pAircraft, *this);
		}

#if 0
		if ( pAircraft->GetSquadIndex() > 0 )
		{
			float fStandardRadius = pAircraft->GetMinRadius() * 2;

			if ( !NxMath::equals(param._Radius, _StableRadius, 0.01f) )
			{
				param._Radius += (_StableRadius - fStandardRadius) * 0.01f;
				pAircraft->SetSpeedFactor(param._Radius / fStandardRadius);
			}
		}
#endif
	}

	void AircraftStateCircle::PostMotion(PhysAircraft* pAircraft)
	{
		if ( !pAircraft )
			return;

		BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
		if ( pBehaviorReport )
			pBehaviorReport->OnAction(pAircraft, *this);
	}

}	//	namespace GPL