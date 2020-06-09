#include "pch.h"
#include "KsEditorCursorManager.h"
#include "Log.h"

KsEditorCursorManager::KsEditorCursorManager() :
	m_hWnd(NULL),
	m_startPosition(0),
	m_endPosition(0),
	m_inlineIndex(0),
	m_pText(NULL),
	m_length(0),
	m_buffer(),
	m_activeSelEnd(TS_AE_NONE)
{
}

KsEditorCursorManager::~KsEditorCursorManager()
{

}

bool KsEditorCursorManager::GetCaretIndex(long long* pStart, long long* pEnd)
{
	return true;
}

bool KsEditorCursorManager::HasReturnCodeInSelectedLine(long long line)
{
	long long start = GetStart();
	long long length = GetSelectLength();
	KsEditorTextBuffer* pBuffer =  getCurrentLine(start);
	std::wstring buffer = L"";
	long long index = start - pBuffer->m_index;
	buffer = pBuffer->m_pStart + index;
	buffer = buffer.substr(0, length);
	LPCWSTR str = buffer.c_str();
	long long lineCount = 0;
	for (int i = 0; i < wcslen(str); i++) {
		if (str[i] == L'\n') {
			if (lineCount == line) {
				return true;
			}
			lineCount++;
		}
	}
	return false;
}


void KsEditorCursorManager::SetText(const wchar_t* pText)
{
	m_pText = pText;
	m_length = wcslen(m_pText);
	const wchar_t* pCurrent = m_pText;
	const wchar_t* pEnd = m_pText + m_length;
	m_buffer.clear();

	const wchar_t* pStart = pCurrent;
	KsEditorTextBuffer* pPrev = NULL;
	while (pCurrent != pEnd) {
		if (*pCurrent != '\n') {
			pCurrent++;
			continue;
		}
		pCurrent++;
		KsEditorTextBuffer* pBuffer = new KsEditorTextBuffer();
		pBuffer->m_index = pStart - m_pText;
		pBuffer->m_pStart = pStart;
		pBuffer->m_pEnd = pCurrent - 1;
		pBuffer->m_pPrev = pPrev;
		pBuffer->m_pNext = NULL;
		if (pPrev) { pPrev->m_pNext = pBuffer; }
		pPrev = pBuffer;
		m_buffer.push_back(pBuffer);
		pStart = pCurrent;
	}
	KsEditorTextBuffer* pBuffer = new KsEditorTextBuffer();
	pBuffer->m_index = pStart - m_pText;
	pBuffer->m_pStart = pStart;
	pBuffer->m_pEnd = pCurrent;
	pBuffer->m_pPrev = pPrev;
	pBuffer->m_pNext = NULL;
	if (pPrev) { pPrev->m_pNext = pBuffer; }
	m_buffer.push_back(pBuffer);
	InvalidateRect(m_hWnd, NULL, FALSE);
}

// action
bool KsEditorCursorManager::Left(bool select , bool control, bool update)
{
	if (select) {
		if (IsSelecting()) {
			if (m_activeSelEnd == TS_AE_START || m_activeSelEnd == TS_AE_NONE) {
				m_startPosition--;
			} else {
				if (m_endPosition > 0) {
					m_endPosition--;
					if (!IsSelecting()) {
						m_activeSelEnd = TS_AE_NONE;
					}
				}
			}
		} else {
			m_startPosition--;
			m_activeSelEnd = TS_AE_START;
		}
		if (m_startPosition <= 0) {
			m_startPosition = 0;
		}
		Log::Info(L"start:%lld end:%lld", m_startPosition, m_endPosition);
		updateInlineIndex();
		InvalidateRect(m_hWnd, NULL, FALSE);
		return true;
	}
	if (m_activeSelEnd == TS_AE_NONE) {
		m_startPosition--;
	} else {
		m_endPosition = m_startPosition;
	}
	if (m_startPosition <= 0) {
		m_startPosition = 0;
	}
	m_endPosition = m_startPosition;
	m_activeSelEnd = TS_AE_NONE;
	updateInlineIndex();

	if (update) { InvalidateRect(m_hWnd, NULL, FALSE); }
	return true;
}

bool KsEditorCursorManager::Right(bool select, bool control, bool update)
{
	if (select) {
		if (IsSelecting()) {
			if (m_activeSelEnd == TS_AE_END || m_activeSelEnd == TS_AE_NONE) {
				m_endPosition++;
			} else {
				m_startPosition++;
				if (!IsSelecting()) {
					m_activeSelEnd = TS_AE_NONE;
				}
			}
		} else {
			m_endPosition++;
			m_activeSelEnd = TS_AE_END;
		}
		if (m_endPosition >= m_length) {
			m_endPosition = m_length;
		}
		Log::Info(L"start:%lld end:%lld", m_startPosition, m_endPosition);
		updateInlineIndex();
		InvalidateRect(m_hWnd, NULL, FALSE);
		return true;
	}
	if (m_activeSelEnd == TS_AE_NONE) {
		m_startPosition++;
	} else {
		m_startPosition = m_endPosition;
	}
	if (m_startPosition >= m_length) {
		m_startPosition = m_length;
	}
	m_activeSelEnd = TS_AE_NONE;
	m_endPosition = m_startPosition;
	updateInlineIndex();
	if (update) { InvalidateRect(m_hWnd, NULL, FALSE); }
	return true;
}

bool KsEditorCursorManager::Up(bool select, bool control, bool update)
{
	KsEditorTextBuffer* pCurrent = getCurrentLine(getCursorPosition());
	if (pCurrent == NULL) { return false; }
	if (pCurrent->m_pPrev == NULL) {
		if (!select) {
			m_endPosition = m_startPosition;
			m_activeSelEnd = TS_AE_NONE;
			if (update) { InvalidateRect(m_hWnd, NULL, FALSE); }
		}
		return true;
	}
	long long index = getCurentInlineIndex();
	KsEditorTextBuffer* pPrev = pCurrent->m_pPrev;

	if (select) {
		if (IsSelecting()) {
			if (m_activeSelEnd == TS_AE_START || m_activeSelEnd == TS_AE_NONE) {
				if (pPrev->visibleLength() > index) {
					m_startPosition = pPrev->m_index + getVisualizeIndex(pPrev, index);
				} else {
					m_startPosition = pPrev->m_index + pPrev->length();
				}
			} else {
				if (m_endPosition > 0) {
					if (pPrev->visibleLength() > index) {
						m_endPosition = pPrev->m_index + getVisualizeIndex(pPrev, index);
					} else {
						m_endPosition = pPrev->m_index + pPrev->length();
					}
					if (!IsSelecting()) {
						m_activeSelEnd = TS_AE_NONE;
					} else if (IsTurning()) {
						long long tmp = m_startPosition;
						m_startPosition = m_endPosition;
						m_endPosition = tmp;
						m_activeSelEnd = TS_AE_START;
					}
				}
			}
		} else {
			if (pPrev->visibleLength() > index) {
				m_startPosition = pPrev->m_index + getVisualizeIndex(pPrev, index);
			} else {
				m_startPosition = pPrev->m_index + pPrev->length();
			}
			m_activeSelEnd = TS_AE_START;
		}
		if (m_startPosition <= 0) {
			m_startPosition = 0;
		}
	} else {
		if (pPrev->visibleLength() > index) {
			m_startPosition = pPrev->m_index + getVisualizeIndex(pPrev, index);
		} else {
			m_startPosition = pPrev->m_index + pPrev->length();
		}
		m_endPosition = m_startPosition;
		m_activeSelEnd = TS_AE_NONE;
	}

	Log::Info(L"start:%lld end:%lld", m_startPosition, m_endPosition);
	if (update) { InvalidateRect(m_hWnd, NULL, FALSE); }
	return true;

}

bool KsEditorCursorManager::Down(bool select, bool control, bool update)
{
	KsEditorTextBuffer* pCurrent = getCurrentLine(getCursorPosition());
	if (pCurrent == NULL) { return false; }
	if (pCurrent->m_pNext == NULL) {
		if (!select) {
			m_startPosition = m_endPosition;
			m_activeSelEnd = TS_AE_NONE;
			if (update) { InvalidateRect(m_hWnd, NULL, FALSE); }
		}
		return true;
	}
	long long index = getCurentInlineIndex();
	KsEditorTextBuffer* pNext = pCurrent->m_pNext;
	if (select) {
		if (IsSelecting()) {
			if (m_activeSelEnd == TS_AE_END || m_activeSelEnd == TS_AE_NONE) {
				if (pNext->visibleLength() > index) {
					m_endPosition = pNext->m_index + getVisualizeIndex(pNext, index);
				} else {
					m_endPosition = pNext->m_index + pNext->length();
				}
			} else {
				if (m_startPosition < m_length) {
					if (pNext->visibleLength() > index) {
						m_startPosition = pNext->m_index + getVisualizeIndex(pNext, index);
					} else {
						m_startPosition = pNext->m_index + pNext->length();
					}
					if (!IsSelecting()) {
						m_activeSelEnd = TS_AE_NONE;
					} else if (IsTurning()) {
						long long tmp = m_startPosition;
						m_startPosition = m_endPosition;
						m_endPosition = tmp;
						m_activeSelEnd = TS_AE_END;
					}
				}
			}
		} else {
			if (pNext->visibleLength() > index) {
				m_endPosition = pNext->m_index + getVisualizeIndex(pNext, index);
			} else {
				m_endPosition = pNext->m_index + pNext->length();
			}
			m_activeSelEnd = TS_AE_END;
		}
		if (m_endPosition > m_length) {
			m_startPosition = m_length;
		}
	} else {
		if (pNext->visibleLength() > index) {
			m_endPosition = pNext->m_index + getVisualizeIndex(pNext, index);
		} else {
			m_endPosition = pNext->m_index + pNext->length();
		}
		m_startPosition = m_endPosition;
		m_activeSelEnd = TS_AE_NONE;
	}

	Log::Info(L"start:%lld end:%lld", m_startPosition, m_endPosition);
	if (update) { InvalidateRect(m_hWnd, NULL, FALSE); }
	return true;
}
bool KsEditorCursorManager::Home(bool select, bool control, bool update)
{
	KsEditorTextBuffer* pCurrent = getCurrentLine(getCursorPosition());
	if (pCurrent == NULL) { return false; }

	if (select) {
		if (IsSelecting()) {
			if (m_activeSelEnd == TS_AE_START || m_activeSelEnd == TS_AE_NONE) {
				m_startPosition = pCurrent->m_index;
				if (control) { m_startPosition = 0; }
			} else {
				if (m_endPosition > 0) {
					m_endPosition = pCurrent->m_index;
					if (!IsSelecting()) {
						m_activeSelEnd = TS_AE_NONE;
					} else if (IsTurning()) {
						long long tmp = m_startPosition;
						m_startPosition = m_endPosition;
						if (control) { m_startPosition = 0; }
						m_endPosition = tmp;
						m_activeSelEnd = TS_AE_START;
					}
				}
			}
		} else {
			m_startPosition = pCurrent->m_index;
			if (control) { m_startPosition = 0; }
			m_activeSelEnd = TS_AE_START;
		}
		if (m_startPosition <= 0) {
			m_startPosition = 0;
		}
	} else {
		m_startPosition = pCurrent->m_index;
		if (control) { m_startPosition = 0; }
		m_endPosition = m_startPosition;
		m_activeSelEnd = TS_AE_NONE;
	}
	updateInlineIndex();
	if (update) { InvalidateRect(m_hWnd, NULL, FALSE); }
	return true;
}

bool KsEditorCursorManager::End(bool select, bool control, bool update)
{
	KsEditorTextBuffer* pCurrent = getCurrentLine(getCursorPosition());
	if (pCurrent == NULL) { return false; }
	if (select) {
		if (IsSelecting()) {
			if (m_activeSelEnd == TS_AE_END || m_activeSelEnd == TS_AE_NONE) {
				m_endPosition = pCurrent->m_index + pCurrent->length();
				if (control) { m_endPosition = m_length; }
			} else {
				if (m_startPosition < m_length) {
					m_startPosition = pCurrent->m_index + pCurrent->length();
					if (!IsSelecting()) {
						m_activeSelEnd = TS_AE_NONE;
					} else if (IsTurning()) {
						long long tmp = m_startPosition;
						m_startPosition = m_endPosition;
						m_endPosition = tmp;
						if (control) { m_endPosition = m_length; }
						m_activeSelEnd = TS_AE_END;
					}
				}
			}
		} else {
			m_endPosition = pCurrent->m_index + pCurrent->length();
			m_activeSelEnd = TS_AE_END;
		}
		if (m_endPosition > m_length) {
			m_startPosition = m_length;
		}
	} else {
		m_endPosition = pCurrent->m_index + pCurrent->length();
		if (control) { m_endPosition = m_length; }
		m_startPosition = m_endPosition;
		m_activeSelEnd = TS_AE_NONE;
	}

	updateInlineIndex();
	if (update) { InvalidateRect(m_hWnd, NULL, FALSE); }
	return true;
}

bool KsEditorCursorManager::AllSelect(bool update)
{
	m_startPosition = 0;
	m_endPosition = m_length;
	m_activeSelEnd = TS_AE_START;
	updateInlineIndex();
	if (update) { InvalidateRect(m_hWnd, NULL, FALSE); }
	return true;
}


bool KsEditorCursorManager::PageUp(bool select, bool control, bool update)
{
	return true;
}

bool KsEditorCursorManager::PageDown(bool select, bool control, bool update)
{
	return true;
}

bool KsEditorCursorManager::LButtonClick(unsigned short x, unsigned short y, unsigned short fontSize, unsigned short fontHeight, bool shift, bool update)
{
	auto itr = m_buffer.begin();
	KsEditorTextBuffer* pBuffer = reinterpret_cast<KsEditorTextBuffer*>(*itr);
	if (pBuffer == NULL) { return false; }
	for (int i = 0; i < y / fontHeight; i++) {
		if (pBuffer->m_pNext != NULL) {
			pBuffer = pBuffer->m_pNext;
		} else {
			return false;
		}
	}
	if (pBuffer == NULL) {
		return false;
	}
	LPCWSTR pWStr = pBuffer->m_pStart;
	long long index = pBuffer->m_index;
	int xCnt = 0;
	for (int i = 0; i < pBuffer->length(); i++) {
		if (KsEditorTextUtility::IsHalf(pWStr[i])) {
			xCnt += fontSize / 2;
		} else {
			xCnt += fontSize;
		}
		if (x == 0 && y == 0) { break; }
		index++;
		if (xCnt >= x) {
			break;
		}
	}
	if (shift) {
		if (index < m_startPosition) {
			if (m_activeSelEnd == TS_AE_NONE || m_activeSelEnd == TS_AE_START) {
				m_startPosition = index;
			} else {
				m_endPosition = index;
				std::swap(m_startPosition, m_endPosition);
				m_activeSelEnd = TS_AE_START;
			}
		} else if (index > m_endPosition) {
			if (m_activeSelEnd == TS_AE_NONE || m_activeSelEnd == TS_AE_END) {
				m_endPosition = index;
			} else {
				m_startPosition = index;
				std::swap(m_startPosition, m_endPosition);
				m_activeSelEnd = TS_AE_END;
			}
		} else {
			m_startPosition = index;
			m_endPosition = index;
			m_activeSelEnd = TS_AE_NONE;
		}
	} else {
		m_startPosition = index;
		m_endPosition = index;
		m_activeSelEnd = TS_AE_NONE;
	}

	if (update) { InvalidateRect(m_hWnd, NULL, FALSE); }
	return true;
}

bool KsEditorCursorManager::LButtonDrag(unsigned short x, unsigned short y, unsigned short fontSize, unsigned short fontHeight, bool update)
{
	auto itr = m_buffer.begin();
	KsEditorTextBuffer* pBuffer = reinterpret_cast<KsEditorTextBuffer*>(*itr);
	if (pBuffer == NULL) { return false; }
	for (int i = 0; i < y / fontHeight; i++) {
		if (pBuffer->m_pNext != NULL) {
			pBuffer = pBuffer->m_pNext;
		} else {
			return false;
		}
	}
	if (pBuffer == NULL) {
		return false;
	}
	LPCWSTR pWStr = pBuffer->m_pStart;
	long long index = pBuffer->m_index;
	int xCnt = 0;
	for (int i = 0; i < pBuffer->length(); i++) {
		if (KsEditorTextUtility::IsHalf(pWStr[i])) {
			xCnt += fontSize / 2;
		} else {
			xCnt += fontSize;
		}
		if (x == 0 && y == 0) { break; }
		index++;
		if (xCnt >= x) {
			break;
		}
	}
	if (index < m_startPosition) {
		if (m_activeSelEnd == TS_AE_NONE || m_activeSelEnd == TS_AE_START) {
			m_startPosition = index;
			m_activeSelEnd = TS_AE_START;
		} else {
			m_endPosition = index;
			std::swap(m_startPosition, m_endPosition);
			m_activeSelEnd = TS_AE_START;
		}
	} else if (index > m_endPosition) {
		if (m_activeSelEnd == TS_AE_NONE || m_activeSelEnd == TS_AE_END) {
			m_endPosition = index;
			m_activeSelEnd = TS_AE_END;
		} else {
			m_startPosition = index;
			std::swap(m_startPosition, m_endPosition);
			m_activeSelEnd = TS_AE_END;
		}
	} else if (m_startPosition < index && index < m_endPosition) {
		if (m_activeSelEnd == TS_AE_NONE) {
			//?
		} else if (m_activeSelEnd == TS_AE_END) {
			m_endPosition = index;
		} else if (m_activeSelEnd == TS_AE_START) {
			m_startPosition = index;
		}
	}

	if (update) { InvalidateRect(m_hWnd, NULL, FALSE); }
	return true;
}


KsEditorCursorManager::KsEditorTextBuffer* KsEditorCursorManager::getCurrentLine(long long index)
{
	for (auto itr = m_buffer.begin(); itr != m_buffer.end(); ++itr) {
		KsEditorTextBuffer* pTextBuffer = (*itr);
		long long length = pTextBuffer->length();
		if (pTextBuffer->m_index <= index && index <= pTextBuffer->m_index + length) {
			return pTextBuffer;
		}
	}
	return NULL;
}

void KsEditorCursorManager::updateInlineIndex()
{
	KsEditorTextBuffer* pCurrent = NULL;
	pCurrent = getCurrentLine(getCursorPosition());
	if (pCurrent == NULL) { return; }
	m_inlineIndex = 0;
	for (int i = 0; i < getCursorPosition() - pCurrent->m_index; i++) {
		if (KsEditorTextUtility::IsHalf(pCurrent->m_pStart[i])) {
			m_inlineIndex++;
		} else {
			m_inlineIndex += 2;
		}
	}
}

long long KsEditorCursorManager::getCurentInlineIndex()
{
	return m_inlineIndex;
}


long long KsEditorCursorManager::getRealizeCursorIndex(KsEditorTextBuffer* pBuffer, long long visualizeInlineIndex)
{
	long long ret = 0;
	if (pBuffer == NULL) { return -1; }
	if (visualizeInlineIndex == 0) { return 0; }
	for (size_t i = 0; i < pBuffer->length(); i++) {
		if (KsEditorTextUtility::IsHalf(pBuffer->m_pStart[i])) {
			ret++;
		} else {
			ret += 2;
		}
		if (visualizeInlineIndex <= ret) {
			break;
		}
	}
	return ret;
}

long long KsEditorCursorManager::getVisualizeIndex(KsEditorTextBuffer* pBuffer, long long inlineIndex)
{
	long long ret = 0;
	if (pBuffer == NULL) { return -1; }
	if (inlineIndex == 0) { return 0; }
	for (int i = 0; ret < pBuffer->length(); ret++) {
		if (KsEditorTextUtility::IsHalf(pBuffer->m_pStart[ret])) {
			i++;
		} else {
			i += 2;
		}
		if (inlineIndex <= i) {
			break;
		}
	}
	return ret + 1;
}

long long KsEditorCursorManager::getCursorPosition()
{
	if (m_activeSelEnd == TS_AE_END) {
		return m_endPosition;
	}
	return m_startPosition;
}
