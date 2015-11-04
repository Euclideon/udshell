#pragma once
#ifndef UDDEBUG_FONT
#define UDDEBUG_FONT

struct epDebugFont;

void epDebugFont_BeginRender(const Float4x4 *pWVP = nullptr);

float epDebugFont_RenderString(epDebugFont *pFont, const char *pString, float x, float y, float scale = 1.f, const Float4 &color = Float4::one());

void epDebugFont_EndRender();

int epDebugFont_GetHeight(epDebugFont *pFont);


void epDebugConsole_SetCursorPos(float x, float y);
void epDebugConsole_SetTextScale(float scale);
void epDebugConsole_SetTextColor(const Float4 &color);
void epDebugConsole_Print(const char *pString);
void epDebugConsole_Printf(const char *pFormat, ...);

#endif // UDDEBUG_FONT
