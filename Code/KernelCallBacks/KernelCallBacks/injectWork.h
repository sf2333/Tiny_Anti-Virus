VOID UnicodeToChar(PUNICODE_STRING uniSource, CHAR *szDest);

VOID UnloadDriver(PDRIVER_OBJECT DriverObject);

NTSTATUS startInject(char *dllName,char *apiName);

VOID stopInject();