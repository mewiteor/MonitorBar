#pragma once
#include<string>
#include<tchar.h>

class IMonitor
{
public:
	virtual ~IMonitor( ){}
	virtual const std::basic_string<TCHAR> ToString( )const = 0;
	virtual const std::wstring ToLongString( )const = 0;
	virtual const double GetValue( )const = 0;
	virtual bool Init( ) = 0;
	virtual void Update() = 0;
	virtual void Reset( ) = 0;
};