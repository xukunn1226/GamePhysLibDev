//------------------------------------------------------------------------
//	@brief	����״̬�������ࣩ
//	@author	chenpu
//	@date	2013-2-3
//------------------------------------------------------------------------
#include "PhysXSupport.h"
#include "GameObjectState.h"
#include "GameObjectFSM.h"

namespace GPL {

	bool GameObjectFSM::Init(FSMActor* owner)
	{
		if ( NULL == owner )
			return false;

		_Actor = owner;
		return true;
	}
	
	void GameObjectFSM::Update(float DeltaTime)
	{
		if ( NULL == _Actor )
			return;

		//if ( NULL == _ActiveState )
		//{
		//	if ( NULL != _DefaultState )
		//	{
		//		GameObjectEvent defaultArgs;
		//		_DefaultState->OnEnter(defaultArgs);
		//	}
		//}

		if ( _ActiveState )
			_ActiveState->Update(DeltaTime);
	}

	bool GameObjectFSM::FlushEvent(GameObjectEvent& args)
	{
		//	�Ƿ���ֹ��ǰ״̬
		if ( NULL != _ActiveState )
		{
			if ( !_ActiveState->OnEvent(args) )
				return false;

			_ActiveState->OnLeave(args);
		}

		//	����Ŀ��״̬
		if ( NULL != args._State )
		{
			args._State->OnEnter(args);
		}
		else 
		{
			if ( NULL != _DefaultState )
			{
				GameObjectEvent defaultArgs;
				_DefaultState->OnEnter(defaultArgs);
			}
		}

		return true;
	}

	void GameObjectFSM::OnStateActivated(GameObjectState* state)
	{
		_ActiveState = state;
	}

	void GameObjectFSM::OnStateDeactived(GameObjectState* state)
	{
		if ( _ActiveState == state )
			_ActiveState = NULL;
	}
}