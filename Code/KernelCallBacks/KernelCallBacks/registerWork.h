void GetRegisterPath(PUNICODE_STRING pRegistryPath, PVOID pRegistryObject);

BOOLEAN Compare(UNICODE_STRING ustrRegPath);

void ShowProcessName();

NTSTATUS RegistryCallback(_In_ PVOID CallbackContext, _In_opt_ PVOID Argument1, _In_opt_ PVOID Argument2);


//CmRegisterCallback(RegistryCallback, NULL, &cookie);
//CmUnRegisterCallback(cookie);


