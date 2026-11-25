#pragma once

// 「エディタのインスペクターに表示できるもの」の約束
// これを継承したクラスは、OnInspector() を必ず書く
class IEditorInspectable {
public:
	virtual ~IEditorInspectable() = default;

	// 自分のパラメータを ImGui で表示・編集する
	virtual void OnInspector() = 0;
};
