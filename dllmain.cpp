#include "headers.h"

static cl_enginefuncs_s* pEngFuncs;
static pmodule_t hw, gameui;
static PColor24 Console_TextColor;
static HMODULE handle;

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

void ConsolePrintColor(byte R, byte G, byte B, const char *fmt, ...)
{
	va_list va_alist;
	char buf[256];
	va_start(va_alist, fmt);
	_vsnprintf(buf, sizeof(buf), fmt, va_alist);
	va_end(va_alist);
	TColor24 DefaultColor; PColor24 Ptr; Ptr = Console_TextColor; DefaultColor = *Ptr; Ptr->R = R; Ptr->G = G; Ptr->B = B; pEngFuncs->Con_Printf(buf); *Ptr = DefaultColor;
}


static void LoadLibraryFunc(void)
{
	if (pEngFuncs->Cmd_Argc() < 2)
	{
		ConsolePrintColor(255, 0, 0, "ERROR: the path is not specified.\n");
		return;
	}

	LoadLibrary(pEngFuncs->Cmd_Argv(1));
}

static void UnloadLibraryFunc(void)
{
	if (pEngFuncs->Cmd_Argc() < 2)
	{
		ConsolePrintColor(255, 0, 0, "ERROR: the path is not specified.\n");
		return;
	}

	const auto path = std::string(pEngFuncs->Cmd_Argv(1));
	const auto idx = path.find_last_of("/\\");

	if (idx == std::string::npos)
	{
		con_nprint_s con;
		con.index = 0;
		con.time_to_live = 5.f;
		con.color[0] = 1.f;
		con.color[1] = con.color[2] = 0.f;

		pEngFuncs->Con_NXPrintf(&con, "ERROR: Wrong path");
		return;
	}

	FreeLibrary(GetModuleHandle(path.substr(idx + 1).c_str()));
}

static void Unload()
{
	auto cmd = pEngFuncs->pfnGetCmdList();
	
	cmd_t *v0;
	cmd_t *v1;
	cmd_s *v2;

	v0 = 0;
	v1 = cmd;
	if (cmd)
	{
		while (true)
		{
			v2 = v1->next;
			if (v1->function == (xcommand_t)LoadLibraryFunc ||
				v1->function == (xcommand_t)UnloadLibraryFunc ||
				v1->function == (xcommand_t)Unload)
			{
				free(v1);
				if (!v2)
					break;
			}
			else
			{
				v1->next = v0;
				v0 = v1;
				if (!v2)
					break;
			}
			v1 = v2;
		}
	}
	cmd = v0;

	FreeLibrary(handle);
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
	pEngFuncs->pfnAddCommand("unload_loader", Unload);
}

std::string GetPath()
{
	char path[MAX_PATH];
	HMODULE hm = NULL;

	if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCSTR)&Init, &hm) == 0)
	{
		int ret = GetLastError();
		fprintf(stderr, "GetModuleHandle failed, error = %d\n", ret);
		// Return or however you want to handle an error.
	}
	if (GetModuleFileName(hm, path, sizeof(path)) == 0)
	{
		int ret = GetLastError();
		fprintf(stderr, "GetModuleFileName failed, error = %d\n", ret);
		// Return or however you want to handle an error.
	}

	// The path variable should now contain the full filepath for this DLL.
	return path;
}

static void LoadCfg()
{
	std::string fileName(GetPath());
	fileName.append("\\");
	fileName.append(PROJECT_NAME);
	fileName.append(".cfg");

	if (std::filesystem::exists(fileName))
	{
		std::ifstream it(fileName);

		if (it.is_open())
		{
			std::string line;

			while (std::getline(it, line))
			{
				const auto idx = line.rfind(".dll");
				if (idx == line.length() - 4)
					LoadLibrary(line.c_str());
			}
		}
	}
	else
		std::ofstream(fileName).close();
}


BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
			return TRUE;

		handle = hModule;
		DisableThreadLibraryCalls(hModule);

		LoadCfg();
		Init();
	}

	return TRUE;
}