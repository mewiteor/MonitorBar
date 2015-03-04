#include<Windows.h>
#include<tchar.h>
#include"Win.h"

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int nCmdShow)
{
	return CWin(hInstance, nCmdShow).MessageLoop( );
}
