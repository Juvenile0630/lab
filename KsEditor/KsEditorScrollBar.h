#pragma once

#include <windows.h>

class KsEditorScrollBar
{
public:
	KsEditorScrollBar();
	virtual ~KsEditorScrollBar();
	bool Create(HWND hWnd, HINSTANCE hInstance);
	bool SetClientRect(RECT rect);
	bool SetCaretPosition(float caretX, float caretY);

private:
	HINSTANCE m_hInstance;
	HWND m_hWndV;
	HWND m_hWndH;
	HWND m_parent;
	SCROLLINFO m_scrollInfoV;
	SCROLLINFO m_scrollInfoH;
};
