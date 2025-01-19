#include "pch.h"

std::map<std::string, HMODULE> loaded_modules;

pmodule_t hw, gameui;
HMODULE handle;
cl_enginefuncs_s* pEngFuncs;
int iBuildNum = 0;

#pragma region Offsets

static void* Engine()
{
	void* sig = iBuildNum <= 4554 ? (void*)"\x68\xFF\xFF\xFF\xFF\xFF\x15\xFF\xFF\xFF\xFF\x68\xFF\xFF\xFF\xFF\xE8\xFF\xFF\xFF\xFF\x83\xC4\x1C" : (void*)"\x68\xFF\xFF\xFF\xFF\xFF\x15\xFF\xFF\xFF\xFF\x68\xFF\xFF\xFF\xFF\xE8\xFF\xFF\xFF\xFF\x83\xC4\x1C";
	void* ptr = _aob_scan(sig, hw->base, hw->end);

	if (_far_pointer(ptr, hw->base, hw->end))
		return 0;

	return *(void**)((unsigned)ptr + 0x1);
}

static void* GameConsole()
{
	void* PatternAddress = _aob_scan((void*)"GameConsole003", gameui->base, gameui->end);
	void* ReferenAddress = _find_reference(PatternAddress, gameui->base, gameui->end);

	if (_far_pointer(ReferenAddress, gameui->base, gameui->end))
		return 0;

	return *(void**)((unsigned)ReferenAddress + 0x21);
}

static void* LECT()
{
	void* sig = (void*)"Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Sep\0Oct\0Nov\0Dec";
	void* ptr = _aob_scan(sig, hw->base, hw->end, 48);

	if (_far_pointer(ptr, hw->base, hw->end))
		return 0;

	uintptr_t* exe_build_string_end_ptr = (uintptr_t*)((uint8_t*)ptr - 1);
	uintptr_t* exe_build_string_start_ptr = nullptr;

	static const char* mon[12] =
	{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	// walk down the binary until we come across any of the months.
	char* date_string_final = nullptr;
	for (int i = 0; i < 11; i++)
	{
		char* b = (char*)exe_build_string_end_ptr;

		bool found = false;
		int k = 0;
		while (true)
		{
			if (k > 20)
			{
				break;
			}

			if (!_strnicmp(b, mon[i], 3))
			{
				found = true;
				break;
			}

			k++;
			b--;
		}

		if (found)
		{
			date_string_final = b;
			break;
		}
	}

	if (!date_string_final || !date_string_final[0])
	{
		// "Fatal error while parsing the timestamp string!"
		// "Couldn't get engine compile date string!"
		return 0;
	}

	return (void*)date_string_final;
}

#pragma endregion

#pragma region Main

static void LoadLibraryFunc(void)
{
	if (pEngFuncs->Cmd_Argc() < 2)
	{
		Console_PrintError("ERROR: the path is not specified.\n");
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
		Console_PrintError("ERROR: the path is not specified.\n");
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
					Console_PrintColor(color.R, color.G, color.B, "%s\n", ModuleEntry32.szModule);

				} while (Module32Next(hSnapshot, &ModuleEntry32));
			}
			CloseHandle(hSnapshot);
		}

		return;
	}

	const auto pModule = _get_module_information(pEngFuncs->Cmd_Argv(1));

	if (!pModule)
	{
		Console_PrintError("ERROR: module not found.\n");
		return;
	}

	Console_PrintColor(color.R, color.G, color.B, "Path: ");
	Console_PrintColor(0, 0, 255, "%s\n", pModule->file_name);

	Console_PrintColor(color.R, color.G, color.B, "Allocation base: ");
	Console_PrintColor(0, 255, 0, "%p\n", pModule->base);

	Console_PrintColor(color.R, color.G, color.B, "Size of image: ");
	Console_PrintColor(255, 255, 255, "%u\n", pModule->size);
}

static bool Init()
{
	//if (!HideDll((HINSTANCE)handle))
	//	return false;

	if (!(hw = _get_module_information("hw.dll")))
		return false;

	if (!(gameui = _get_module_information("gameui.dll")))
		return false;

	auto date = (const char*)LECT();

	if (!date || !date[0])
		return false;

	if (!(iBuildNum = build_number(date)))
		return false;

	if (!(pEngFuncs = (cl_enginefuncs_s*)Engine()))
		return false;

	if (!Console_InitializeColor(GameConsole()))
		return false;

	if (!Console_TextColor)
		return false;

	pEngFuncs->pfnAddCommand("load_library", LoadLibraryFunc);
	pEngFuncs->pfnAddCommand("unload_library", UnloadLibraryFunc);
	pEngFuncs->pfnAddCommand("log_modules", LogModules);

	return true;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
			return TRUE;

		handle = hModule;
		DisableThreadLibraryCalls(hModule);
		
		if (!Init())
			return FALSE;
	}

	return TRUE;
}

#pragma endregion
