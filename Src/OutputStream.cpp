#include "..\Inc\PhysXSupport.h"
#include <tchar.h>

namespace GPL
{

#define MAX_SPRINTF 1024
#define GET_VARARGS_RESULT(msg,msgsize,len,lastarg,fmt,result) { va_list ap; va_start(ap,lastarg); result = DoGetVarArgs(msg,msgsize,len,fmt,ap); }
#define GET_VARARGS_RESULT_ANSI(msg,msgsize,len,lastarg,fmt,result) { va_list ap; va_start(ap,lastarg); result = DoGetVarArgsAnsi(msg,msgsize,len,fmt,ap); }

INT DoGetVarArgs( TCHAR* Dest, SIZE_T DestSize, INT Count, const TCHAR*& Fmt, va_list ArgPtr)
{
	INT Result = _vsntprintf_s(Dest, DestSize, Count, Fmt, ArgPtr);
	va_end(ArgPtr);
	return Result;
}

INT DoGetVarArgsAnsi( char* Dest, SIZE_T DestSize, INT Count, const char*& Fmt, va_list ArgPtr )
{
	INT Result = _vsnprintf_s(Dest, DestSize, Count, Fmt, ArgPtr);
	va_end(ArgPtr);
	return Result;
}

void gplPrintf(const TCHAR* Fmt, ...)
{
	if( GOutputStream != NULL )
	{
		TCHAR Message[MAX_SPRINTF] = TEXT("");
		INT Ret;
		GET_VARARGS_RESULT( Message, MAX_SPRINTF, MAX_SPRINTF - 1, Fmt, Fmt, Ret );
		GOutputStream->Printf(Message);
	}
}

void gplPrintfANSI(const char* Fmt, ...)
{
	if( GOutputStream != NULL )
	{
		char Message[MAX_SPRINTF] = "";
		INT Ret;
		GET_VARARGS_RESULT_ANSI( Message, MAX_SPRINTF, MAX_SPRINTF - 1, Fmt, Fmt, Ret );
		GOutputStream->PrintfANSI(Message);
	}
}

}