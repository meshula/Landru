#pragma once

#include "imgui.h"

#include <memory>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>          // vsnprintf, sscanf, printf

namespace lab { class FontManager;  }

// For the console example, here we are using a more C++ like approach of declaring a class to hold the data and the functions.
class LandruConsole
{
	char                  InputBuf[256];
	ImVector<char*>       Items;
	bool                  ScrollToBottom;
	ImVector<char*>       History;
	int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
	ImVector<const char*> Commands;

	std::shared_ptr<lab::FontManager> _fontManager;

public:
	LandruConsole(std::shared_ptr<lab::FontManager>);
	~LandruConsole();

	// Portable helpers
	static int   Stricmp(const char* str1, const char* str2) { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
	static int   Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
	static char* Strdup(const char *str) { size_t len = strlen(str) + 1; void* buff = malloc(len); return (char*)memcpy(buff, (const void*)str, len); }

	void    ClearLog();
	void    AddLog(const char* fmt, ...) IM_PRINTFARGS(2);
	void    Draw(const char* title, bool* p_open);
	void    draw_contents();
	void    ExecCommand(const char* command_line);
	static int TextEditCallbackStub(ImGuiTextEditCallbackData* data); // In C++11 you are better off using lambdas for this sort of forwarding callbacks
	int     TextEditCallback(ImGuiTextEditCallbackData* data);
};
