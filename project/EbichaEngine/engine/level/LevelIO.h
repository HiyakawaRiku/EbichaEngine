#pragma once
#include <string>
#include "LevelData.h"

// レベルデータ（JSON）の読み書き
namespace LevelIO {

	// JSON ファイルを読み込んで outData に入れる。失敗したら false を返す（outData は変更しない）
	bool Load(const std::string& filePath, LevelData& outData);

	// data を JSON ファイルに書き出す。失敗したら false を返す
	bool Save(const std::string& filePath, const LevelData& data);

}
