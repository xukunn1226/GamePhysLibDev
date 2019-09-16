//------------------------------------------------------------------------
//	@brief	舰载机降落状态
//	@author	chenpu
//	@date	2013-2-4
//------------------------------------------------------------------------
#include "PhysXSupport.h"
#include "AircraftFSM.h"
#include "AircraftStateTakeOff.h"
#include "PhysGameObjectAircraft.h"
#include "PhysGameObjectVehicle.h"

namespace GPL {

	bool AircraftStateLanding::OnEvent(GameObjectEvent& args)
	{
		if ( NULL == args._State )
			return false;

		if ( Crash == args._State->GetEnumValue() )
			return true;

		return false;
	}

	void AircraftStateLanding::OnEnter(GameObjectEvent& args)
	{
		__super::OnEnter(args);

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if ( !pAircraft )
			return;

		pAircraft->SetFlag(LockHeader, false);
		pAircraft->SetFlag(VerticalMove, false);
		pAircraft->SetSpeedFactor(1.f);

		AircraftEventLanding& landingArgs = (AircraftEventLanding&)args;

		//	At the client, in case that the aircraft was not specified an owner, we use points passed by EventArgs as OrbitParam
		//	Actrually, it can be updated by RefreshParam, if the aircraft do has an owner
		NxVec3 CatchPoint = landingArgs._Target - landingArgs._Direction * 20;

		//	下降阶段
		OrbitParams declinePhase(_Host->GetActor()->GetGlobalPosition(), CatchPoint);
		_Step.push_back(declinePhase);

		//	滑行阶段
		OrbitParams slidePhase(CatchPoint, landingArgs._Target);
		_Step.push_back(slidePhase);

		BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
		if ( pBehaviorReport )
			pBehaviorReport->OnInnerCommand(pAircraft, Landing, pAircraft->_UTargetPos, pAircraft->_UTargetDir);
	}

	void AircraftStateLanding::OnLeave(GameObjectEvent& args)
	{
		__super::OnLeave(args);

		((AircraftFSM*)_Host)->ClearActions();
		((AircraftFSM*)_Host)->PushAction(Ready, ZERO_VEC, ZERO_VEC, NULL);
	}

	void AircraftStateLanding::Update(float DeltaTime)
	{
		__super::Update(DeltaTime);
	}

	void AircraftStateLanding::RefreshParam(OrbitParams& param, PhysAircraft* pAircraft)
	{
		if ( !pAircraft )
			return;

		if ( !pAircraft->GetBaseShip() )
			return;

		if ( _Step.empty() )
			return;

		if ( _Step.size() > 1 )
		{
			//	下降阶段更新轨迹参数
			param._Positon = pAircraft->GetGlobalPosition();
			param._Target = pAircraft->GetCatchLocation();

			//	移动目标点，用当前位置和DeltaTime作线段更新
			param._PassedTime = 0.f;
		}
		else 
		{
			//	滑行阶段更新轨迹参数
			param._Positon = pAircraft->GetCatchLocation();
			param._Target = pAircraft->GetStartLocation();
		}
	}

	void AircraftStateLanding::PostMotion(PhysAircraft* pAircraft)
	{
		if ( !pAircraft )
			return;

		BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
		if ( pBehaviorReport )
			pBehaviorReport->OnAction(pAircraft, *this);

		if ( _Step.empty() )
		{		
			GameObjectEvent args;
			OnLeave(args);
		} 
		else
		{
			pAircraft->SetSpeedFactor(0.5f);
		}
	}

}	// namespace GPL