//------------------------------------------------------------------------
//	@brief	舰载机状态
//	@note	将轨迹控制单独抽象出来所构成的基类，用于位置计算
//	@author	chenpu
//	@date	2013-4-16
//------------------------------------------------------------------------
#include "PhysCommonDefine.h"
#include "GameObjectState.h"

#pragma once

namespace GPL {

	//	@brief	绕行方式
	enum WiseType
	{
		None = 0,
		CW,
		CCW,
	};

	//	@brief	轨迹参数
	struct OrbitParams 
	{
		WiseType	_Wise;			//	绕行方向
		NxVec3		_Positon;		//	初始位置
		NxVec3		_Target;		//	目标位置
		NxVec3		_Center;		//	绕行圆心
		float		_Radius;		//	绕行半径
		float		_Critical;		//	临界角度
		float		_PassedTime;	//	经过时间
		float		_SpeculateTime;	//	推测时间

		//	default constructor
		OrbitParams()
			: _Wise(None)
			, _Positon(ZERO_VEC)
			, _Target(ZERO_VEC)
			, _Center(ZERO_VEC)
			, _Radius(10.f)
			, _Critical(0.f)
			, _PassedTime(0.f)
		{
		}

		//	constructor for line
		OrbitParams(const NxVec3& from, const NxVec3& to)
			: _Wise(None)
			, _Positon(from)
			, _Target(to)
			, _Center(ZERO_VEC)
			, _Radius(10.f)
			, _Critical(0.f)
			, _PassedTime(0.f)
		{
			//assert(from.x < 100000.f && from.x > -100000.f);
			//assert(from.y < 100000.f && from.y > -100000.f);
			//assert(to.x < 100000.f && to.x > -100000.f);
			//assert(to.y < 100000.f && to.y > -100000.f);
		}

		//	constructor for circle
		OrbitParams(WiseType wise, const NxVec3& entry, const NxVec3& center, float radius)
			: _Wise(wise)
			, _Positon(entry)
			, _Target(ZERO_VEC)
			, _Center(center)
			, _Radius(radius)
			, _Critical(0.f)
			, _PassedTime(0.f)
		{
			//assert(!entry.equals(center, 0.01f));
		}
	};

	//	@brief	动作队列
	typedef std::vector<OrbitParams>	MotionList;

	class PhysAircraft;
	class AircraftState : public GameObjectState
	{
	public:
		AircraftState(NxU8 _enum) 
			: GameObjectState(_enum)
		{
		}
		virtual ~AircraftState()		{ _Step.clear(); }

		virtual void			OnEnter	(GameObjectEvent& args);

		virtual void			OnLeave	(GameObjectEvent& args);

		virtual void			Update	(float DeltaTime);

		//	@brief	获得动作参数
		//	@param	[phase]		指定阶段
		//	@param	[params]	out	动作参数
		//	@return	是否找到指定阶段的动作
		bool					GetMotionParam(UINT phase, OrbitParams& params);

		//	@brief	更改动作参数
		//	@param	[phase]		指定阶段
		//	@param	[params]	新的参数
		void					ModifyMotionParam(UINT phase, const OrbitParams& params);

		//	@brief	获取剩余时间
		float					GetRemainTime();

		//	@brief	获得动作数
		inline UINT				GetMotionNum() const	{ return (UINT)_Step.size(); }

		//	@brief	强制弹栈
		void					ForceStep();

		//	@brief	状态同调
		void					Coordinate(const NxVec3& Pos, const NxVec3& Dir);

		//	@brief	位置预测
		//	@param	[Time]	经过时间
		//	@param	[Pos]	预测位置
		//	@param	[Dir]	预测朝向
		void					MovementForecast(float Time, NxVec3& Pos, NxVec3& Dir);

	protected:
		//	@brief	计算位置
		//	@param	[args]		轨迹参数
		//	@param	[accumTime]	累积时间
		//	@return	是否完成轨迹
		bool					StepMovement(OrbitParams& args, float DeltaTime);

		//	@brief	轨迹参数更新
		//	@note	重载时需严格注意用法，只可用于轨迹参数修正，不可对动作序列进行操作
		//	@param	[params]	当前动作轨迹参数
		//	@param	[pAircraft]	动作执行对象
		virtual	void			RefreshParam(OrbitParams& params, PhysAircraft* pAircraft)	{}

		//	@brief	动作完成通知
		//	@param	[pAircraft]	动作执行对象
		virtual void			PostMotion(PhysAircraft* pAircraft)	{}

	protected:
		//	@brief	动作队列
		MotionList		_Step;
	};
}