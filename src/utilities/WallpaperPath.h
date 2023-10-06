#pragma once

#include <string>

#ifdef _WIN32

#include <Windows.h>

inline std::wstring GetWallpaperPath()
{
	std::wstring result;

	WCHAR backgroundPath[MAX_PATH];
	if (SystemParametersInfoW(SPI_GETDESKWALLPAPER, MAX_PATH, backgroundPath, 0))
	{
		result = std::wstring(backgroundPath);
	}

	return result;
}

#endif