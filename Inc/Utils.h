//------------------------------------------------------------------------
//	@brief	Utils
//	@author	chenpu
//	@date	2013-2-24
//------------------------------------------------------------------------
#pragma once

namespace GPL
{
#define SAFE_DELETE(p)			if ( NULL != p )	{ delete p; p = NULL; }
#define SAFE_DELETE_ARRAY(p)	if ( NULL != p )	{ delete []p; p = NULL; }

#define MASK(p)					(1 << p)
#define RESET_STATE(state)		(state = 0)
#define ENABLE_STATE(state, p)	(state |= MASK(p))
#define DISABLE_STATE(state, p)	(state &= (~MASK(p)))
#define CHECK_STATE(state, p)	(0 != (state & MASK(p)))
#define FLAG_TEST(state, flag)	(0 != (state & flag))
}	//	namespace GPL