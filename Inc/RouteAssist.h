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
		//	@brief	·���滮
		//	@param	[SrcPos]	���λ��
		//	@param	[SrcDir]	��㷽��
		//	@param	[DesPos]	Ŀ��λ��
		//	@param	[DesDir]	Ŀ�귽��
		//	@param	[Radius]	���а뾶
		//	@return				���·��
		MotionList		Route(const NxVec3& SrcPos, const NxVec3& SrcDir, const NxVec3& DesPos, const NxVec3& DesDir, float Radius);
#endif

		//	@brief	·���滮
		//	@param	[Actor]		·���滮����
		//	@param	[DesPos]	Ŀ��λ��
		//	@param	[DesDir]	Ŀ�귽��
		//	@param	[Path]		���·��
		void			Route(PhysAircraft* Actor, const NxVec3& DesPos, const NxVec3& DesDir, MotionList& Path);

		//	@brief	·���滮
		//	@param	[Actor]		·���滮����
		//	@param	[DesPos]	Ŀ��λ��
		//	@param	[Path]		���·��
		void			RouteWithoutDirection(PhysAircraft* Actor, const NxVec3& DesPos, MotionList& Path);

		//	@brief	�������
		//	@param	[Actor]		·���滮����
		//	@param	[TargetObj]	Ŀ�����
		//	@param	[Path]		���·��
		//	@param	[ignoreDir]	����Ŀ�������
		void			ObjectTrace(PhysAircraft* Actor, PhysGameObject* TargetObj, MotionList& Path, bool ignoreDir = true);

		//	@brief	��ÿ���߶�
		//	@param	[layer]		����ö��
		float			GetLayerHeight(FlightLayer layer);

		//	@brief	���������
		//	@param	[Action]	״̬
		//	@param	[Layer]		���п���
		//	@param	[Target]	Ŀ���
		//	@param	[Direction]	Ŀ�귽��
		NxVec3			CalcStepIn(PhysAircraft* Aircraft, AircraftAction Action, FlightLayer Layer, const NxVec3& Target, const NxVec3& Direction);

		//	@brief	�����г���
		//	@param	[Action]	״̬
		//	@param	[Layer]		���п���
		//	@param	[Target]	Ŀ���
		//	@param	[Direction]	Ŀ�귽��
		NxVec3			CalcStepOut(PhysAircraft* Aircraft, AircraftAction Action, const NxVec3& SrcPos, const NxVec3& Direction);

		//	@brief	����·������
		//	@param	[src]		��ʼԲ
		//	@param	[des]		Ŀ��Բ
		//	@param	[from]		��ʼ��
		//	@param	[to]		Ŀ���
		//	@param	[Motions]	out	��������
		//	@return	·�����ȣ���ֵ��ʾ�޷�����
		float			CalcRoutingLength(const DirectionalCircle& src, const DirectionalCircle& des, const NxVec3& from, const NxVec3& to, MotionList& Motions);

		//	@brief	��ĩ�׶���չ
		void			ExtendLastMotion(PhysAircraft* Actor, MotionList& Path);

		//	@brief	���㸩��ʱ��
		float			CalcDiveDuration(float DiveHeight, float DiveAcceleration, float SpeedLimit);

	protected:
		//	@brief	����߶�
		float			_LayerHeight[LayerNum];
	};
}