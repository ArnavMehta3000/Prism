#include "Log.h"
#include <Windows.h>

namespace Prism
{
	static bool s_isLogInitialized = false;

	void Log::Init()
	{
		if (s_isLogInitialized)  // Already initialized
		{
			return;
		}

		if (HANDLE hOut = ::GetStdHandle(STD_OUTPUT_HANDLE))
		{
			DWORD dwMode = 0;
			::GetConsoleMode(hOut, &dwMode);
			dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			::SetConsoleMode(hOut, dwMode);
		}
		s_isLogInitialized = true;
	}
}
