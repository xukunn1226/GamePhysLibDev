//------------------------------------------------------------------------
//	@brief	µ¥¼þÄ£°å
//	@author	chenpu
//	@date	2013-2-24
//------------------------------------------------------------------------
#pragma once

namespace GPL
{
	template<class T>
	class Singleton
	{
	public:
		static T&	GetSingletonRef()
		{
			if ( NULL == _instance )
				_instance = new T();

			return *_instance;
		}

		static void Finalize()	
		{
			SAFE_DELETE(_instance);
		}

	protected:
		Singleton()	{}
		virtual ~Singleton()	{}

		static T*	_instance;
	};

	template<class T>
	T* Singleton<T>::_instance = NULL;

}	//	namespace GPL