#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include "KsEditorTextUtility.h"
#include <TextStor.h>

class KsEditorCursorManager
{
public:
	KsEditorCursorManager();
	virtual ~KsEditorCursorManager();

	long long GetStart() { return m_startPosition; };
	long long GetEnd() { return m_endPosition; };
	long long GetSelectLength() { return m_endPosition - m_startPosition; }
	long long GetCurrentLineStart() { return getCurrentLine(getCursorPosition())->m_index; }
	long long GetCurrentLineEnd() { return getCurrentLine(getCursorPosition())->m_index + getCurrentLine(getCursorPosition())->length(); }
	TsActiveSelEnd GetActiveSelEnd() { return m_activeSelEnd; }
	void SetActiveSelEnd(TsActiveSelEnd sel) { m_activeSelEnd = sel; }
	bool GetInterimChar() { return m_interimChar; }
	void SetInterimChar(bool interimChar) { m_interimChar = interimChar; }
	long long GetCaretLineIndex() { return getCurentInlineIndex(); }

	bool GetCaretIndex(long long* pStart, long long* pEnd);
	void SetText(const wchar_t* pText);
	void SetWindowHandle(HWND hWnd) { m_hWnd = hWnd; }

	bool IsSelecting() { return m_startPosition != m_endPosition; }
	bool IsTurning() { return m_startPosition > m_endPosition; }
	bool HasReturnCodeInSelectedLine(long long line);

	void SetNewPosition(long long start, long long end) { m_startPosition = start; m_endPosition = end; }
	// action
	bool Left(bool select = false, bool control = false, bool update = true);
	bool Right(bool select = false, bool control = false, bool update = true);
	bool Up(bool select = false, bool control = false, bool update = true);
	bool Down(bool select = false, bool control = false, bool update = true);
	bool Home(bool select = false, bool control = false, bool update = true);
	bool End(bool select = false, bool control = false, bool update = true);
	bool PageUp(bool select = false, bool control = false, bool update = true);
	bool PageDown(bool select = false, bool control = false, bool update = true);
	bool AllSelect(bool update = true);
	bool LButtonClick(unsigned short x, unsigned short y, unsigned short fontWidth, unsigned short fontHeight, bool shift, bool update = true);
	bool LButtonDrag(unsigned short x, unsigned short y, unsigned short fontWidth, unsigned short fontHeight, bool update = true); // case WM_MOUSEMOVE: if (wParam & MK_LBUTTON) { LButtonDrag(LOWORD(lParam), HIWORD(lParam)); }

private:
	class KsEditorTextBuffer {
	public:
		std::wstring Text() {
			std::wstring buffer = L"";
			buffer.insert(0, m_pStart);
			buffer = buffer.substr(0, m_pEnd - m_pStart);
			return buffer;
		}
		const wchar_t* m_pStart;
		const wchar_t* m_pEnd;
		KsEditorTextBuffer* m_pPrev;
		KsEditorTextBuffer* m_pNext;
		long long m_index;
		size_t length() { return m_pEnd - m_pStart; }
		size_t visibleLength() {
			int length = this->length();
			const wchar_t* pCurrent = m_pStart;
			size_t ret = 0;
			for (int i = 0; i < length; i++) {
				if (KsEditorTextUtility::IsHalf(pCurrent[i])) { ret++; }
				else { ret += 2; }
			}
			return ret;
		}
		KsEditorTextBuffer(KsEditorTextBuffer* pPrev = NULL) :
			m_pStart(NULL),
			m_pEnd(NULL),
			m_pPrev(pPrev),
			m_pNext(NULL),
			m_index(-1)
		{
		}
	};

	KsEditorTextBuffer* getCurrentLine(long long index);
	long long getCurentInlineIndex();
	long long getRealizeCursorIndex(KsEditorTextBuffer* pBuffer, long long visualizeInlineIndex);
	long long getVisualizeIndex(KsEditorTextBuffer* pBuffer, long long index);
	long long getCursorPosition();
	void updateInlineIndex();

	HWND m_hWnd;
	long long m_startPosition;
	long long m_endPosition;
	long long m_inlineIndex;
	const wchar_t* m_pText;
	long long m_length;
	std::vector<KsEditorTextBuffer*> m_buffer;
	TsActiveSelEnd m_activeSelEnd;
	bool m_interimChar;
};
