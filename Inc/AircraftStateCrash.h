//------------------------------------------------------------------------
//	@brief	舰载机坠机状态
//	@author	chenpu
//	@date	2013-2-4
//------------------------------------------------------------------------
#include "AircraftState.h"

#pragma once

namespace GPL {

	class AircraftStateCrash : public AircraftState
	{
	public:
		AircraftStateCrash() 
			: AircraftState(Crash)
			, _Duration(0.f)
			, _Height(0.f)
		{}
		virtual ~AircraftStateCrash()	{}

		//	@brief	事件处理
		virtual bool	OnEvent(GameObjectEvent& args);

		virtual void	OnEnter(GameObjectEvent& args);

		virtual void	OnLeave(GameObjectEvent& args);

		virtual void	Update(float DeltaTime);

	protected:
		//	@brief	轨迹参数更新
		virtual	void	RefreshParam(OrbitParams& params, PhysAircraft* pAircraft);

		//	@brief	动作完成通知
		virtual void	PostMotion(PhysAircraft* pAircraft);

		//	@brief	坠机状态下的碰撞检测
		PhysGameObject*	CollisionDetect(NxScene* Scene, const NxVec3& Src, const NxVec3& Des);

	protected:
		float			_Duration;

		float			_Height;
	};

}	//	namespace GPL