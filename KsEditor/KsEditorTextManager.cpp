#include "pch.h"
#include "KsEditorTextManager.h"
#include "Log.h"
#include <regex>

KsEditorTextManager::KsEditorTextManager() :
	m_pCursorManager(NULL),
	m_tabSpace(4)
{

}

KsEditorTextManager::~KsEditorTextManager()
{

}

std::wstring KsEditorTextManager::toString()
{
	return m_string;
}

std::wstring KsEditorTextManager::GetSelectedString( bool bExtraCase )
{
	std::wstring ret = L"";
	if (m_pCursorManager->IsSelecting()) {
		ret = m_string.substr(m_pCursorManager->GetStart(), m_pCursorManager->GetSelectLength());
	} else {
		if (bExtraCase && !m_string.empty()) {
			ret = m_string.substr(m_pCursorManager->GetCurrentLineStart(), m_pCursorManager->GetCurrentLineEnd());
		}
	}
	return ret;
}

std::wstring KsEditorTextManager::toColoredString()
{
	std::wstring ret = L"";
	std::wstring fsplit = m_string;
	std::wregex separator{ LR"((\s|\t|\n|\(|\)\[|\]|\{|\}|\<|\>|\"|\'|;|:|\?|\+|\*|\-|\/))" };
	auto itr = std::wsregex_token_iterator(fsplit.begin(), fsplit.end(), separator, -1);
	auto end = std::wsregex_token_iterator();
	int i = 0;
	while (itr != end) {
		std::wstring str = *itr;
		if (str == L"#include") {
			ret += L"#include";
			ret += L" ";
		} else {
			for (int i = 0; i < str.length() + 1; i++) {
				ret += L" ";
			}
		}
		itr++;
	}

	for (int i = 0; i < m_string.length(); i++) {
		if (m_string[i] == L'\n') {
			ret.replace(i, 1, L"\n");
		}
	}
	return ret;
}

void KsEditorTextManager::Add(std::wstring string)
{
}
bool KsEditorTextManager::Back()
{
	return erase(true);
}

bool KsEditorTextManager::Erase()
{
	return erase();
}

bool KsEditorTextManager::InsertTab()
{
	long long start = m_pCursorManager->GetCaretLineIndex();
	int indent = m_tabSpace - (start % m_tabSpace);
	if (indent == 0) { indent = m_tabSpace; }
	std::wstring str = L"";
	for (int i = 0; i < indent; i++) {
		str += L" ";
	}
	return Insert(str);

}

bool KsEditorTextManager::Insert(wchar_t c)
{
	std::wstring buf = L"";
	buf += c;
	bool ret = false;
	if (m_pCursorManager->IsSelecting()) {
		ret = Replace(c);
	} else {
		ret = Insert(buf);
	}
	return ret;
}

bool KsEditorTextManager::Insert(wchar_t *pStr)
{
	if (pStr == NULL) { return false; }
	std::wstring buf = pStr;
	bool ret = false;
	if (m_pCursorManager->IsSelecting()) {
		ret = Replace(buf);
	} else {
		ret = Insert(buf);
	}
	return ret;
}

bool KsEditorTextManager::Insert(char* pStr)
{
	if (pStr == NULL) { return false; }
	long long len = strlen(pStr) + 1;
	wchar_t* pWChar = new wchar_t[static_cast<unsigned int>(len)];
	std::mbstowcs(pWChar, pStr, static_cast<size_t>(len));
	bool ret = Insert(pWChar);
	delete[] pWChar;
	return ret;
}

bool KsEditorTextManager::Insert(std::wstring string)
{
	bool ret = true;
	std::wregex reg(L"\r\n");
	string = std::regex_replace(string, reg, L"\n");
	std::wregex reg2(L"\r");
	string = std::regex_replace(string, reg2, L"\n");
	m_string.insert(static_cast<unsigned int>(m_pCursorManager->GetStart()), string);
	m_pCursorManager->SetText(m_string.c_str());
	for (size_t i = 0; i < string.length(); i++) {
		m_pCursorManager->Right();
	}
	return ret;
}

bool KsEditorTextManager::Replace(wchar_t wc)
{
	wc = wc == '\r' ? '\n' : wc;
	m_string.replace(static_cast<unsigned int>(m_pCursorManager->GetStart()), static_cast<unsigned int>(m_pCursorManager->GetSelectLength()), 1, wc);
	m_pCursorManager->SetText(m_string.c_str());
	m_pCursorManager->SetNewPosition(m_pCursorManager->GetStart(), m_pCursorManager->GetStart());
	m_pCursorManager->SetActiveSelEnd(TS_AE_NONE);
	m_pCursorManager->Right();
	return true;
}

bool KsEditorTextManager::Replace(std::wstring string)
{
	std::wregex reg(L"\r\n");
	string = std::regex_replace(string, reg, L"\n");
	std::wregex reg2(L"\r");
	string = std::regex_replace(string, reg2, L"\n");
	m_string.replace(static_cast<unsigned int>(m_pCursorManager->GetStart()), static_cast<unsigned int>(m_pCursorManager->GetSelectLength()), string);
	m_pCursorManager->SetText(m_string.c_str());
	m_pCursorManager->Right();
	return true;
}


bool KsEditorTextManager::Copy(wchar_t* pSrc)
{
	m_string.copy(pSrc, static_cast<unsigned int>(m_pCursorManager->GetSelectLength()), static_cast<unsigned int>(m_pCursorManager->GetStart()));
	m_pCursorManager->SetText(m_string.c_str());
	return true;
}

long long KsEditorTextManager::length()
{
	return m_string.length();
}

bool KsEditorTextManager::erase(bool back)
{
	if (m_string.empty()) { return true; }

	bool ret = true;
	long long start = m_pCursorManager->GetStart();
	long long end = m_pCursorManager->GetEnd();
	if (m_pCursorManager->IsSelecting()) {
		m_string.erase(static_cast<unsigned int>(start), static_cast<unsigned int>(end - start));
	} else {
		if (start <= 0) {
			return true;
		}
		if (back) {
			m_string.erase(static_cast<unsigned int>(start - 1), 1);
			m_pCursorManager->Left();
		} else {
			m_string.erase(static_cast<unsigned int>(start), static_cast<unsigned int>(end - start));
		}
	}
	m_pCursorManager->SetText(m_string.c_str());

	return ret;
}

