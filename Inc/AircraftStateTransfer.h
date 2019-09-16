//------------------------------------------------------------------------
//	@brief	舰载机移动状态
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

		//	@brief	事件处理
		virtual bool	OnEvent(GameObjectEvent& args);

		//	@brief	进入状态响应
		virtual void	OnEnter(GameObjectEvent& args);

		//	@brief	离开状态响应
		virtual void	OnLeave(GameObjectEvent& args);

		//	@brief	状态更新
		virtual void	Update(float DeltaTime);

		//	@brief	获取阶段数
		inline UINT		GetMotionNum()	{ return (UINT)_Step.size(); }

	protected:
		//	@brief	轨迹参数更新
		virtual	void	RefreshParam(OrbitParams& params, PhysAircraft* pAircraft);

		//	@brief	动作完成通知
		virtual void	PostMotion	(PhysAircraft* pAircraft);
	};

}	//	namespace GPL