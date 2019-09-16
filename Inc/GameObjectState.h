//------------------------------------------------------------------------
//	@brief	����״̬��
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

		//	@brief	��״̬��
		void			PostBinding	(GameObjectFSM* fsm)	{ _Host = fsm; }

		//	@brief	�¼�����
		virtual bool	OnEvent		(GameObjectEvent& args)	{ return true; }

		//	@brief	����״̬��Ӧ
		virtual void	OnEnter		(GameObjectEvent& args);

		//	@brief	�뿪״̬��Ӧ
		virtual void	OnLeave		(GameObjectEvent& args);

		//	@brief	״̬����
		virtual void	Update		(float DeltaTime);

		//	@brief	��ȡ״̬ö��ֵ
		inline NxU8		GetEnumValue() const	{ return _Value; }

		//	@brief	����ۻ�ʱ��
		inline float	GetAccumTime() const	{ return _AccumTime; }

	protected:
		//	@brief	����״̬��
		GameObjectFSM*	_Host;

		//	@brief	״ֵ̬
		NxU8			_Value;

		//	@brief	�ۻ�ʱ��
		float			_AccumTime;
	};

}	//	namespace GPL