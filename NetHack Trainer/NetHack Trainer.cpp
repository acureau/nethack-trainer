#include "process.h"
#include "memory.h"
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>


// Global flags.
BOOL hasQuit = FALSE;
BOOL hasClosed = FALSE;
BOOL debug = FALSE;
BOOL acPatched = FALSE;

// Shameless stack overflow grab. Splits string into vector at delimeter.
std::vector<std::string> split(const std::string& s, char delim) {
	std::vector<std::string> result;
	std::stringstream ss(s);
	std::string item;
	while (getline(ss, item, delim)) {
		result.push_back(item);
	}
	return result;
}

void checkClosed(process* processPointer) {
	DWORD exitCode;
	GetExitCodeProcess(processPointer->readProcessHandle(), &exitCode);
	if (exitCode == STILL_ACTIVE) {
		hasClosed = FALSE;
	}
	else {
		hasClosed = TRUE;
	}
}

int main();
void reattach(process* processPointer) {
	delete(processPointer);
	system("cls");
	main();
}

void title() {
	std::cout << "Process attached. Enter 'help' to get started." << std::endl;
}

void help() {
	const char* helpMenu =
		"\n\tSTATS\n"
		"\t=================\n"
		"\tarmorclass (num) \t\t set the armorclass to specified num.\n"
		"\tstrength (num) \t\t\t set the strength value to specified num.\n"
		"\tintelligence (num) \t\t set the intelligence value to specified num.\n"
		"\twisdom (num) \t\t\t set the wisdom value to specified num.\n"
		"\tdexterity (num) \t\t set the dexterity value to specified num.\n"
		"\tconstitution (num) \t\t set the constitution value to specified num.\n"
		"\tcharisma (num) \t\t\t set the charisma value to specified num.\n"
		"\thealth (num)/(max_num) \t\t set the health and maximum health to specified nums.\n"
		"\tpower (num)/(max_num) \t\t set the health and maximum health to specified nums.\n"

		"\n\tNON-VISIBLE STATS\n"
		"\t=================\n"
		"\tviewinvis \t\t\t view the values of invisible stats.\n"
		"\tnutrition (num) \t\t set the nutrition value to specified num.\n"
		"\texp (num) \t\t\t set the experience value to specified num.\n"
		"\tpos (x, y) \t\t\t set the position to provided coordinates.\n"

		"\n\tMISC\n"
		"\t=================\n"
		"\twizard \t\t\t\t enter debug mode. (https://nethackwiki.com/wiki/Wizard_mode)\n"
		"\tturn (num) \t\t\t set the turn value to specified num.\n"
		"\t!(CMD) \t\t\t\t execute system command.\n"
		"\thelp \t\t\t\t view this menu.\n"
		"\tquit \t\t\t\t exit the program.\n";
	std::cout << helpMenu;
}

void tick(process* processPointer) {
	// Toggle flags.
	BOOL wizardMode = FALSE;

	// Revert any patches made.
	auto unPatch = [&]() {
		if (acPatched) {
			uintptr_t jmpAddress = (processPointer->readModuleBaseAddress() + 0x72068);
			memory::patchBytes((BYTE*)jmpAddress, (BYTE*)"\x74\x10", 2, processPointer->readProcessHandle());
			acPatched = FALSE;
		}
	};

	// Edits a single value for commands taking one numerical argument.
	auto valueEdit = [&](std::string arg2, uintptr_t address) {
		try {
			int newValue = std::stoi(arg2);
			WriteProcessMemory(processPointer->readProcessHandle(), (BYTE*)address, &newValue, sizeof(newValue), nullptr);
		}
		catch (...) {
			std::cout << "\n\t> second argument should be a number\n";
		}
	};

	// Edits a value which has a cap.
	auto valCap = [&](std::vector<std::string> commandVector, uintptr_t minAddress, uintptr_t maxAddress) {
		std::vector<std::string> values = split(commandVector[1], '/');
		if (values.size() == 2) {
			if (values[0].length() > 0 and values[1].length() > 0) {
				try {
					int minValue;
					int maxValue;
					minValue = stoi(values[0]);
					maxValue = stoi(values[1]);
					WriteProcessMemory(processPointer->readProcessHandle(), (BYTE*)minAddress, &minValue, sizeof(minValue), nullptr);
					WriteProcessMemory(processPointer->readProcessHandle(), (BYTE*)maxAddress, &maxValue, sizeof(maxValue), nullptr);
				}
				catch (...) {
					std::cout << "\n\t> both values must be numbers\n";
				}
			}
			else {
				std::cout << "\n\t> enter argument in the format 'num/max_num'\n";
			}
		}
		else {
			std::cout << "\n\t> enter argument in the format 'num/max_num'\n";
		}
	};

	// Command parsing and execution.
	auto inputCommand = [&]() {

		std::cout << "\n# ";
		std::string command;
		std::getline(std::cin, command);
		std::vector<std::string> commandVector = split(command, ' ');

		// Check if process is closed, do nothing if so.
		checkClosed(processPointer);
		if (hasClosed) {
			return(FALSE);												// Pseudo-break.
		}

		if (commandVector.size() == 0) {
			std::cout << "\n\t> enter a command\n";
		}
		else if (commandVector[0].at(0) == '!') {
			if (commandVector[0].length() > 1) {
				command.erase(command.begin());
				const char* cmd = command.c_str();
				system(cmd);
			}
			else {
				std::cout << "\n\t> invalid command syntax\n";
			}
		}
		else if (commandVector[0] == "help") {
			help();
		}
		else if (commandVector[0] == "quit") {
			unPatch();
			hasQuit = TRUE;
		}
		else if (commandVector[0] == "viewinvis") {
			int nutritionValue;
			uint8_t xValue;
			uint8_t yValue;
			int expValue;

			ReadProcessMemory(processPointer->readProcessHandle(), (BYTE*)(processPointer->readModuleBaseAddress() + 0x3FBC7C), &nutritionValue, sizeof(nutritionValue), nullptr);
			ReadProcessMemory(processPointer->readProcessHandle(), (BYTE*)(processPointer->readModuleBaseAddress() + 0x3FBC30), &xValue, sizeof(xValue), nullptr);
			ReadProcessMemory(processPointer->readProcessHandle(), (BYTE*)(processPointer->readModuleBaseAddress() + 0x3FBC31), &yValue, sizeof(yValue), nullptr);
			ReadProcessMemory(processPointer->readProcessHandle(), (BYTE*)(processPointer->readModuleBaseAddress() + 0x3FC0E4), &expValue, sizeof(expValue), nullptr);


			std::cout << std::dec << "\n\t> nutrition: " << nutritionValue;
			std::cout << "\n\t> position: (" << unsigned(xValue) << ", " << unsigned(yValue) << ")";
			std::cout << "\n\t> exp: " << expValue << std::endl;
		}
		else if (commandVector[0] == "wizard") {
			int currentMode;
			uintptr_t wizardAddress = (processPointer->readModuleBaseAddress() + 0x3FC309);
			ReadProcessMemory(processPointer->readProcessHandle(), (BYTE*)wizardAddress, &currentMode, sizeof(currentMode), nullptr);
			
			if (currentMode == 0) {
				currentMode = 1;
				WriteProcessMemory(processPointer->readProcessHandle(), (BYTE*)wizardAddress, &currentMode, sizeof(currentMode), nullptr);
				std::cout << "\n\t> wizard mode toggled on\n";
			}
			else {
				currentMode = 0;
				WriteProcessMemory(processPointer->readProcessHandle(), (BYTE*)wizardAddress, &currentMode, sizeof(currentMode), nullptr);
				std::cout << "\n\t> wizard mode toggled off\n";
			}
		}
		else if (commandVector[0] == "turn") {
			if (commandVector.size() == 2) {
				uintptr_t turnAddress = (processPointer->readModuleBaseAddress() + 0x3E59D4);
				valueEdit(commandVector[1], turnAddress);
			}
			else {
				std::cout << "\n\t> command takes one argument\n";
			}
		}
		else if (commandVector[0] == "nutrition") {
			if (commandVector.size() == 2) {
				uintptr_t nutritionAddress = (processPointer->readModuleBaseAddress() + 0x3FBC7C);
				valueEdit(commandVector[1], nutritionAddress);
			}
			else {
				std::cout << "\n\t> command takes one argument\n";
			}
		}
		else if (commandVector[0] == "strength") {
			if (commandVector.size() == 2) {
				uintptr_t strengthAddress = (processPointer->readModuleBaseAddress() + 0x3FC048);
				valueEdit(commandVector[1], strengthAddress);
			}
			else {
				std::cout << "\n\t> command takes one argument\n";
			}
		}
		else if (commandVector[0] == "intelligence") {
			if (commandVector.size() == 2) {
				uintptr_t intelligenceAddress = (processPointer->readModuleBaseAddress() + 0x3FC049);
				valueEdit(commandVector[1], intelligenceAddress);
			}
			else {
				std::cout << "\n\t> command takes one argument\n";
			}
		}
		else if (commandVector[0] == "wisdom") {
			if (commandVector.size() == 2) {
				uintptr_t wisdomAddress = (processPointer->readModuleBaseAddress() + 0x3FC04A);
				valueEdit(commandVector[1], wisdomAddress);
			}
			else {
				std::cout << "\n\t> command takes one argument\n";
			}
		}
		else if (commandVector[0] == "dexterity") {
			if (commandVector.size() == 2) {
				uintptr_t dexterityAddress = (processPointer->readModuleBaseAddress() + 0x3FC04B);
				valueEdit(commandVector[1], dexterityAddress);
			}
			else {
				std::cout << "\n\t> command takes one argument\n";
			}
		}
		else if (commandVector[0] == "constitution") {
			if (commandVector.size() == 2) {
				uintptr_t constitutionAddress = (processPointer->readModuleBaseAddress() + 0x3FC04C);
				valueEdit(commandVector[1], constitutionAddress);
			}
			else {
				std::cout << "\n\t> command takes one argument\n";
			}
		}
		else if (commandVector[0] == "charisma") {
			if (commandVector.size() == 2) {
				uintptr_t charismaAddress = (processPointer->readModuleBaseAddress() + 0x3FC04D);
				valueEdit(commandVector[1], charismaAddress);
			}
			else {
				std::cout << "\n\t> command takes one argument\n";
			}
		}
		else if (commandVector[0] == "exp") {
			if (commandVector.size() == 2) {
				uintptr_t expAddress = (processPointer->readModuleBaseAddress() + 0x3FC0E4);
				valueEdit(commandVector[1], expAddress);
			}
			else {
				std::cout << "\n\t> command takes one argument\n";
			}
		}
		else if (commandVector[0] == "pos") {
			if (commandVector.size() == 3) {
				uint8_t xPos;
				uint8_t yPos;
				try {
					if (commandVector[1].length() > 1 && commandVector[1].length() > 1) {
						commandVector[1].erase(0, 1);							// Remove first character from X. '('
						commandVector[1].pop_back();							// Remove last character from X. ','
						commandVector[2].pop_back();							// Remove last character from Y. ')'
						xPos = stoi(commandVector[1]);
						yPos = stoi(commandVector[2]);

						uintptr_t xAddress = (processPointer->readModuleBaseAddress() + 0x3FBC30);
						uintptr_t yAddress = (processPointer->readModuleBaseAddress() + 0x3FBC31);
						WriteProcessMemory(processPointer->readProcessHandle(), (BYTE*)xAddress, &xPos, sizeof(xPos), nullptr);
						WriteProcessMemory(processPointer->readProcessHandle(), (BYTE*)yAddress, &yPos, sizeof(yPos), nullptr);
					}
					else {
						std::cout << "\n\t> enter position in the format (x, y)\n";
					}
				}
				catch (...) {
					std::cout << "\n\t> position values must be numbers\n";
				}
			}
			else {
				std::cout << "\n\t> enter position in the format (x, y)\n";
			}
		}
		else if (commandVector[0] == "health") {
			if (commandVector.size() == 2) {
				uintptr_t minAddress = (processPointer->readModuleBaseAddress() + 0x3FC080);
				uintptr_t maxAddress = (processPointer->readModuleBaseAddress() + 0x3FC084);
				valCap(commandVector, minAddress, maxAddress);
			}
			else {
				std::cout << "\n\t> command takes one argument\n";
			}
		}
		else if (commandVector[0] == "power") {
			if (commandVector.size() == 2) {
				uintptr_t minAddress = (processPointer->readModuleBaseAddress() + 0x3FC088);
				uintptr_t maxAddress = (processPointer->readModuleBaseAddress() + 0x3FC08C);
				valCap(commandVector, minAddress, maxAddress);
			}
			else {
				std::cout << "\n\t> command takes one argument\n";
			}
		}
		else if (commandVector[0] == "armorclass") {
			if (commandVector.size() == 2) {
				// Patch JE instruction to JMP, will allow AC value to be edited.
				if (!acPatched) {
					uintptr_t jeAddress = (processPointer->readModuleBaseAddress() + 0x72068);
					memory::patchBytes((BYTE*)jeAddress, (BYTE*)"\xEB\x10", 2, processPointer->readProcessHandle());
					acPatched = TRUE;
				}
				uintptr_t acAddress = (processPointer->readModuleBaseAddress() + 0x3FC07A);
				valueEdit(commandVector[1], acAddress);
			}
			else {
				std::cout << "\n\t> command takes one argument\n";
			}
		}
		
		// Debug commands & fail path.
		else if (commandVector[0] == "debug") {
			if (debug) {
				debug = FALSE;
				std::cout << "\n\t> debug stats disabled\n";
			}
			else {
				debug = TRUE;
				std::cout << "\n\t> debug stats enabled\n";
			}
		}
		else if (commandVector[0] == "reattach") {
			reattach(processPointer);
		}
		else {															// Unrecognized input.
			std::cout << "\n\t> unknown command\n";
		}
	};

	inputCommand();
}

void processError(std::string errorMessage, process* processPointer);
int main() {
	SetConsoleTitleA("NetHack Trainer");
	process* processPointer = new process(L"NetHack.exe");				// Create a pointer to a new process object for 'NetHack'.
	int status = processPointer->attach();								// Attach program to process. Refer to 'process.cpp'.
	system("cls");
	if (debug) {
		processPointer->printStatus();
	}
	if (status == 1) {
		title();
		do {															// Main trainer loop. While not closed or quit.
			checkClosed(processPointer);
			tick(processPointer);
		} while (!hasQuit && !hasClosed);
		if (!hasQuit) {													// Call self if not terminated.
			processError("Process closed or trainer error. Attach again? (y/n)", processPointer);
		}
	}
	else {
		processError("Attach unsuccessful. Try again? (y/n)", processPointer);
	}
}

void processError(std::string errorMessage, process* processPointer) {
	delete(processPointer);
	system("cls");
	std::cout << errorMessage << std::endl;
	std::string option;
	std::getline(std::cin, option);
	if (option == "y") {
		main();
	}
	else {
		exit(0);
	}
}