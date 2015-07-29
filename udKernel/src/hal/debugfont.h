#pragma once
#ifndef UDDEBUG_FONT
#define UDDEBUG_FONT

struct udDebugFont;

void udDebugFont_BeginRender(const udFloat4x4 *pWVP = nullptr);

float udDebugFont_RenderString(udDebugFont *pFont, const char *pString, float x, float y, float scale = 1.f, const udFloat4 &color = udFloat4::one());

void udDebugFont_EndRender();

int udDebugFont_GetHeight(udDebugFont *pFont);


void udDebugConsole_SetCursorPos(float x, float y);
void udDebugConsole_SetTextScale(float scale);
void udDebugConsole_SetTextColor(const udFloat4 &color);
void udDebugConsole_Print(const char *pString);
void udDebugConsole_Printf(const char *pFormat, ...);

#endif // UDDEBUG_FONT
