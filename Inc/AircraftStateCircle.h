//------------------------------------------------------------------------
//	@brief	舰载机盘旋状态
//	@author	chenpu
//	@date	2013-2-4
//------------------------------------------------------------------------
#include "PhysCommonDefine.h"
#include "AircraftState.h"
#include "NxVec3.h"

#pragma once

namespace GPL {

	class RangeTrigger;
	class GameObjectFSM;
	class AircraftStateCircle : public AircraftState
	{
	public:
		AircraftStateCircle() : AircraftState(Circle) {}
		virtual ~AircraftStateCircle()	{}

		//	@brief	事件处理
		virtual bool	OnEvent(GameObjectEvent& args);

		//	@brief	进入状态响应
		virtual void	OnEnter(GameObjectEvent& args);

		//	@brief	离开状态响应
		virtual void	OnLeave(GameObjectEvent& args);

		//	@brief	状态更新
		virtual void	Update(float DeltaTime);

		//	@brief	获得盘旋中心
		inline NxVec3	GetCenter()	{ return _Center; }

	protected:
		//	@brief	轨迹参数更新
		virtual void	RefreshParam(OrbitParams& params, PhysAircraft* pAircraft);

		//	@brief	动作完成通知
		virtual void	PostMotion	(PhysAircraft* pAircraft);

	protected:
		//	@brief	稳定运行半径
		float			_StableRadius;

		//	@brief	绕行圆心
		NxVec3			_Center;

		//	@brief	切入圆圆心
		NxVec3			_StepInCenter;

		//	@brief	与AXIS_VEC_X绕顺时针方向的夹角
		//	@brief	用于逼近算法
		float			_Angle;
	};

}	//	namespace GPL