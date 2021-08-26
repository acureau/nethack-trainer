#include "memory.h"
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <iostream>

// Resolve the address pointed to by a pointer chain.
uintptr_t memory::resolvePointer(HANDLE process, uintptr_t basePointer, std::vector<unsigned int> offsets) {
	uintptr_t address = basePointer;
	for (unsigned int i = 0; i < offsets.size(); ++i) {
		ReadProcessMemory(process, (BYTE*)address, &address, sizeof(address), 0);
		address += offsets[i];
	}
	return(address);
}

// Patch some address with specified bytes.
void memory::patchBytes(BYTE* destination, BYTE* source, unsigned int size, HANDLE processHandle) {
	DWORD oldProtect;
	VirtualProtectEx(processHandle, destination, size, PAGE_EXECUTE, &oldProtect);
	WriteProcessMemory(processHandle, destination, source, size, nullptr);
	VirtualProtectEx(processHandle, destination, size, oldProtect, &oldProtect);
	// Protection MUST be restored after patching.
}

// Patch some specified address with NOP instructions (0x90).
void memory::nopBytes(BYTE* destination, unsigned int size, HANDLE processHandle) {
	BYTE* nopArray = new BYTE[size];
	memset(nopArray, 0x90, size);
	patchBytes(destination, nopArray, size, processHandle);
	delete[] nopArray;
}