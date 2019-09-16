//------------------------------------------------------------------------
//	@brief	½¢ÔØ»ú´ý»ú×´Ì¬
//	@author	chenpu
//	@date	2013-2-4
//------------------------------------------------------------------------
#include "PhysXSupport.h"
#include "AircraftFSM.h"
#include "AircraftStateReady.h"

namespace GPL {

	bool AircraftStateReady::OnEvent(GameObjectEvent& args)
	{
		if ( NULL == args._State )
			return false;

		if (TakeOff == args._State->GetEnumValue())
			return true;

		if (Crash == args._State->GetEnumValue())
		{
			PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
			if (NULL != pAircraft)
			{
				PhysVehicle* pCarrier = DynamicCast(PhysVehicle, pAircraft->GetOwner());
				if (NULL == pCarrier)
					return true;

				if (pCarrier->IsSinking())
					return true;
			}
		}

		return false;
	}

	void AircraftStateReady::OnEnter(GameObjectEvent& args)
	{
		__super::OnEnter(args);

		if ( !_Host )
			return;

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if ( !pAircraft )
			return;

		PhysVehicle* pBaseShip = DynamicCast(PhysVehicle, pAircraft->GetOwner());
		if ( !pBaseShip )
			return;

		NxVec3 Pos = pAircraft->GetStartLocation();
		NxQuat Rot = pBaseShip->GetGlobalOritationQuat();
		NxVec3 Dir = Rot.rot(AXIS_VEC_X);

		pAircraft->SetGlobalPosition(Pos);
		pAircraft->SetForward(Dir);
	}

	void AircraftStateReady::Update(float DeltaTime)
	{
		__super::Update(DeltaTime);

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if ( !pAircraft )
			return;

		PhysVehicle* pBaseShip = DynamicCast(PhysVehicle, pAircraft->GetOwner());
		if ( !pBaseShip )
			return;

		NxVec3 Pos = pAircraft->GetStartLocation();
		NxQuat Rot = pBaseShip->GetGlobalOritationQuat();
		NxVec3 Dir = Rot.rot(AXIS_VEC_X);

		pAircraft->SetGlobalPosition(Pos);
		pAircraft->SetForward(Dir);
	}

}	// namespace GPL