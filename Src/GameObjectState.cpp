//------------------------------------------------------------------------
//	@brief	¶ÔÏó×´Ì¬Àà
//	@author	chenpu
//	@date	2013-2-3
//------------------------------------------------------------------------
#include "PhysXSupport.h"
#include "GameObjectState.h"

namespace GPL {

	void GameObjectState::OnEnter(GameObjectEvent& args)
	{
		_AccumTime = 0.f;

		//gplDebugf(TEXT("Enter State[%d]"), _Value);

		_Host->OnStateActivated(this);
	}

	void GameObjectState::OnLeave(GameObjectEvent& args)
	{
		//gplDebugf(TEXT("Leave State[%d]"), _Value);

		_Host->OnStateDeactived(this);
	}

	void GameObjectState::Update(float DeltaTime)
	{
		_AccumTime += DeltaTime;
	}
}	//	namespace GPL