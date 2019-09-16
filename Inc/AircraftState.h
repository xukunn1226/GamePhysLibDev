//------------------------------------------------------------------------
//	@brief	���ػ�״̬
//	@note	���켣���Ƶ���������������ɵĻ��࣬����λ�ü���
//	@author	chenpu
//	@date	2013-4-16
//------------------------------------------------------------------------
#include "PhysCommonDefine.h"
#include "GameObjectState.h"

#pragma once

namespace GPL {

	//	@brief	���з�ʽ
	enum WiseType
	{
		None = 0,
		CW,
		CCW,
	};

	//	@brief	�켣����
	struct OrbitParams 
	{
		WiseType	_Wise;			//	���з���
		NxVec3		_Positon;		//	��ʼλ��
		NxVec3		_Target;		//	Ŀ��λ��
		NxVec3		_Center;		//	����Բ��
		float		_Radius;		//	���а뾶
		float		_Critical;		//	�ٽ�Ƕ�
		float		_PassedTime;	//	����ʱ��
		float		_SpeculateTime;	//	�Ʋ�ʱ��

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

	//	@brief	��������
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

		//	@brief	��ö�������
		//	@param	[phase]		ָ���׶�
		//	@param	[params]	out	��������
		//	@return	�Ƿ��ҵ�ָ���׶εĶ���
		bool					GetMotionParam(UINT phase, OrbitParams& params);

		//	@brief	���Ķ�������
		//	@param	[phase]		ָ���׶�
		//	@param	[params]	�µĲ���
		void					ModifyMotionParam(UINT phase, const OrbitParams& params);

		//	@brief	��ȡʣ��ʱ��
		float					GetRemainTime();

		//	@brief	��ö�����
		inline UINT				GetMotionNum() const	{ return (UINT)_Step.size(); }

		//	@brief	ǿ�Ƶ�ջ
		void					ForceStep();

		//	@brief	״̬ͬ��
		void					Coordinate(const NxVec3& Pos, const NxVec3& Dir);

		//	@brief	λ��Ԥ��
		//	@param	[Time]	����ʱ��
		//	@param	[Pos]	Ԥ��λ��
		//	@param	[Dir]	Ԥ�⳯��
		void					MovementForecast(float Time, NxVec3& Pos, NxVec3& Dir);

	protected:
		//	@brief	����λ��
		//	@param	[args]		�켣����
		//	@param	[accumTime]	�ۻ�ʱ��
		//	@return	�Ƿ���ɹ켣
		bool					StepMovement(OrbitParams& args, float DeltaTime);

		//	@brief	�켣��������
		//	@note	����ʱ���ϸ�ע���÷���ֻ�����ڹ켣�������������ɶԶ������н��в���
		//	@param	[params]	��ǰ�����켣����
		//	@param	[pAircraft]	����ִ�ж���
		virtual	void			RefreshParam(OrbitParams& params, PhysAircraft* pAircraft)	{}

		//	@brief	�������֪ͨ
		//	@param	[pAircraft]	����ִ�ж���
		virtual void			PostMotion(PhysAircraft* pAircraft)	{}

	protected:
		//	@brief	��������
		MotionList		_Step;
	};
}