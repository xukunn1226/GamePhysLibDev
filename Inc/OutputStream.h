#ifndef _PHYS_OUTPUTSTREAM_H_
#define _PHYS_OUTPUTSTREAM_H_

namespace GPL
{
class PhysUserOutputStream
{
public:
	virtual void Printf(const TCHAR* Message) = 0;
	virtual void PrintfANSI(const char* Message) = 0;
};

void gplPrintf(const TCHAR* Fmt, ...);
void gplPrintfANSI(const char* Fmt, ...);

class NOutputStream : public NxUserOutputStream
{
public:
	void reportError(NxErrorCode e, const char* message, const char* file, int line)
	{
		gplDebugfANSI("[GPL]: reportError (%d) in file %s, line %d: %s", e, file, line, message);
	}

	NxAssertResponse reportAssertViolation(const char* message, const char* file, int line)
	{
		gplDebugfANSI("[GPL]: reportAssertViolation in file %s, line %d: %s", file, line, message);
		return NX_AR_BREAKPOINT;
	}

	void print(const char* message)
	{
		gplDebugfANSI("[GPL]: print message %s", message);
	}
};

}
#endif