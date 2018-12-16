
#pragma once


#ifdef _MSC_VER
// suppress warnings until USD is cleaned up
#  pragma warning(push)
#  pragma warning(disable : 4244 4305)
#endif

#include <pxr/usd/usd/stage.h>


#ifdef _MSC_VER
// suppress warnings until USD is cleaned up
#  pragma warning(pop)
#endif

namespace lab { class EditState; class FontManager; }


class Outliner
{
public:
	Outliner();
	~Outliner();

	void ui(lab::EditState & edit_state, lab::FontManager & fm);

private:
	void stage_created(UsdStageRefPtr new_stage);
	void layer_created(const std::string & s);
		
	class Detail;
	Detail * detail;
};



