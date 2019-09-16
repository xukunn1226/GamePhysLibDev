#include "SingletonSupport.h"

#pragma once

namespace GPL
{
	class DirectionalCircle;
	class RouteAssist : public Singleton<RouteAssist>
	{
	public:
		RouteAssist();
		virtual ~RouteAssist()	{}

#if 0
		//	@brief	路径规划
		//	@param	[SrcPos]	起点位置
		//	@param	[SrcDir]	起点方向
		//	@param	[DesPos]	目标位置
		//	@param	[DesDir]	目标方向
		//	@param	[Radius]	绕行半径
		//	@return				输出路径
		MotionList		Route(const NxVec3& SrcPos, const NxVec3& SrcDir, const NxVec3& DesPos, const NxVec3& DesDir, float Radius);
#endif

		//	@brief	路径规划
		//	@param	[Actor]		路径规划对象
		//	@param	[DesPos]	目标位置
		//	@param	[DesDir]	目标方向
		//	@param	[Path]		输出路径
		void			Route(PhysAircraft* Actor, const NxVec3& DesPos, const NxVec3& DesDir, MotionList& Path);

		//	@brief	路径规划
		//	@param	[Actor]		路径规划对象
		//	@param	[DesPos]	目标位置
		//	@param	[Path]		输出路径
		void			RouteWithoutDirection(PhysAircraft* Actor, const NxVec3& DesPos, MotionList& Path);

		//	@brief	对象跟踪
		//	@param	[Actor]		路径规划对象
		//	@param	[TargetObj]	目标对象
		//	@param	[Path]		输出路径
		//	@param	[ignoreDir]	忽略目标对象朝向
		void			ObjectTrace(PhysAircraft* Actor, PhysGameObject* TargetObj, MotionList& Path, bool ignoreDir = true);

		//	@brief	获得空域高度
		//	@param	[layer]		空域枚举
		float			GetLayerHeight(FlightLayer layer);

		//	@brief	计算切入点
		//	@param	[Action]	状态
		//	@param	[Layer]		飞行空域
		//	@param	[Target]	目标点
		//	@param	[Direction]	目标方向
		NxVec3			CalcStepIn(PhysAircraft* Aircraft, AircraftAction Action, FlightLayer Layer, const NxVec3& Target, const NxVec3& Direction);

		//	@brief	计算切出点
		//	@param	[Action]	状态
		//	@param	[Layer]		飞行空域
		//	@param	[Target]	目标点
		//	@param	[Direction]	目标方向
		NxVec3			CalcStepOut(PhysAircraft* Aircraft, AircraftAction Action, const NxVec3& SrcPos, const NxVec3& Direction);

		//	@brief	计算路径长度
		//	@param	[src]		起始圆
		//	@param	[des]		目标圆
		//	@param	[from]		起始点
		//	@param	[to]		目标点
		//	@param	[Motions]	out	动作序列
		//	@return	路径长度，负值表示无法计算
		float			CalcRoutingLength(const DirectionalCircle& src, const DirectionalCircle& des, const NxVec3& from, const NxVec3& to, MotionList& Motions);

		//	@brief	最末阶段扩展
		void			ExtendLastMotion(PhysAircraft* Actor, MotionList& Path);

		//	@brief	计算俯冲时间
		float			CalcDiveDuration(float DiveHeight, float DiveAcceleration, float SpeedLimit);

	protected:
		//	@brief	空域高度
		float			_LayerHeight[LayerNum];
	};
}