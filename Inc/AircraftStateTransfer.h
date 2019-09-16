//------------------------------------------------------------------------
//	@brief	���ػ��ƶ�״̬
//	@author	chenpu
//	@date	2013-2-4
//------------------------------------------------------------------------
#include "PhysCommonDefine.h"
#include "AircraftState.h"

#pragma once

namespace GPL {

	class GameObjectFSM;
	class AircraftStateTransfer : public AircraftState
	{
	public:
		AircraftStateTransfer() 
			: AircraftState(Transfer)
		{}
		virtual ~AircraftStateTransfer()	{}

		//	@brief	�¼�����
		virtual bool	OnEvent(GameObjectEvent& args);

		//	@brief	����״̬��Ӧ
		virtual void	OnEnter(GameObjectEvent& args);

		//	@brief	�뿪״̬��Ӧ
		virtual void	OnLeave(GameObjectEvent& args);

		//	@brief	״̬����
		virtual void	Update(float DeltaTime);

		//	@brief	��ȡ�׶���
		inline UINT		GetMotionNum()	{ return (UINT)_Step.size(); }

	protected:
		//	@brief	�켣��������
		virtual	void	RefreshParam(OrbitParams& params, PhysAircraft* pAircraft);

		//	@brief	�������֪ͨ
		virtual void	PostMotion	(PhysAircraft* pAircraft);
	};

}	//	namespace GPL