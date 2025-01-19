#ifndef MEMORY_H
#define MEMORY_H

typedef struct module_t
{
	HMODULE handle;
	char file_name[MAX_PATH];
	void* base;
	unsigned size;
	void* end;
} *pmodule_t;

extern pmodule_t _get_module_information(LPCSTR lpModuleName);
extern bool _far_pointer(void* ptr, void* start, void* end);
extern void* _aob_scan(void* sig, void* start, void* end, size_t size = 0);
extern void* _find_reference(void* ptr, void* start, void* end);

#endif
