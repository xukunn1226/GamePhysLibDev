//------------------------------------------------------------------------
//	@brief	���ػ�����״̬
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

		//	@brief	�¼�����
		virtual bool	OnEvent(GameObjectEvent& args);

		//	@brief	����״̬��Ӧ
		virtual void	OnEnter(GameObjectEvent& args);

		//	@brief	�뿪״̬��Ӧ
		virtual void	OnLeave(GameObjectEvent& args);

		//	@brief	״̬����
		virtual void	Update(float DeltaTime);

		//	@brief	�����������
		inline NxVec3	GetCenter()	{ return _Center; }

	protected:
		//	@brief	�켣��������
		virtual void	RefreshParam(OrbitParams& params, PhysAircraft* pAircraft);

		//	@brief	�������֪ͨ
		virtual void	PostMotion	(PhysAircraft* pAircraft);

	protected:
		//	@brief	�ȶ����а뾶
		float			_StableRadius;

		//	@brief	����Բ��
		NxVec3			_Center;

		//	@brief	����ԲԲ��
		NxVec3			_StepInCenter;

		//	@brief	��AXIS_VEC_X��˳ʱ�뷽��ļн�
		//	@brief	���ڱƽ��㷨
		float			_Angle;
	};

}	//	namespace GPL