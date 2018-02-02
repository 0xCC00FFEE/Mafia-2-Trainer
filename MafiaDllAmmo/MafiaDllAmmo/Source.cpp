#include<Windows.h>

DWORD dwAmmoJMPBackAdd;
DWORD dwHealthJMPBackAdd;

BOOL HookMe(void* HookFunc, void* myFunc, int len)
{
	if (len < 5)
		return false;

	DWORD dwOldProtection;
	VirtualProtect(HookFunc, len, PAGE_EXECUTE_READWRITE, &dwOldProtection);

	memset(HookFunc, 0x90, len);

	DWORD dwRelativeAdd = ((DWORD)myFunc - (DWORD)HookFunc) - 5;

	*(BYTE*)HookFunc = 0xE9;
	*(DWORD*)((DWORD)HookFunc + 1) = dwRelativeAdd;

	VirtualProtect(HookFunc, len, dwOldProtection, NULL);

	return true;
}

void __declspec(naked) AmmoHook()
{
	__asm
	{
		mov eax, DWORD PTR DS:[ecx]
		jmp [dwAmmoJMPBackAdd]
	}
}

void __declspec(naked) HealthHook()
{
	__asm
	{
		jmp[dwHealthJMPBackAdd]
	}
}

DWORD WINAPI dwMyThread(LPVOID param)
{
	// Ammo hook
	DWORD dwAmmoHookAdd = 0x00992F57;
	DWORD dwAmmoHookLength = 5;

	// Health hook
	DWORD dwHealthHookAdd = 0x0097F266;
	DWORD dwHealthHookLength = 6;

	// Calculate jump back addresses
	dwAmmoJMPBackAdd = dwAmmoHookAdd + dwAmmoHookLength;
	dwHealthJMPBackAdd = dwHealthHookAdd + dwHealthHookLength;

	HookMe((void*)dwAmmoHookAdd, AmmoHook, dwAmmoHookLength);
	HookMe((void*)dwHealthHookAdd, HealthHook, dwHealthHookLength);
	
	while (!GetAsyncKeyState(VK_INSERT));

	FreeLibraryAndExitThread((HMODULE)param, 0);

	return 0;
}

BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)dwMyThread, hModule, 0, NULL);
	}

	return true;
}