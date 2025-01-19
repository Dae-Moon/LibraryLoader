#include "pch.h"

pmodule_t _get_module_information(LPCSTR lpModuleName)
{
	const auto hModule = GetModuleHandle(lpModuleName);

	if (!hModule)
		return 0;

	MEMORY_BASIC_INFORMATION MBI;

	if (!VirtualQuery(hModule, &MBI, sizeof(MBI)))
		return 0;

	if (MBI.State != MEM_COMMIT)
		return 0;

	if (!MBI.AllocationBase)
		return 0;

	PIMAGE_DOS_HEADER pIDH = (PIMAGE_DOS_HEADER)MBI.AllocationBase;
	PIMAGE_NT_HEADERS pIND = (PIMAGE_NT_HEADERS)((unsigned)pIDH + (unsigned)pIDH->e_lfanew);

	if (pIND->Signature != IMAGE_NT_SIGNATURE)
		return 0;

	auto pMI = new module_t;

	pMI->handle = hModule;
	GetModuleFileName(hModule, pMI->file_name, MAX_PATH);
	pMI->base = MBI.AllocationBase;
	pMI->size = (unsigned)pIND->OptionalHeader.SizeOfImage;
	pMI->end = (void*)((unsigned)pMI->base + pMI->size - 1);

	return pMI;
}

bool _far_pointer(void* ptr, void* start, void* end)
{
	if (start > end)
	{
		void* safe = end;
		start = end;
		end = safe;
	}

	return !(ptr > start && ptr < end);
}

void* _aob_scan(void* sig, void* start, void* end, size_t size)
{
	//const auto size = sizeof(sig) / sizeof(*(void**)sig);

	if (!size)
		size = strlen((char*)sig);

	if (!size)
		return 0;

	if (start > end)
	{
		void* safe = end;
		start = end;
		end = safe;
	}

	for (void* ptr = start; ptr < (void*)((unsigned)end - size); ptr = (void*)((unsigned)ptr + 1))
	{
		bool found = true;

		for (auto i = 0; i < size; i++)
		{
			auto ss = ((PBYTE)sig)[i];
			if (ss != 0xFF && ss != *(PBYTE)((unsigned)ptr + i))
			{
				found = false;
				break;
			}
		}

		if (found)
			return ptr;
	}

	return 0;
}

void* _find_reference(void* ptr, void* start, void* end)
{
	char szPattern[] = { 0x68, 0x00, 0x00, 0x00, 0x00, 0x00 };
	*(void**)&szPattern[1] = ptr;
	return _aob_scan((void*)szPattern, start, end);
}