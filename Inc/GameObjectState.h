//------------------------------------------------------------------------
//	@brief	对象状态类
//	@author	chenpu
//	@date	2013-2-3
//------------------------------------------------------------------------
#include "GameObjectFSM.h"

#pragma once

namespace GPL {

	class GameObjectState
	{
	public:
		GameObjectState(NxU8 _enum) 
			: _Host(NULL)
			, _Value(_enum)	
		{
		}
		virtual ~GameObjectState() {}

		//	@brief	绑定状态机
		void			PostBinding	(GameObjectFSM* fsm)	{ _Host = fsm; }

		//	@brief	事件处理
		virtual bool	OnEvent		(GameObjectEvent& args)	{ return true; }

		//	@brief	进入状态响应
		virtual void	OnEnter		(GameObjectEvent& args);

		//	@brief	离开状态响应
		virtual void	OnLeave		(GameObjectEvent& args);

		//	@brief	状态更新
		virtual void	Update		(float DeltaTime);

		//	@brief	获取状态枚举值
		inline NxU8		GetEnumValue() const	{ return _Value; }

		//	@brief	获得累积时间
		inline float	GetAccumTime() const	{ return _AccumTime; }

	protected:
		//	@brief	所属状态机
		GameObjectFSM*	_Host;

		//	@brief	状态值
		NxU8			_Value;

		//	@brief	累积时间
		float			_AccumTime;
	};

}	//	namespace GPL