
#include "labCursorManager.h"
#include <map>

#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>

namespace lab
{

    class CursorManager::Detail
    {
        public:
        ~Detail()
        {
            for (auto c : cursors)
                DestroyCursor(c.second);
        }
        std::map<Cursor, HCURSOR> cursors;
		Cursor lru = Cursor::None;
    };

    CursorManager::CursorManager()
    : _detail(new Detail)
    {
    }

	CursorManager::~CursorManager()
	{
		delete _detail;
	}

	void CursorManager::hide()
	{
		SetCursor(NULL);
	}

	void CursorManager::set_cursor(Cursor cursor)
	{
		if (_detail->lru == cursor)
			return;

		HCURSOR c = NULL;
		auto cursorIt = _detail->cursors.find(cursor);
		if (cursorIt == _detail->cursors.end())
		{
			HCURSOR newCursor = NULL;

			switch (cursor)
			{
			case Cursor::Arrow:                  newCursor = LoadCursor(NULL, IDC_ARROW);       break;
			case Cursor::ArrowWait:              newCursor = LoadCursor(NULL, IDC_APPSTARTING); break;
			case Cursor::Wait:                   newCursor = LoadCursor(NULL, IDC_WAIT);        break;
			case Cursor::Text:                   newCursor = LoadCursor(NULL, IDC_IBEAM);       break;
			case Cursor::Hand:                   newCursor = LoadCursor(NULL, IDC_HAND);        break;
			case Cursor::SizeHorizontal:         newCursor = LoadCursor(NULL, IDC_SIZEWE);      break;
			case Cursor::SizeVertical:           newCursor = LoadCursor(NULL, IDC_SIZENS);      break;
			case Cursor::SizeTopLeftBottomRight: newCursor = LoadCursor(NULL, IDC_SIZENWSE);    break;
			case Cursor::SizeBottomLeftTopRight: newCursor = LoadCursor(NULL, IDC_SIZENESW);    break;
			case Cursor::SizeAll:                newCursor = LoadCursor(NULL, IDC_SIZEALL);     break;
			case Cursor::Cross:                  newCursor = LoadCursor(NULL, IDC_CROSS);       break;
			case Cursor::Help:                   newCursor = LoadCursor(NULL, IDC_HELP);        break;
			case Cursor::NotAllowed:             newCursor = LoadCursor(NULL, IDC_NO);          break;
			default: return;
			}

			// Create a copy of the shared system cursor that we can destroy later
			c = CopyCursor(newCursor);

			_detail->cursors[cursor] = c;
		}
		else
			c = cursorIt->second;

		SetCursor(c);
		_detail->lru = cursor;
	}

	void CursorManager::set_cursor(ImGuiMouseCursor cursor)
	{
		switch (cursor)
		{
		case ImGuiMouseCursor_None: hide(); break;
		case ImGuiMouseCursor_Arrow: set_cursor(Cursor::Arrow); break;
		case ImGuiMouseCursor_TextInput: set_cursor(Cursor::Text); break;
		case ImGuiMouseCursor_Move: set_cursor(Cursor::Hand); break;
		case ImGuiMouseCursor_ResizeNS: set_cursor(Cursor::SizeVertical); break;
		case ImGuiMouseCursor_ResizeEW: set_cursor(Cursor::SizeHorizontal); break;
		case ImGuiMouseCursor_ResizeNESW: set_cursor(Cursor::SizeBottomLeftTopRight); break;
		case ImGuiMouseCursor_ResizeNWSE: set_cursor(Cursor::SizeBottomLeftTopRight); break;
		}
	};

} // lab

