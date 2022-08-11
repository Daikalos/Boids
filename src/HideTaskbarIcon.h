#pragma once

#include <SFML/Graphics.hpp>
#include <ShObjIdl.h>

static void hide_taskbar_icon(const sf::RenderWindow& render_window)
{
	ITaskbarList* pTaskList = NULL;
	
	if (CoInitialize(NULL) != S_OK)
		return;

	HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList, (LPVOID*)&pTaskList);

	if (hr == S_OK)
	{
		pTaskList->DeleteTab(render_window.getSystemHandle());
		pTaskList->Release();
	}

	CoUninitialize();
}