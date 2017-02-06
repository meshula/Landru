
#pragma once

#include <imgui.h>

namespace lab
{

    class CursorManager
    {
        class Detail;
        Detail * _detail;

        public:

        enum class Cursor {
            Arrow, ArrowWait, Wait, Text, Hand,
            SizeHorizontal, SizeVertical, SizeTopLeftBottomRight, SizeBottomLeftTopRight,
            SizeAll, Cross, Help, NotAllowed, None
        };

        CursorManager();
        ~CursorManager();

        void hide();
        void set_cursor(Cursor cursor);
		void set_cursor(ImGuiMouseCursor);
    };

}