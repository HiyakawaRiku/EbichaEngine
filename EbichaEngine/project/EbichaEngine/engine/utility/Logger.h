#pragma once
#include <Windows.h>
#include <fstream>

namespace Logger {
	void Log(const std::string& message);

	void Log(std::ostream& os, const std::string& message);

	void Log(const std::wstring& message);

}

