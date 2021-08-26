#include "process.h"
#include <string>
#include <Windows.h>
#include <tlhelp32.h>
#include <vector>
#include <iostream>

// A process id is a DWORD in the windows API.
// We use a wide char because of the visual studio character set being UNICODE.
DWORD grabProcessID(const wchar_t* processName) {
	DWORD processId = 0;
	HANDLE hSnap = (CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
	if (hSnap != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		if (Process32First(hSnap, &procEntry)) {
			do {
				if (_wcsicmp(procEntry.szExeFile, processName) == 0) {					// _wcsicmp compares two wide chars and returns 0 if identical.
					processId = procEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &procEntry));
		}
	}
	CloseHandle(hSnap);
	return(processId);
};

// uintptr_t is an unsigned integer capable of storing a data pointer.
uintptr_t grabModuleBaseAddress(DWORD processId, const wchar_t* moduleName) {
	uintptr_t moduleBaseAddress = 0;
	HANDLE hSnap = (CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId));
	if (hSnap != INVALID_HANDLE_VALUE) {
		MODULEENTRY32 moduleEntry;
		moduleEntry.dwSize = sizeof(moduleEntry);

		if (Module32First(hSnap, &moduleEntry)) {
			do {
				if (_wcsicmp(moduleEntry.szModule, moduleName) == 0) {
					moduleBaseAddress = (uintptr_t)moduleEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &moduleEntry));
		}
	}
	CloseHandle(hSnap);
	return(moduleBaseAddress);
}

int process::attach() {
	// Get process ID.
	if (grabProcessID(name)) {
		pid = grabProcessID(name);

		// Get module base address.
		if (grabModuleBaseAddress(pid, name)) {
			moduleBaseAddress = grabModuleBaseAddress(pid, name);

			// Get handle to process.
			if (OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid) != NULL) {
				processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
				return(1);
			}
		}
	}
	return(0);
};

// Print debug details.
void process::printStatus() {
	std::cout << "======== DEBUG STATUS ========\n";
	std::cout << "PID: " << std::dec << pid << std::endl;
	std::cout << "Base Address: 0x" << std::hex << moduleBaseAddress << std::endl;
	DWORD flags;
	GetHandleInformation(processHandle, &flags);

	if (flags == 0) {
		std::cout << "Handle is open." << std::endl;
	}
	else {
		std::cout << "ERROR WITH HANDLE!" << std::endl;
	}

	std::cout << "==============================\n\n";
}

process::~process() {
	CloseHandle(processHandle);
}