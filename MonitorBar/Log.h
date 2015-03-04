#pragma once

#ifdef _DEBUG
#define LOGOUT(message) CLog((const char*)(message))
#else
#define LOGOUT(message)
#endif

#define LOGERR(function,errCode) CLog((const char*)__FUNCTION__,__LINE__,(function),(errCode))

class CLog
{
public:
	CLog(const char*);
	CLog(const char*,unsigned int,const char*,unsigned long);
	~CLog( ){}
};

