#include "public.h"
VOID addIAT (
			 IN HANDLE    ProcessId, // where image is mapped
			 IN PVOID    BaseImage,
			 IN char*	DllName,
			 IN char*	FunctionName
);
VOID NotifyRoutine(
				   IN PUNICODE_STRING    FullImageName,
				   IN HANDLE    ProcessId, // where image is mapped
				   IN PIMAGE_INFO    ImageInfo
				   );