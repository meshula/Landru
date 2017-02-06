
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
			case  ImGuiMouseCursor_Arrow: set_cursor(Cursor::Arrow); break;
			case ImGuiMouseCursor_TextInput: set_cursor(Cursor::Text); break;
			case ImGuiMouseCursor_Move: set_cursor(Cursor::Hand); break;
			case ImGuiMouseCursor_ResizeNS: set_cursor(Cursor::SizeVertical); break;
			case ImGuiMouseCursor_ResizeEW: set_cursor(Cursor::SizeHorizontal); break;
			case ImGuiMouseCursor_ResizeNESW: set_cursor(Cursor::SizeBottomLeftTopRight); break;
			case ImGuiMouseCursor_ResizeNWSE: set_cursor(Cursor::SizeBottomLeftTopRight); break;
			}
		};

} // lab

#if 0
////////////////////////////////////////////////////////////
void WindowImplWin32::setMouseCursor(const std::uint8_t* pixels, unsigned int width, unsigned int height, unsigned int hotspotX, unsigned int hotspotY)
{
    // Create the bitmap that will hold our color data
    BITMAPV5HEADER bitmapHeader;
    std::memset(&bitmapHeader, 0, sizeof(BITMAPV5HEADER));

    bitmapHeader.bV5Size        = sizeof(BITMAPV5HEADER);
    bitmapHeader.bV5Width       = width;
    bitmapHeader.bV5Height      = -(int)height; // Negative indicates origin is in upper-left corner
    bitmapHeader.bV5Planes      = 1;
    bitmapHeader.bV5BitCount    = 32;
    bitmapHeader.bV5Compression = BI_BITFIELDS;
    bitmapHeader.bV5RedMask     = 0x00ff0000;
    bitmapHeader.bV5GreenMask   = 0x0000ff00;
    bitmapHeader.bV5BlueMask    = 0x000000ff;
    bitmapHeader.bV5AlphaMask   = 0xff000000;

	std::uint8_t* bitmapData = NULL;

    HDC screenDC = GetDC(NULL);
    HBITMAP color = CreateDIBSection(
        screenDC,
        reinterpret_cast<const BITMAPINFO*>(&bitmapHeader),
        DIB_RGB_COLORS,
        reinterpret_cast<void**>(&bitmapData),
        NULL,
        0
    );
    ReleaseDC(NULL, screenDC);

    if (!color)
    {
        err() << "Failed to create cursor color bitmap" << std::endl;
        return;
    }

    // Fill our bitmap with the cursor color data
    std::memcpy(bitmapData, pixels, width * height * 4);

    // Create a dummy mask bitmap (it won't be used)
    HBITMAP mask = CreateBitmap(width, height, 1, 1, NULL);

    if (!mask)
    {
        DeleteObject(color);
        err() << "Failed to create cursor mask bitmap" << std::endl;
        return;
    }

    // Create the structure that describes our cursor
    ICONINFO cursorInfo;
    std::memset(&cursorInfo, 0, sizeof(ICONINFO));

    cursorInfo.fIcon    = FALSE; // This is a cursor and not an icon
    cursorInfo.xHotspot = hotspotX;
    cursorInfo.yHotspot = hotspotY;
    cursorInfo.hbmColor = color;
    cursorInfo.hbmMask  = mask;

    // Create the cursor
    HCURSOR newCursor = reinterpret_cast<HCURSOR>(CreateIconIndirect(&cursorInfo));

    // The data has been copied into the cursor, so get rid of these
    DeleteObject(color);
    DeleteObject(mask);

    if (!newCursor)
    {
        err() << "Failed to create cursor from bitmaps" << std::endl;
        return;
    }

    HCURSOR oldCursor = m_loadedCursor;
    m_loadedCursor = newCursor;

    if (m_cursor)
    {
        m_cursor = m_loadedCursor;
        SetCursor(m_cursor);
    }

    if (oldCursor)
        DestroyCursor(oldCursor);
}

#endif
