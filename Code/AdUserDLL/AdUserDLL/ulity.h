#include <stdio.h>
#include <string.h>

#define ExportDLL extern "C" __declspec(dllexport) 

//��ӡdebug��Ϣ
//OUTINFO_0_PARAM��ʾ������ַ�����OUTINFO_1_PARAM��ʾ����Я��һ���������Դ�����2��3
#define OUTINFO_0_PARAM(fmt) {CHAR sOut[256];CHAR sfmt[100];sprintf_s(sfmt,"%s%s","INFO--",fmt);sprintf_s(sOut,(sfmt));OutputDebugStringA(sOut);}    
#define OUTINFO_1_PARAM(fmt,var1) {CHAR sOut[256];CHAR sfmt[100];sprintf_s(sfmt,"%s%s","INFO--",fmt);sprintf_s(sOut,(sfmt),var1);OutputDebugStringA(sOut);}    
#define OUTINFO_2_PARAM(fmt,var1,var2) {CHAR sOut[256];CHAR sfmt[100];sprintf_s(sfmt,"%s%s","INFO--",fmt);sprintf_s(sOut,(sfmt),var1, var2);OutputDebugStringA(sOut);}    

struct YR_RULE
{
	int flags;

	// Number of atoms generated for this rule.
	int num_atoms;

	// Number of strings that must match for this rule to have some possibility
	// to match.
	int required_strings;

	// Just for padding.
	int unused;

	const char* identifier;
	const char* tags;
	void* metas;
	void* trings;
	void* ns;
};

