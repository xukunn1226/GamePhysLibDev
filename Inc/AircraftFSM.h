//------------------------------------------------------------------------
//	@brief	���ػ�״̬��
//	@author	chenpu
//	@date	2013-2-3
//------------------------------------------------------------------------
#include "GameObjectState.h"
#include "AircraftStateAttack.h"
#include "AircraftStateCircle.h"
#include "AircraftStateLanding.h"
#include "AircraftStateTransfer.h"
#include "AircraftStateReady.h"
#include "AircraftStateTakeOff.h"
#include "AircraftStateCrash.h"

#pragma once

namespace GPL {

	//	@brief	�¼�����
	DECLARE_FSM_EVENT(Aircraft, TakeOff)
	DECLARE_FSM_EVENT_X2(Aircraft, Landing, NxVec3, _Target, NxVec3, _Direction)
	DECLARE_FSM_EVENT_X3(Aircraft, Transfer, NxVec3, _Target, NxVec3, _Direction, PhysGameObject*, _TargetObj)
	DECLARE_FSM_EVENT_X2(Aircraft, Circle, NxVec3, _Center, NxVec3, _Direction)
	DECLARE_FSM_EVENT_X2(Aircraft, Attack, NxVec3, _Target, NxVec3, _Direction)
	DECLARE_FSM_EVENT(Aircraft, Ready)
	DECLARE_FSM_EVENT_X1(Aircraft, Crash, bool, _Shoot)

	//	��������
	struct ActionData
	{
		AircraftAction	_action;
		NxVec3			_tar;
		NxVec3			_dir;
		PhysGameObject*	_obj;

		ActionData() 
			: _action(Ready)
			, _tar(ZERO_VEC)
			, _dir(ZERO_VEC)
			, _obj(NULL)
		{
		}
	};

	//	��������
	typedef std::list<ActionData> ActionQueue;

	class PhysAircraft;
	class AircraftFSM : public GameObjectFSM
	{
	public:
		AircraftFSM()	{}
		virtual ~AircraftFSM()	{}

		virtual bool	Init(FSMActor* owner);

		virtual void	Update(float DeltaTime);

		void			Coordinate(const NxVec3& Pos, const NxVec3& Dir);

		//	@brief	������д�ִ�ж���
		void			ClearActions();

		//	@brief	����ִ�ж���ѹ�����
		void			PushAction(AircraftAction action, const NxVec3& tar, const NxVec3& dir, PhysGameObject* obj);

		//	@brief	ִ�в��������׶���
		bool			PopAction();

		//	@brief	��ȡջ�ڶ���
		ActionData*		GetPendingAction(UINT idx);

		//	@brief	��ȡ���һ����ִ�ж���
		AircraftAction	GetLastAction();

		//	@brief	�������һ����ִ�ж���
		void			DropLastAction();

		//	@brief	��ȡ��ִ�ж�������
		UINT			GetPendingActionNum();

		//	@brief	������ִ�ж�������
		void			CalcPendingMoveDir();

	protected:
		bool			GotoTakeoff();															//	�л������״̬
		bool			GotoCircle();															//	������λ��ΪԲ������
		bool			GotoCircle(const NxVec3& center);										//	��ָ��λ��ΪԲ������
		bool			GotoTransfer(const NxVec3& pos, const NxVec3& dir = ZERO_VEC);			//	Ǩ�Ƶ�ָ��Ŀ���
		bool			GotoTransfer(PhysGameObject* obj);										//	׷��ָ��Ŀ��
		bool			GotoLanding(const NxVec3& pos = ZERO_VEC, const NxVec3& dir = ZERO_VEC);//	�л�������״̬
		bool			GotoAttack(const NxVec3& pos, const NxVec3& dir = ZERO_VEC);
		bool			GotoAttack(PhysGameObject* pObj);
		bool			GotoCrash(bool bShoot);

	protected:
		AircraftStateReady		_StateReady;
		AircraftStateTakeOff	_StateTakeOff;
		AircraftStateLanding	_StateLanding;
		AircraftStateTransfer	_StateTransfer;
		AircraftStateCircle		_StateCircle;
		AircraftStateAttack		_StateAttack;
		AircraftStateCrash		_StateCrash;

		//	@brief	��������
		ActionQueue				_ActionQueue;

		//	@brief	ǰһ������
		DWORD					_PrevState;
	};

}	//	namespace GPL