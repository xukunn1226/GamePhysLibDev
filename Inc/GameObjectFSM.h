//------------------------------------------------------------------------
//	@brief	����״̬�������ࣩ
//	@author	chenpu
//	@date	2013-2-3
//------------------------------------------------------------------------
#include "GameObjects.h"

#pragma once

namespace GPL {

	class GameObjectState;

	enum GameObjectEnum
	{
		Vehicle = 0,
		Aircraft,
		Projectile
	};

	typedef PhysGameObject	FSMActor;

	struct GameObjectEvent {
		unsigned char		_Category;
		GameObjectState*	_State;
		void*				_Userdata;
	};

	class GameObjectFSM
	{
	public:
		GameObjectFSM() 
			: _Actor(NULL)
			, _ActiveState(NULL)
			, _DefaultState(NULL)
		{}

		virtual ~GameObjectFSM()	{}

		virtual bool			Init			(FSMActor* owner);

		virtual void			Update			(float DeltaTime);

		//	@brief	�¼�����
		virtual bool			FlushEvent		(GameObjectEvent& args);

		//	@brief	״̬�����
		void					OnStateActivated(GameObjectState* state);

		//	@brief	״̬ʧ���
		void					OnStateDeactived(GameObjectState* state);

		//	@brief	��õ�ǰ�״̬
		inline GameObjectState*	GetActiveState	()	{ return _ActiveState; }

		//	@brief	����Ĭ��״̬
		inline void				SetDefaultState	(GameObjectState* state)	{ _DefaultState = state; }

		//	@brief	���ִ�ж���
		inline FSMActor*		GetActor		()	{ return _Actor; }

	protected:
		//	@brief	״̬��ִ�ж���
		FSMActor*			_Actor;

		//	@brief	��е�״̬
		GameObjectState*	_ActiveState;

		//	@brief	Ĭ��״̬
		GameObjectState*	_DefaultState;
	};

	//------------------------------------------------------------------------
	//	@brief	������״̬��������
	//	@param	[ObjectName]	״̬����������
	//	@param	[StateEnum]		����״̬ö��
	//------------------------------------------------------------------------
#define DECLARE_FSM_STATE(ObjectName, StateEnum)	\
	class ObjectName##State##StateEnum : public GameObjectState { \
	public:\
	ObjectName##State##StateEnum() : GameObjectState(StateEnum) {}	\
	virtual ~ObjectName##State##StateEnum() {}	\
	};

	//------------------------------------------------------------------------
	//	@brief	������״̬�¼�����
	//	@param	[ObjectEnum]	����ö��
	//	@param	[StateEnum]		����״̬ö��
	//	@param	[Tx]			�Զ����������
	//	@param	[Px]			�Զ������
	//------------------------------------------------------------------------
#define DECLARE_FSM_EVENT(ObjectEnum, StateEnum)	\
	struct ObjectEnum##Event##StateEnum : public GameObjectEvent {	\
	ObjectEnum##Event##StateEnum()	\
	{	_Category = ObjectEnum;	}	\
	};

#define DECLARE_FSM_EVENT_X1(ObjectEnum, StateEnum, T1, P1)	\
	struct ObjectEnum##Event##StateEnum : public GameObjectEvent { \
	T1 P1;\
	ObjectEnum##Event##StateEnum()	\
	{	_Category = ObjectEnum;	}	\
	};

#define DECLARE_FSM_EVENT_X2(ObjectEnum, StateEnum, T1, P1, T2, P2)	\
	struct ObjectEnum##Event##StateEnum : public GameObjectEvent { \
	T1 P1; T2 P2;\
	ObjectEnum##Event##StateEnum()	\
	{	_Category = ObjectEnum;	}	\
	};

#define DECLARE_FSM_EVENT_X3(ObjectEnum, StateEnum, T1, P1, T2, P2, T3, P3)	\
	struct ObjectEnum##Event##StateEnum : public GameObjectEvent { \
	T1 P1; T2 P2; T3 P3;\
	ObjectEnum##Event##StateEnum()	\
	{	_Category = ObjectEnum;	}	\
	};

#define DECLARE_FSM_EVENT_X4(ObjectEnum, StateEnum, T1, P1, T2, P2, T3, P3, T4, P4)	\
	struct ObjectEnum##Event##StateEnum : public GameObjectEvent { \
	T1 P1; T2 P2; T3 P3; T4 P4;\
	ObjectEnum##Event##StateEnum()	\
	{	_Category = ObjectEnum;	}	\
	};

}	//	namespace GPL