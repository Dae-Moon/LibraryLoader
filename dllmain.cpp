#include "headers.h"

std::map<std::string, HMODULE> loaded_modules;

#pragma region Offsets

static pmodule_t hw, gameui;
static HMODULE handle;
static cl_enginefuncs_s* pEngFuncs;
static PColor24 Console_TextColor;

void* Engine()
{
	void* sig = (void*)"\x68\xFF\xFF\xFF\xFF\xFF\x15\xFF\xFF\xFF\xFF\x68\xFF\xFF\xFF\xFF\xE8\xFF\xFF\xFF\xFF\x83\xC4\x1C";
	void* ptr = _aob_scan(sig, hw->base, hw->end);

	if (_far_pointer(ptr, hw->base, hw->end))
		return 0;

	return *(void**)((unsigned)ptr + 0x1);
}

void* GameConsole()
{
	void* PatternAddress = _aob_scan((void*)"GameConsole003", gameui->base, gameui->end);
	void* ReferenAddress = _find_reference(PatternAddress, gameui->base, gameui->end);

	if (_far_pointer(ReferenAddress, gameui->base, gameui->end))
		return 0;

	return *(void**)((unsigned)ReferenAddress + 0x21);
}

void ConsoleColorInitialize()
{
	auto gameConsole = (unsigned)GameConsole();

	if (gameConsole)
	{
		DWORD Panel = (*(PDWORD)(gameConsole + 8) - gameConsole);

		Console_TextColor = PColor24(Panel + gameConsole + 288 + sizeof(DWORD));

		if (*(PDWORD)(DWORD(Console_TextColor) + 8) != 0)
		{
			Console_TextColor = PColor24(Panel + gameConsole + 288 + (sizeof(DWORD) * 2));
		}
	}
}

#pragma endregion

#pragma region Main

void ConsolePrintColor(byte R, byte G, byte B, const char *fmt, ...)
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

static void LoadLibraryFunc(void)
{
	if (pEngFuncs->Cmd_Argc() < 2)
	{
		ConsolePrintColor(-1, 255, 0, 0, "ERROR: the path is not specified.\n");
		return;
	}

	std::string file_name(pEngFuncs->Cmd_Argv(1));
	const auto handle = LoadLibrary(file_name.c_str());

	if (handle != INVALID_HANDLE_VALUE)
		loaded_modules.insert({ file_name, handle });
}

static void UnloadLibraryFunc(void)
{
	if (pEngFuncs->Cmd_Argc() < 2)
	{
		ConsolePrintColor(-1, 255, 0, 0, "ERROR: the path is not specified.\n");
		return;
	}

	const auto path = std::string(pEngFuncs->Cmd_Argv(1));

	for (const auto& pair : loaded_modules)
	{
		if (path.find(pair.first) != std::string::npos)
		{
			FreeLibrary(pair.second);
			loaded_modules.erase(pair.first);
			break;
		}
	}
}

static void LogModules(void)
{
	Color24 color = *Console_TextColor;

	if (pEngFuncs->Cmd_Argc() < 2)
	{
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());

		if (hSnapshot != INVALID_HANDLE_VALUE)
		{
			MODULEENTRY32 ModuleEntry32 = { 0 };
			ModuleEntry32.dwSize = sizeof(MODULEENTRY32);
			if (Module32First(hSnapshot, &ModuleEntry32))
			{
				do
				{
					for (const auto& pair : loaded_modules)
					{
						if (pair.first.find(ModuleEntry32.szModule) != std::string::npos)
							color.R = color.G = color.B = 255;
					}
					ConsolePrintColor(color.R, color.G, color.B, "%s\n", ModuleEntry32.szModule);

				} while (Module32Next(hSnapshot, &ModuleEntry32));
			}
			CloseHandle(hSnapshot);
		}

		return;
	}

	const auto pModule = _get_module_information(pEngFuncs->Cmd_Argv(1));

	if (!pModule)
	{
		ConsolePrintColor(255, 0, 0, "ERROR: module not found.\n");
		return;
	}

	ConsolePrintColor(color.R, color.G, color.B, "Path: ");
	ConsolePrintColor(0, 0, 255, "%s\n", pModule->file_name);

	ConsolePrintColor(color.R, color.G, color.B, "Allocation base: ");
	ConsolePrintColor(0, 255, 0, "%p\n", pModule->base);

	ConsolePrintColor(color.R, color.G, color.B, "Size of image: ");
	ConsolePrintColor(255, 255, 255, "%u\n", pModule->size);
}


static void Init()
{
	hw = _get_module_information("hw.dll");

	if (!hw)
		return;

	gameui = _get_module_information("gameui.dll");

	if (!gameui)
		return;

	pEngFuncs = (cl_enginefuncs_s*)Engine();

	if (!pEngFuncs)
		return;

	ConsoleColorInitialize();

	if (!Console_TextColor)
		return;

	pEngFuncs->pfnAddCommand("load_library", LoadLibraryFunc);
	pEngFuncs->pfnAddCommand("unload_library", UnloadLibraryFunc);
	pEngFuncs->pfnAddCommand("log_modules", LogModules);
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
			return TRUE;
		handle = hModule;
		DisableThreadLibraryCalls(hModule);
		Init();
	}

	return TRUE;
}

#pragma endregion