#pragma once
#include <windows.h>
#include <vector>

namespace memory {
	uintptr_t resolvePointer(HANDLE process, uintptr_t basePointer, std::vector<unsigned int> offsets);
	void patchBytes(BYTE* destination, BYTE* source, unsigned int size, HANDLE processHandle);
	void nopBytes(BYTE* destination, unsigned int size, HANDLE processHandle);
}