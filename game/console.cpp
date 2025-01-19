#include "pch.h"

PColor24 Console_TextColor;

bool Console_InitializeColor(void* gameConsole)
{
	auto pGameConsole = (unsigned)gameConsole;

	if (gameConsole)
	{
		DWORD Panel = (*(PDWORD)(pGameConsole + 8) - pGameConsole);

		Console_TextColor = PColor24(Panel + pGameConsole + 288 + sizeof(DWORD));

		if (*(PDWORD)(DWORD(Console_TextColor) + 8) != 0)
		{
			Console_TextColor = PColor24(Panel + pGameConsole + 288 + (sizeof(DWORD) * 2));
			return true;
		}
	}

	return false;
}

void Console_PrintColor(byte R, byte G, byte B, const char* fmt, ...)
{
	va_list va_alist;
	char buf[256];
	va_start(va_alist, fmt);
	_vsnprintf(buf, sizeof(buf), fmt, va_alist);
	va_end(va_alist);
	TColor24 DefaultColor;
	PColor24 Ptr = Console_TextColor;
	DefaultColor = *Ptr; Ptr->R = R; Ptr->G = G; Ptr->B = B;
	pEngFuncs->Con_Printf(buf);
	*Ptr = DefaultColor;
}

void Console_PrintSuccess(const char* fmt, ...)
{
	Console_PrintColor(0, 255, 0, fmt);
}

void Console_PrintLog(const char* fmt, ...)
{
	Console_PrintColor(255, 255, 255, fmt);
}

void Console_PrintWarning(const char* fmt, ...)
{
	Console_PrintColor(255, 255, 0, fmt);
}

void Console_PrintError(const char* fmt, ...)
{
	Console_PrintColor(255, 0, 0, fmt);
}
