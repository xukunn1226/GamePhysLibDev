//------------------------------------------------------------------------
//	@brief	舰载机坠机状态
//	@author	chenpu
//	@date	2013-2-4
//------------------------------------------------------------------------
#include "PhysXSupport.h"
#include "AircraftFSM.h"
#include "AircraftStateCrash.h"

namespace GPL {

	bool AircraftStateCrash::OnEvent(GameObjectEvent& args)
	{
		return false;
	}

	void AircraftStateCrash::OnEnter(GameObjectEvent& args)
	{
		__super::OnEnter(args);

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if ( !pAircraft )
			return;

		NxVec3 Src = pAircraft->GetGlobalPosition();

#ifdef _SERVER_RUNTIME
		//	服务器随机产生坠机参数
		AircraftEventCrash& e = (AircraftEventCrash&)args;
		if ( e._Shoot )
			_Duration = NxMath::rand(5.f, 14.f);
		else
			_Duration = 255;
		NxVec3 Des = Src + pAircraft->GetForward() * pAircraft->GetSpeed() * _Duration;

		OrbitParams param(Src, Des);
		_Step.push_back(param);

		pAircraft->_UTargetPos = Des * P2GScale;
		pAircraft->_UTargetDir = pAircraft->GetForward();
#endif // _SERVER_RUNTIME

#ifdef _CLIENT_RUNTIME
		//	客户端用服务器下发的数据
		NxVec3 Des = pAircraft->_UTargetPos * G2PScale;

		OrbitParams param(Src, Des);
		_Step.push_back(param);
#endif // _CLIENT_RUNTIME

		_Height = Src.z;

		pAircraft->SetFlag(VerticalMove, true);
		pAircraft->SetFlag(Falling, true);

		BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
		if ( pBehaviorReport )
			pBehaviorReport->OnInnerCommand(pAircraft, Crash, pAircraft->_UTargetPos, pAircraft->_UTargetDir);
	}

	void AircraftStateCrash::OnLeave(GameObjectEvent& args)
	{
		__super::OnLeave(args);

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if ( !pAircraft )
			return;

		pAircraft->SetFlag(Falling, false);
		pAircraft->SetFlag(VerticalMove, false);
	}

	void AircraftStateCrash::Update(float DeltaTime)
	{
		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if ( !pAircraft )
			return;

		NxVec3 PrevPos = pAircraft->GetGlobalPosition();

		__super::Update(DeltaTime);

		if ( !_Step.empty() )
		{
			PhysGameObject* pObj = CollisionDetect(pAircraft->GetPhysScene(), PrevPos, pAircraft->GetGlobalPosition());
			if ( NULL != pObj )
			{
				//if ( IsKind(PhysOcean, pObj) )
				//{
				//}
				//else if ( IsKind(PhysVehicle, pObj) )
				//{
				//}
				_Step.clear();

				BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
				if ( NULL != pBehaviorReport )
					pBehaviorReport->OnAction(pAircraft, *this);
			}
		}
	}

	void AircraftStateCrash::RefreshParam(OrbitParams& params, PhysAircraft* pAircraft)
	{
		if ( !pAircraft )
			return;

		params._Positon.z = pAircraft->GetGlobalHeight();
		params._Target.z = params._Positon.z;
	}

	void AircraftStateCrash::PostMotion(PhysAircraft* pAircraft)
	{
		BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
		if ( NULL != pBehaviorReport )
			pBehaviorReport->OnAction(pAircraft, *this);
	}

	PhysGameObject* AircraftStateCrash::CollisionDetect(NxScene* Scene, const NxVec3& Src, const NxVec3& Des)
	{
		if ( !Scene )
			return NULL;

		NxRaycastHit Rst;

		NxRay WorldRay;
		WorldRay.orig = Src;
		WorldRay.dir = Des - Src;
		NxReal MaxDis = WorldRay.dir.normalize();
		if (!NxMath::equals(WorldRay.dir.magnitude(), 1.f, 0.0001f))
		{
			//gplDebugf(TEXT("AircraftStateCrash::CollisionDetect NxRay dir warning"));
			return NULL;
		}

		NxU32 groups = 0;
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_OCEAN);
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_TERRAIN);
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_STATICOBJECT);
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_VEHICLE_SHIP);

		NxShape* Shape = Scene->raycastClosestShape(WorldRay, NX_ALL_SHAPES, Rst, groups, MaxDis);
		if( NULL != Shape )
			//HitComp = (ShipComponent *)Shape->userData;
			return (PhysGameObject*)(Shape->getActor().userData);

		return NULL;
	}

}	// namespace GPL