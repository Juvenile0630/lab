#include "pch.h"
#include "KsEditorScrollBar.h"

KsEditorScrollBar::KsEditorScrollBar() :
	m_hInstance(),
	m_hWndV(NULL),
	m_hWndH(NULL),
	m_parent(NULL),
	m_scrollInfoV(),
	m_scrollInfoH()
{

}

KsEditorScrollBar::~KsEditorScrollBar()
{

}

bool KsEditorScrollBar::Create(HWND hWnd, HINSTANCE hInstance)
{
	if (hWnd == NULL) {
		return false;
	}
	m_parent = hWnd;
	m_hInstance = hInstance;
	m_hWndV = CreateWindowEx(
		0,
		L"SCROLLBAR",
		L"",
		WS_CHILD | SBS_VERT,
		0, 0, 0, 0,
		m_parent,
		(HMENU)1,
		m_hInstance,
		NULL);
	m_scrollInfoV.cbSize = sizeof(SCROLLINFO);
	m_scrollInfoV.fMask = SIF_PAGE | SIF_RANGE;
	m_scrollInfoV.nMin = 0;	m_scrollInfoV.nMax = 100;
	m_scrollInfoV.nPage = 10;
	SetScrollInfo(m_hWndV, SB_CTL, &m_scrollInfoV, TRUE);
	m_scrollInfoV.fMask = SIF_POS;

	m_hWndH = CreateWindowEx(
		0,
		L"SCROLLBAR",
		L"",
		WS_CHILD | SBS_HORZ,
		0, 0, 0, 0,
		m_parent,
		(HMENU)1,
		m_hInstance,
		NULL);

	m_scrollInfoH.cbSize = sizeof(SCROLLINFO);
	m_scrollInfoH.fMask = SIF_PAGE | SIF_RANGE;
	m_scrollInfoH.nMin = 0;	m_scrollInfoH.nMax = 100;
	m_scrollInfoH.nPage = 10;
	SetScrollInfo(m_hWndH, SB_CTL, &m_scrollInfoH, TRUE);
	m_scrollInfoH.fMask = SIF_POS;
	return true;
}

