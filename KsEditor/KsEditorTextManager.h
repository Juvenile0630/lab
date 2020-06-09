#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include "KsEditorCursorManager.h"


class KsEditorTextManager
{
public:
	KsEditorTextManager();
	virtual ~KsEditorTextManager();
	void SetCursorManager(KsEditorCursorManager* pCursorManager) { m_pCursorManager = pCursorManager; }
	std::wstring toString();
	std::wstring toColoredString();
	std::wstring GetSelectedString(bool bExtraCase = true);
	bool SetTabSpace(int indent) { m_tabSpace = indent; }
	bool IsEmpty() { return m_string.empty(); }
	void Add(std::wstring);
	long long length();
	bool Insert(wchar_t wc);
	bool Back();
	bool Erase();
	bool InsertTab();
	bool Insert(wchar_t*);
	bool Insert(std::wstring);
	bool Insert(char*);
	bool Replace(wchar_t wc);
	bool Replace(std::wstring string);
	bool Copy(wchar_t* pSrc);
private:
	bool erase(bool back = false);
	std::wstring m_string;
	KsEditorCursorManager* m_pCursorManager;
	int m_tabSpace;

};
