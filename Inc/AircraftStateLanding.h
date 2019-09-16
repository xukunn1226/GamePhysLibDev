//------------------------------------------------------------------------
//	@brief	���ػ�����״̬
//	@author	chenpu
//	@date	2013-2-4
//------------------------------------------------------------------------
#include "PhysCommonDefine.h"
#include "AircraftState.h"

#pragma once

namespace GPL {

	class AircraftStateLanding : public AircraftState
	{
	public:
		AircraftStateLanding() 
			: AircraftState(Landing)
		{}
		virtual ~AircraftStateLanding()	{}

		//	@brief	�¼�����
		virtual bool	OnEvent(GameObjectEvent& args);

		//	@brief	����״̬��Ӧ
		virtual void	OnEnter(GameObjectEvent& args);

		//	@brief	�뿪״̬��Ӧ
		virtual void	OnLeave(GameObjectEvent& args);

		//	@brief	״̬����
		virtual void	Update(float DeltaTime);

	protected:
		//	@brief	�켣��������
		virtual	void			RefreshParam(OrbitParams& params, PhysAircraft* pAircraft);

		//	@brief	�������֪ͨ
		virtual void			PostMotion(PhysAircraft* pAircraft);
	};

}	//	namespace GPL