//------------------------------------------------------------------------
//	@brief	���ػ�����״̬
//	@author	chenpu
//	@date	2013-2-4
//------------------------------------------------------------------------
#include "PhysCommonDefine.h"
#include "AircraftState.h"

#pragma once

namespace GPL {

	class AircraftStateReady : public AircraftState
	{
	public:
		AircraftStateReady() : AircraftState(Ready) {}
		virtual ~AircraftStateReady()	{}

		//	@brief	�¼�����
		virtual bool	OnEvent(GameObjectEvent& args);

		virtual void	OnEnter(GameObjectEvent& args);

		virtual void	Update(float DeltaTime);
	};

}	//	namespace GPL