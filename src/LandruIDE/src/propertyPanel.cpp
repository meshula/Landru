
#include "editState.h"
#include "interface/ImGuizmo.h"

#include <imgui.h>
#include <stdio.h>

using namespace lab;

static float trans[4] = { 0.f, 1.f, 2.f, 1.f };
static float rot[4] = { 0.f, 0.f, 0.f, 1.f };
static float scale[4] = { 1.f, 1.f, 1.f, 1.f };

static void set_manipulated_matrix(const EditState::float16 & m)
{
	float manipulator_matrix[16];
	memcpy(&manipulator_matrix, &m, sizeof(EditState::float16));
	ImGuizmo::DecomposeMatrixToComponents(manipulator_matrix, trans, rot, scale);
}


void property_panel()
{
	static std::once_flag once;
	std::call_once(once, []()
	{
		evt_set_manipulated_matrix.connect(&set_manipulated_matrix);
	});

   	//ShowHelpMarker("This example shows how you may implement a property editor using two columns.\nAll objects/fields data are dummies here.\nRemember that in many simple cases, you can use ImGui::SameLine(xxx) to position\nyour cursor horizontally instead of using the Columns() API.");

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Columns(2);
	ImGui::Separator();
	ImGui::SetColumnOffset(1, 160);

	ImGui::PushID(0);

	ImGui::AlignFirstTextHeightToWidgets();  // Text and Tree nodes are less high than regular widgets, here we add vertical spacing to make the tree lines equal high.
	bool node_open = ImGui::TreeNode("Transform");
	ImGui::NextColumn();
	ImGui::AlignFirstTextHeightToWidgets();
//	ImGui::Text("why");
	ImGui::NextColumn();

	if (node_open)
	{
		float edit_trans[4];
		memcpy(edit_trans, trans, sizeof(edit_trans));
		float edit_rot[4];
		memcpy(edit_rot, rot, sizeof(edit_rot));
		float edit_scale[4];
		memcpy(edit_scale, scale, sizeof(edit_scale));

		ImGui::AlignFirstTextHeightToWidgets();
		ImGui::PushID(1); // identify translation
//		ImGui::Bullet();
		ImGui::Selectable("Translation");
		ImGui::NextColumn();
		ImGui::InputFloat3("", edit_trans);
		ImGui::PushItemWidth(-1);
		ImGui::NextColumn();
		ImGui::PopItemWidth();
		ImGui::PopID();

		ImGui::AlignFirstTextHeightToWidgets();
		ImGui::PushID(2); // identify rotation
	//	ImGui::Bullet();
		ImGui::Selectable("Rotation");
		ImGui::NextColumn();
		ImGui::InputFloat3("", edit_rot);
		ImGui::PushItemWidth(-1);
		ImGui::NextColumn();
		ImGui::PopItemWidth();
		ImGui::PopID();

		ImGui::AlignFirstTextHeightToWidgets();
		ImGui::PushID(3); // identify scale
		//ImGui::Bullet();
		ImGui::Selectable("Scale");
		ImGui::NextColumn();
		ImGui::InputFloat3("", edit_scale);
		ImGui::PushItemWidth(-1);
		ImGui::NextColumn();
		ImGui::PopItemWidth();
		ImGui::PopID();

		ImGui::TreePop();

		bool recompose = false;
		for (int i = 0; i < 4 && !recompose; ++i)
			recompose |= trans[i] != edit_trans[i];
		for (int i = 0; i < 4 && !recompose; ++i)
			recompose |= rot[i] != edit_rot[i];
		for (int i = 0; i < 4 && !recompose; ++i)
			recompose |= scale[i] != edit_scale[i];

		if (recompose)
		{
			float result[16];
			ImGuizmo::RecomposeMatrixFromComponents(edit_trans, edit_rot, edit_scale, result);
			evt_set_manipulated_matrix(result);
		}
	}

	ImGui::PopID();

	ImGui::Columns(1);
	ImGui::Separator();
	ImGui::PopStyleVar();
}