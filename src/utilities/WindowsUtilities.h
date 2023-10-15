#pragma once

#ifdef _WIN32

#include <string>
#include <ShObjIdl.h>

#include <SFML/Graphics/RenderWindow.hpp>

inline void HideTaskbarIcon(const sf::RenderWindow& window)
{
	ITaskbarList* pTaskList = NULL;

	if (CoInitialize(NULL) != S_OK)
		return;

	HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList, (LPVOID*)&pTaskList);

	if (hr == S_OK)
	{
		pTaskList->DeleteTab(window.getNativeHandle());
		pTaskList->Release();
	}

	CoUninitialize();
}

inline std::wstring GetWallpaperPath()
{
	std::wstring result;

	WCHAR backgroundPath[MAX_PATH] = { 0 };
	if (SystemParametersInfoW(SPI_GETDESKWALLPAPER, MAX_PATH, backgroundPath, 0))
	{
		result = std::wstring(backgroundPath);
	}

	return result;
}

#endif