//------------------------------------------------------------------------
//	@brief	对象状态机（基类）
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

		//	@brief	事件处理
		virtual bool			FlushEvent		(GameObjectEvent& args);

		//	@brief	状态激活处理
		void					OnStateActivated(GameObjectState* state);

		//	@brief	状态失活处理
		void					OnStateDeactived(GameObjectState* state);

		//	@brief	获得当前活动状态
		inline GameObjectState*	GetActiveState	()	{ return _ActiveState; }

		//	@brief	设置默认状态
		inline void				SetDefaultState	(GameObjectState* state)	{ _DefaultState = state; }

		//	@brief	获得执行对象
		inline FSMActor*		GetActor		()	{ return _Actor; }

	protected:
		//	@brief	状态机执行对象
		FSMActor*			_Actor;

		//	@brief	活动中的状态
		GameObjectState*	_ActiveState;

		//	@brief	默认状态
		GameObjectState*	_DefaultState;
	};

	//------------------------------------------------------------------------
	//	@brief	派生类状态对象声明
	//	@param	[ObjectName]	状态机所属对象
	//	@param	[StateEnum]		对象状态枚举
	//------------------------------------------------------------------------
#define DECLARE_FSM_STATE(ObjectName, StateEnum)	\
	class ObjectName##State##StateEnum : public GameObjectState { \
	public:\
	ObjectName##State##StateEnum() : GameObjectState(StateEnum) {}	\
	virtual ~ObjectName##State##StateEnum() {}	\
	};

	//------------------------------------------------------------------------
	//	@brief	派生类状态事件声明
	//	@param	[ObjectEnum]	对象枚举
	//	@param	[StateEnum]		对象状态枚举
	//	@param	[Tx]			自定义参数类型
	//	@param	[Px]			自定义参数
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