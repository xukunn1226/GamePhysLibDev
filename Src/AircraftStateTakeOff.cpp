//------------------------------------------------------------------------
//	@brief	舰载机起飞状态
//	@author	chenpu
//	@date	2013-2-4
//------------------------------------------------------------------------
#include "PhysXSupport.h"
#include "AircraftFSM.h"
#include "AircraftStateTakeOff.h"
#include "PhysGameObjectAircraft.h"

namespace GPL {

	bool AircraftStateTakeOff::OnEvent(GameObjectEvent& args)
	{
		if ( NULL == args._State )
			return false;

		if ( Crash == args._State->GetEnumValue() )
			return true;

		return false;
	}

	void AircraftStateTakeOff::OnEnter(GameObjectEvent& args)
	{
		__super::OnEnter(args);

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if ( !pAircraft )
			return;

		//	离舰点
		NxVec3 LeavePos = pAircraft->GetLeaveLocation();

		OrbitParams slidePhase(pAircraft->GetGlobalPosition(), LeavePos);
		_Step.push_back(slidePhase);

		OrbitParams climbPhase(LeavePos, ZERO_VEC);	//	To be Update
		_Step.push_back(climbPhase);

		BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
		if ( pBehaviorReport )
			pBehaviorReport->OnInnerCommand(pAircraft, TakeOff, pAircraft->_UTargetPos, pAircraft->_UTargetDir);
	}

	void AircraftStateTakeOff::OnLeave(GameObjectEvent& args)
	{
		__super::OnLeave(args);

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if ( !pAircraft )
			return;

		pAircraft->SetFlag(LockHeader, true);
		pAircraft->ResetVerticalSpeed();

		if ( ((AircraftFSM*)_Host)->GetPendingActionNum() == 0 )
		{
			NxVec3 Target;
			AircraftAction Action;

			PhysAircraft* pHeader = pAircraft->GetHeader();
			if ( NULL == pHeader )
			{
				Target = pAircraft->GetGlobalPosition();
				Action = Circle;
			}
			else
			{
				Target = pHeader->GetConvoluteCenter();

				if ( Target.equals(ZERO_VEC, 0.01f) )
				{
					//	Invalid center
					Action = Circle;
					Target = pAircraft->GetGlobalPosition();
				}
				else if ( Target.equals(pAircraft->GetGlobalPosition(), 0.01f) )
				{
					//	Already at center
					Action = Circle;
				}
				else
				{
					//	Need to transfer to center
					Action = Transfer;
				}
			}

			((AircraftFSM*)_Host)->PushAction(Action, Target, ZERO_VEC, NULL);
		}
	}

	void AircraftStateTakeOff::Update(float DeltaTime)
	{
		__super::Update(DeltaTime);
	
		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if (NULL != pAircraft)
		{
			if (pAircraft->GetGlobalHeight() >= RouteAssist::GetSingletonRef().GetLayerHeight(Higher))
			{
				_Step.clear();
				PostMotion(pAircraft);
				return;
			}
		}
	}

	void AircraftStateTakeOff::RefreshParam(OrbitParams& params, PhysAircraft* pAircraft)
	{
		//	滑行阶段，根据运动航母的相对位置更新轨迹参数
		if ( !pAircraft )
			return;

		if (_Step.size() == 2)
		{			
			params._Positon = pAircraft->GetStartLocation();	//	出生点
			params._Target = pAircraft->GetLeaveLocation();		//	离舰点
		}
		else if (_Step.size() == 1)
		{
			params._Positon.z = pAircraft->GetGlobalHeight();
			params._Target.z = pAircraft->GetGlobalHeight();

			pAircraft->SetFlag(VerticalMove, true);
			pAircraft->SetFlag(Ascend, true);
		}
	}

	void AircraftStateTakeOff::PostMotion(PhysAircraft* pAircraft)
	{
		if ( !pAircraft )
			return;

		BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
		if ( pBehaviorReport )
			pBehaviorReport->OnAction(pAircraft, *this);

		if ( _Step.empty() )
		{
			//	主动结束
			GameObjectEvent args;
			OnLeave(args);
		}
		else
		{
			//	更新轨迹参数
			OrbitParams& params = _Step.front();
			params._Positon = pAircraft->GetGlobalPosition();
			params._Target = params._Positon + pAircraft->GetForward() * GPhysGameSetting._GPL_MAX_HALF_WORLD;

#ifdef _SERVER_RUNTIME
			//	通知后续起飞
			pAircraft->OnLeaveBoard();
#endif // _SERVER_RUNTIME
		}
	}

}	// namespace GPL