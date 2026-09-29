#pragma once

//ファイルに書いたり読んだりするライブラリ
#include <fstream>
//時間を扱うライブラリ
#include <chrono>
#include "ConvertString.h"

namespace Logger {

	void Log(const std::string& message);

	void Log(std::ostream& os, const std::string& message);

	void Log(const std::wstring& message);


}
