#pragma once
#include <string>
#include <vector>
#include "EMath.h"

// レベルに置かれた、オブジェクト1つ分のデータ
// ※ 値は JSON（＝Blender）のまま。エンジンの座標への変換は、オブジェクトを作るときに行う
struct LevelObjectData {
	std::string type;       // "EMPTY"（空のオブジェクト）か "MESH"（モデル）
	std::string name;       // 名前
	std::string fileName;   // MESH のときのモデルのファイル名（EMPTY のときは空）

	Vector3 translation = { 0.0f, 0.0f, 0.0f };   // 位置
	Vector3 rotation = { 0.0f, 0.0f, 0.0f };      // 回転（ラジアン）
	Vector3 scaling = { 1.0f, 1.0f, 1.0f };       // 大きさ
};

// レベル1つ分のデータ（JSON ファイル1つに対応する）
struct LevelData {
	std::string name;
	std::vector<LevelObjectData> objects;
};
