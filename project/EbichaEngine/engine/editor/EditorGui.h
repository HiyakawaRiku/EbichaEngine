#pragma once
#include "BlendMode.h"

// エディタ用の、よく使う ImGui の部品をまとめた場所
// USE_IMGUI が定義されていない構成（Release）では、何もしない
namespace EditorGui {

	// BlendMode を選ぶ Combo。変更されたら true を返す
	bool BlendModeCombo(const char* label, BlendMode& mode);

}
