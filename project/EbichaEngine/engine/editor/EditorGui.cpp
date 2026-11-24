#include "EditorGui.h"

#ifdef USE_IMGUI
#include "imgui.h"
#endif

namespace EditorGui {

	bool BlendModeCombo(const char* label, BlendMode& mode) {
#ifdef USE_IMGUI
		bool changed = false;
		if (ImGui::BeginCombo(label, GetBlendModeName(mode))) {
			for (int i = 0; i < (int)BlendMode::kCount; i++) {
				bool isSelected = (i == (int)mode);
				if (ImGui::Selectable(GetBlendModeName((BlendMode)i), isSelected)) {
					mode = (BlendMode)i;
					changed = true;
				}
			}
			ImGui::EndCombo();
		}
		return changed;
#else
		(void)label;
		(void)mode;
		return false;
#endif
	}

}
