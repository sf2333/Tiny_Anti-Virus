typedef struct _SERVICEDESCRIPTORTABLE{
	PULONG ServiceTable;
	PULONG CounterTable;
	ULONG TableSize;
	PUCHAR ArgumentTable;
}SERVICEDESCRIPTORTABLE,*PSERVICEDESCRIPTORTABLE ; 

typedef
NTSTATUS
(NTAPI *
MyZwProtectVirtualMemory)(
					   IN HANDLE 
					   ProcessHandle,
					   IN OUT PVOID 
					   *BaseAddress,
					   IN OUT PSIZE_T
					   ProtectSize,
					   IN ULONG 
					   NewProtect,
					   OUT PULONG 
					   OldProtect
					   );



typedef
NTSTATUS
(NTAPI *
MyZwWriteVirtualMemory)(
					 IN HANDLE 
					 ProcessHandle,
					 IN PVOID 
					 BaseAddress,
					 IN PVOID 
					 Buffer,
					 IN ULONG 
					 BufferLength,
					 OUT PULONG 
					 ReturnLength OPTIONAL
					 );