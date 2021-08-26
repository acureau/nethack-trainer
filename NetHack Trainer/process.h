#pragma once
#include <Windows.h>

class process
{
private:
	const wchar_t* name;
	DWORD pid;
	uintptr_t moduleBaseAddress;
	HANDLE processHandle = NULL;

public:
	wchar_t readName() { return(*name); };
	DWORD readProcessID() { return(pid); };
	uintptr_t readModuleBaseAddress() { return(moduleBaseAddress); };
	HANDLE readProcessHandle() { return(processHandle); };
	int attach();
	void printStatus();

	process() = delete;											// Forbid default constructor.
	process(const wchar_t* winName) { name = winName; };		// Constuctor to set window name on declaration.
	~process();
};
