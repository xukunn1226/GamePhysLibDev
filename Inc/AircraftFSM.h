//------------------------------------------------------------------------
//	@brief	舰载机状态机
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

	//	@brief	事件声明
	DECLARE_FSM_EVENT(Aircraft, TakeOff)
	DECLARE_FSM_EVENT_X2(Aircraft, Landing, NxVec3, _Target, NxVec3, _Direction)
	DECLARE_FSM_EVENT_X3(Aircraft, Transfer, NxVec3, _Target, NxVec3, _Direction, PhysGameObject*, _TargetObj)
	DECLARE_FSM_EVENT_X2(Aircraft, Circle, NxVec3, _Center, NxVec3, _Direction)
	DECLARE_FSM_EVENT_X2(Aircraft, Attack, NxVec3, _Target, NxVec3, _Direction)
	DECLARE_FSM_EVENT(Aircraft, Ready)
	DECLARE_FSM_EVENT_X1(Aircraft, Crash, bool, _Shoot)

	//	动作数据
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

	//	动作队列
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

		//	@brief	清空所有待执行动作
		void			ClearActions();

		//	@brief	将待执行动作压入队列
		void			PushAction(AircraftAction action, const NxVec3& tar, const NxVec3& dir, PhysGameObject* obj);

		//	@brief	执行并弹出队首动作
		bool			PopAction();

		//	@brief	获取栈内动作
		ActionData*		GetPendingAction(UINT idx);

		//	@brief	获取最后一个待执行动作
		AircraftAction	GetLastAction();

		//	@brief	舍弃最后一个待执行动作
		void			DropLastAction();

		//	@brief	获取待执行动作数量
		UINT			GetPendingActionNum();

		//	@brief	调整待执行动作方向
		void			CalcPendingMoveDir();

	protected:
		bool			GotoTakeoff();															//	切换到起飞状态
		bool			GotoCircle();															//	以自身位置为圆心盘旋
		bool			GotoCircle(const NxVec3& center);										//	以指定位置为圆心盘旋
		bool			GotoTransfer(const NxVec3& pos, const NxVec3& dir = ZERO_VEC);			//	迁移到指定目标点
		bool			GotoTransfer(PhysGameObject* obj);										//	追踪指定目标
		bool			GotoLanding(const NxVec3& pos = ZERO_VEC, const NxVec3& dir = ZERO_VEC);//	切换到降落状态
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

		//	@brief	动作队列
		ActionQueue				_ActionQueue;

		//	@brief	前一个动作
		DWORD					_PrevState;
	};

}	//	namespace GPL