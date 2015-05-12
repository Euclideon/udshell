/**
 * @file udDebugFont.h
 * @author Dave Pevreal
 * @date 11/02/2009
 * @brief Interface to sets of graphics shaders (vertex shaders and pixel shaders)
 *
 * Copyright (c) Dave Pevreal
 */

#pragma once
#ifndef UDDEBUG_FONT
#define UDDEBUG_FONT

struct udDebugFont;

/** Begin a series of font renders */
void udDebugFont_BeginRender(const udFloat4x4 *pWVP = nullptr);

/** Render a string */
void udDebugFont_RenderString(udDebugFont *pFont, const char *pString, float x, float y, const udFloat4 &color = udFloat4::one());

/** Complete rendering */
void udDebugFont_EndRender();

/** Get the height */
int udDebugFont_GetHeight(udDebugFont *pFont);

#endif // UDDEBUG_FONT
