typedef struct _UNDOCUMENT_DATA
{
	int PreviousMode;
	PVOID NtProtectVirtualMemory;
	PVOID NtQueryVirtualMemory;

}UMDOCUMENT_DATA,*PUMDOCUMENT_DATA;

extern UMDOCUMENT_DATA g_undocument_data;
