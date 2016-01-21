/*
* Copyright (c) 2016 Frank Richter
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software Foundation,
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

extern "C"
{
#include "atlasimage.h"
}

#include <comdef.h>
#include <gdiplus.h>
#include <Objidl.h>
#include <Shlwapi.h>

#include <memory>

class GdiplusInit
{
    ULONG_PTR token;
public:
    GdiplusInit()
    {
        Gdiplus::GdiplusStartupInput inp;
        Gdiplus::GdiplusStartup (&token, &inp, nullptr);
    }
    ~GdiplusInit()
    {
        Gdiplus::GdiplusShutdown (token);
    }
};
static GdiplusInit _gdip_init;

#define TMT_DISKSTREAM  213

#define TMT_ATLASRECT   8002

_COM_SMARTPTR_TYPEDEF(IStream, __uuidof(IStream));

HBITMAP get_atlas_image (HTHEME theme, int part, int state)
{
    RECT atlas_rect;
    if (!SUCCEEDED (GetThemeRect (theme, part, state, TMT_ATLASRECT, &atlas_rect)))
        return 0;

    HBITMAP result = 0;
    WCHAR theme_fn[MAX_PATH];

    if (SUCCEEDED (GetCurrentThemeName (theme_fn, sizeof (theme_fn) / sizeof (theme_fn[0]), NULL, 0, NULL, 0)))
    {
        HMODULE hInst = LoadLibraryExW (theme_fn, NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (hInst)
        {
            void* pStream;
            DWORD cbStream;
            if (SUCCEEDED (GetThemeStream (theme, part, state, TMT_DISKSTREAM, &pStream, &cbStream, hInst)))
            {
                IStreamPtr stream (SHCreateMemStream (static_cast<BYTE*> (pStream), cbStream), false);
                std::unique_ptr<Gdiplus::Bitmap> atlas_bmp (new Gdiplus::Bitmap (stream));
                atlas_bmp.reset (atlas_bmp->Clone (atlas_rect.left, atlas_rect.top,
                                                   atlas_rect.right - atlas_rect.left,
                                                   atlas_rect.bottom - atlas_rect.top,
                                                   atlas_bmp->GetPixelFormat ()));
                atlas_bmp->GetHBITMAP (Gdiplus::Color::Transparent, &result);
            }
            FreeLibrary (hInst);
        }
    }

    return result;
}
