#include "pch.h"
#include <olectl.h>
#include <thread>
#include <msctf.h>
#include <wrl.h>
#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include "KsEditor.h"
#include "Log.h"
#undef min

using Microsoft::WRL::ComPtr;

KsEditor::KsEditor() :
	m_d2d(),
	m_tbuilder(),
	m_drawer(),
	m_docmgr(),
	m_attr_prop(),
	m_edit_cookie(),
	m_category_mgr(),
	m_attribute_mgr(),
	m_context(),
	m_threadmgr(),
	m_clientId(0),
	m_hInstance(NULL),
	m_parent(NULL),
	m_hWnd(NULL),
	m_request_lock_async(),
	m_queue_lock(),
	m_write_queue(),
	m_read_queue(),
	m_caret_display(true),
	m_caret_update_time(),
	m_sink(),
	m_textManager(),
	m_sinkmask(0),
	m_ref_cnt(0),
	m_lock(),
	m_composition(false),
	m_pKeyboardManager(new KeyboardManager()),
	m_pCursorManager(new KsEditorCursorManager()),
	m_pScrollBar(new KsEditorScrollBar())
{
	m_pKeyboardManager->AddCallback(this);
	m_textManager.SetCursorManager(m_pCursorManager);

}

KsEditor::~KsEditor()
{
	delete m_pKeyboardManager;
	m_pKeyboardManager = NULL;
	delete m_pCursorManager;
	m_pCursorManager = NULL;
	m_docmgr->Pop(0);
}

HRESULT KsEditor::RegisterConsoleWindowClass(HINSTANCE hinst)
{
	WNDCLASSEX wcex;

	ZeroMemory((LPVOID)&wcex, sizeof(WNDCLASSEX));

	//ÉEÉBÉìÉhÉEÉNÉâÉXÇìoò^
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc = KsEditor::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hinst;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = className;
	wcex.hIconSm = NULL;
	HRESULT result = RegisterClassEx(&wcex);
	return result;
}

LRESULT CALLBACK KsEditor::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static KsEditor* self;
	if (self) {
		self->CaretUpdate();
	}
	switch (message) {
		case WM_CREATE:
			AddClipboardFormatListener(hWnd);
			self = static_cast<KsEditor*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
			break;
		case WM_TIMER:
			self->OnTimer();
			break;
		case WM_SETFOCUS:
			self->OnSetFocus();
			break;
		case WM_KEYDOWN:
			self->OnKeyDown(wParam);
			break;
		case WM_CHAR:
			self->OnChar(wParam);
			break;
		case WM_PAINT:
			self->OnPaint();
			return 0;
		case WM_SIZE:
			self->OnSize();
			break;
		case WM_LBUTTONDBLCLK:
			// word select
			break;
		case WM_MOUSEMOVE:
			self->OnMouseMove(LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_LBUTTONUP:
			self->OnLButtonUp(LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_LBUTTONDOWN:
			self->OnLButtonDown(LOWORD(lParam), HIWORD(lParam), (wParam&MK_SHIFT));
			break;
		case WM_MOUSEWHEEL:
			break;
		case WM_CLIPBOARDUPDATE:
//			self->OnPaste();
			break;
		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;
		case WM_DESTROY:
			RemoveClipboardFormatListener(hWnd);
			PostQuitMessage(0);
			break;
//			Log::Detail(self->m_string.c_str());
		case WM_ACTIVATE:
		case WM_ACTIVATEAPP:
			self->m_pKeyboardManager->SetActive((wParam&0xFFFF) != WA_INACTIVE);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;

}

void KsEditor::Init(int x, int y, int w, int h, HMENU m, ID2D1Factory* d2d_f,IDWriteFactory* dwrite_f)
{
	HRESULT hr;
	hr = m_threadmgr->CreateDocumentMgr(&m_docmgr);
	if (FAILED(hr)) {
		throw hr;
	}
	hr = m_docmgr->CreateContext(m_clientId, 0, dynamic_cast<ITextStoreACP*>(this), &m_context, &m_edit_cookie);
	if (FAILED(hr)) {
		throw hr;
	}
	hr = m_context->GetProperty(GUID_PROP_ATTRIBUTE, &m_attr_prop);
	if (FAILED(hr)) {
		throw hr;
	}
	hr = RegisterConsoleWindowClass(m_hInstance);
	if (FAILED(hr)) {
		throw hr;
	}
	m_hWnd = CreateWindowEx(0, className, NULL, WS_OVERLAPPED | WS_CHILD | WS_VISIBLE, x, y, w, h, m_parent, m, m_hInstance, this);
	m_pCursorManager->SetWindowHandle(m_hWnd);
	m_pScrollBar->Create(m_hWnd, m_hInstance);
	m_d2d = Direct2DWithHWnd::Create(d2d_f, m_hWnd);
	TsfDWriteDrawer::Create(m_d2d->GetFactory(), &m_drawer);
	m_tbuilder = std::make_unique<TextBuilder>(dwrite_f,
		L"ÇlÇr ÉSÉVÉbÉN",
		DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_SEMI_EXPANDED,
		static_cast<FLOAT>(20),
		L"ja-jp");
	hr = m_docmgr->Push(m_context.Get());
	if (FAILED(hr)) {
		throw hr;
	}
	SetTimer(m_hWnd, CallAsyncTimerId, 60u, NULL);
}

void KsEditor::OnSetFocus()
{
	Log::Detail(L"KsEditor::%s()", __FUNCTIONW__);
	if (FAILED(m_threadmgr->SetFocus(m_docmgr.Get()))) {
		Log::Detail(L"KsEditor::%s() Fail", __FUNCTIONW__);
	}
}

void KsEditor::OnSize()
{
	if (m_d2d) {
		m_d2d->ReSize();
	}
	if (m_sink) {
		m_sink->OnLayoutChange(TS_LC_CHANGE, 0UL);
	}
}

void KsEditor::OnPaste()
{
	if (IsClipboardFormatAvailable(CF_UNICODETEXT) == false) {
		return;
	}
	OpenClipboard(NULL);
	HANDLE hClip = GetClipboardData(CF_UNICODETEXT);
	if (hClip == NULL) {
		CloseClipboard();
		return;
	}
	wchar_t* pBuffer = reinterpret_cast<wchar_t*>(GlobalLock(hClip));
	m_textManager.Insert(pBuffer);
	GlobalUnlock(hClip);
	CloseClipboard();
}

void KsEditor::CopyToClipboard(std::wstring str)
{
	if (str.length() <= 0) {
		return;
	}
	OpenClipboard(NULL);
	HANDLE hClip = GlobalAlloc(GHND | GMEM_SHARE, (str.length() + 1) * sizeof(wchar_t));
	if (hClip == NULL) {
		CloseClipboard();
		return;
	}
	LPWSTR lpWStr = reinterpret_cast<LPWSTR>(GlobalLock(hClip));
	if (lpWStr == NULL) { return; }
	memset(lpWStr, 0, (str.length() + 1) * sizeof(wchar_t));
	memcpy(lpWStr, str.c_str(), str.length() * sizeof(wchar_t));
	SetClipboardData(CF_UNICODETEXT, lpWStr);
	CloseClipboard();
}

void KsEditor::OnMouseMove(int x, int y)
{
	if (GetCapture() != m_hWnd) {
		return;
	}
	RECT rc;
	GetClientRect(m_hWnd, &rc);

	BOOL isTrailingHit;
	BOOL isInside;
	DWRITE_HIT_TEST_METRICS caretMetrics;
	auto layout = m_tbuilder->CreateTextLayout(m_textManager.toString(), static_cast<FLOAT>(rc.right - rc.left), static_cast<FLOAT>(rc.bottom - rc.top));

	layout->HitTestPoint(
		x,
		y,
		&isTrailingHit,
		&isInside,
		&caretMetrics
	);
	FLOAT fSize = layout->GetFontSize();

	WORD xpos = caretMetrics.left;
	WORD ypos = caretMetrics.top;
	if (isTrailingHit) { xpos += caretMetrics.width; }
	m_pCursorManager->LButtonDrag(xpos, ypos, static_cast<int>(fSize), caretMetrics.height);
	Log::Info(L"Click RECT{%f, %f, %f, %f} trailing:%d inside:%d size:%f", caretMetrics.left, caretMetrics.top, caretMetrics.width, caretMetrics.height, isTrailingHit, isInside, fSize);

}

void KsEditor::OnLButtonUp(int x, int y)
{
	ReleaseCapture();
}

void KsEditor::OnLButtonDown(int x, int y, bool shift)
{
	SetCapture(m_hWnd);
	RECT rc;
	GetClientRect(m_hWnd, &rc);

	BOOL isTrailingHit;
	BOOL isInside;
	DWRITE_HIT_TEST_METRICS caretMetrics;
	auto layout = m_tbuilder->CreateTextLayout(m_textManager.toString(), static_cast<FLOAT>(rc.right - rc.left), static_cast<FLOAT>(rc.bottom - rc.top));

	layout->HitTestPoint(
		x,
		y,
		&isTrailingHit,
		&isInside,
		&caretMetrics
	);
	FLOAT fSize = layout->GetFontSize();

	WORD xpos = caretMetrics.left;
	WORD ypos = caretMetrics.top;
	if (isTrailingHit) { xpos += caretMetrics.width; }
	m_pCursorManager->LButtonClick(xpos, ypos, static_cast<int>(fSize), caretMetrics.height, shift);
	Log::Info(L"Click RECT{%f, %f, %f, %f} trailing:%d inside:%d size:%f", caretMetrics.left, caretMetrics.top, caretMetrics.width, caretMetrics.height, isTrailingHit, isInside, fSize);

	return;
}


void KsEditor::OnChar(WPARAM wp)
{
	TCHAR c = static_cast<TCHAR>(wp);
#ifdef UNICODE
	wchar_t wc = c;
#else
	wchar_t wc = std::btowc(c);
#endif
	CallWithAppLock(true, [this, wc]()->void {
		switch (wc) {
			case 0x00: break; // NULL
			case 0x01: break; // Ctrl+A
			case 0x02: break; // Ctrl+B
			case 0x03: break; // Ctrl+V
			case 0x04: break; // Ctrl+D
			case 0x05: break; // Ctrl+E
			case 0x06: break; // Ctrl+F
			case 0x07: break; // Ctrl+G
			case 0x08://back space
				m_textManager.Back();
				break;
			case 0x09:
				m_textManager.InsertTab();
				break; // Tab
//			case 0x0A: break; // \n
			case 0x0B: break; // Ctrl+K
			case 0x0C: break; // Page Down?
//			case 0x0D: break; // \r
			case 0x0E: break; // Ctrl+N
			case 0x0F: break; // Ctrl+O
			case 0x10: break; // Ctrl+P
			case 0x11: break; // Ctrl+Q
			case 0x12: break; // Ctrl+R
			case 0x13: break; // Ctrl+S
			case 0x14: break; // Ctrl+T
			case 0x15: break; // Ctrl+U
			case 0x16: break; // Ctrl+V
			case 0x17: break; // Ctrl+W
			case 0x18: break; // Ctrl+X
			case 0x19: break; // Ctrl+Y
			case 0x1A: break; // Ctrl+Z
			case 0x1B: break; // Ctrl+[
			case 0x1C: break; // Ctrl+Åè
			case 0x1D: break; // Ctrl+]
			case 0x1E: break; // Ctrl+^
			case 0x1F: break; // Ctrl+_
			default:
				m_textManager.Insert(wc);
		}
		m_sink->OnSelectionChange();
		UpdateText();
	});
}

void KsEditor::OnTimer()
{
	CallAsync();
}

void KsEditor::CaretUpdate()
{
}

void KsEditor::OnKeyDown(WPARAM param)
{
}

void KsEditor::UpdateText()
{
	InvalidateRect(m_hWnd, NULL, FALSE);
}

static constexpr const inline LineStyle convertLineStyle(TF_DA_LINESTYLE ls) {
	switch (ls) {
		case TF_LS_SOLID:
			return LineStyle_Solid;
		case TF_LS_DOT:
			return LineStyle_Dot;
		case TF_LS_DASH:
			return LineStyle_Dash;
		case TF_LS_SQUIGGLE:
			return LineStyle_Squiggle;
		default:
			throw "NotSupported";
	}
}

template <class R>
static inline ComPtr<R> convertColor(TF_DA_COLOR& color, ID2D1RenderTarget* t, R* ifNone)
{
	ComPtr<R> r;
	switch (color.type) {
		case TF_CT_NONE:
			return ifNone;
		case TF_CT_COLORREF:
			t->CreateSolidColorBrush(D2D1::ColorF(color.cr, 1.0f), &r);
			return r;
		case TF_CT_SYSCOLOR:
			t->CreateSolidColorBrush(D2D1::ColorF(GetSysColor(color.nIndex), 1.0f), &r);
			return r;
		default:
			return ifNone;
	}
}

void KsEditor::OnPaint()
{
	CallWithAppLock(false, [this]() {
		Log::Info(L"OnPaint()");
		HRESULT hr;
		PAINTSTRUCT pstruct;
		RECT rc;

		GetClientRect(m_hWnd, &rc);

		auto t = m_d2d->GetRenderTarget();

		t->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

//		ComPtr<ID2D1SolidColorBrush> current;
//		t->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Pink, 1.0f), &current);


		ComPtr<ID2D1SolidColorBrush> textColor;
		t->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 1.0f), &textColor);

		ComPtr<ID2D1SolidColorBrush> textColor2;
		t->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightBlue, 1.0f), &textColor2);

		ComPtr<ID2D1SolidColorBrush> clearColorBrush;
		static auto clearColor = D2D1::ColorF(D2D1::ColorF::Black, 1.0f);
		t->CreateSolidColorBrush(clearColor, &clearColorBrush);

		BeginPaint(m_hWnd, &pstruct);

		t->BeginDraw();
		t->Clear(clearColor);

		DrawString(t, m_textManager.toString(), textColor, true);
		DrawString(t, m_textManager.toColoredString(), textColor2);

		hr = t->EndDraw();
		if (FAILED(hr)) {
			throw hr;
		}
		EndPaint(m_hWnd, &pstruct);
	});
}

void KsEditor::DrawString(ID2D1RenderTarget* target, std::wstring string, ComPtr<ID2D1SolidColorBrush> textColor, bool drawCaret)
{
	ComPtr<ID2D1SolidColorBrush> caret;
	target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 1.0f), &caret);
	ComPtr<ID2D1SolidColorBrush> selection;
	target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.5f), &selection);
	ComPtr<ID2D1SolidColorBrush> transparency;
	target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 0.0f), &transparency);

	RECT rc;
	GetClientRect(m_hWnd, &rc);

	auto layout = m_tbuilder->CreateTextLayout(string, static_cast<FLOAT>(rc.right - rc.left), static_cast<FLOAT>(rc.bottom - rc.top));
	DWRITE_HIT_TEST_METRICS hitTestMetrics;
	bool isTrailingHit = false;
	float caretX, caretY;
	UINT32 position = static_cast<UINT32>(m_pCursorManager->GetActiveSelEnd() == TS_AE_END ? m_pCursorManager->GetEnd() : m_pCursorManager->GetStart());
	HRESULT hr = layout->HitTestTextPosition(position, isTrailingHit, &caretX, &caretY, &hitTestMetrics);

	if (FAILED(hr)) {
		throw hr;
	}
	DWORD caretWidth = 1;
	hr = SystemParametersInfo(SPI_GETCARETWIDTH, 0, &caretWidth, 0);
	if (FAILED(hr)) {
		throw hr;
	}
	DWORD halfCaretWidth = caretWidth / 2u;

	if (drawCaret) {
		target->FillRectangle({ caretX - halfCaretWidth, hitTestMetrics.top, caretX + (caretWidth - halfCaretWidth), hitTestMetrics.top + hitTestMetrics.height }, caret.Get());

		if (m_pCursorManager->IsSelecting()) {
			UINT32 count;
			layout->HitTestTextRange(static_cast<UINT32>(m_pCursorManager->GetStart()), static_cast<UINT32>(m_pCursorManager->GetSelectLength()), 0, 0, NULL, 0, &count);

			std::unique_ptr<DWRITE_HIT_TEST_METRICS[]> mats(new DWRITE_HIT_TEST_METRICS[count]);
			hr = layout->HitTestTextRange(static_cast<UINT32>(m_pCursorManager->GetStart()), static_cast<UINT32>(m_pCursorManager->GetSelectLength()), 0, 0, mats.get(), count, &count);
			if (FAILED(hr)) {
				throw hr;
			}


			for (auto i = 0UL; i < count; i++) {
				if (m_pCursorManager->HasReturnCodeInSelectedLine(i)) {
					mats[i].width += 8.0f;
				}
				Log::Info(L"mats[%d] {%f, %f, %f, %f}", i, mats[i].left, mats[i].top, mats[i].width, mats[i].height);
				target->FillRectangle({ mats[i].left, mats[i].top, mats[i].left + mats[i].width, mats[i].top + mats[i].height }, selection.Get());
			}

		}
	}

	//draw string
	ComPtr<IEnumTfRanges> enumRanges;
	hr = m_attr_prop->EnumRanges(m_edit_cookie, &enumRanges, NULL);
	if (FAILED(hr)) {
		throw hr;
	}

	ComPtr<ITfRange> range;
	while (enumRanges->Next(1, &range, NULL) == S_OK) {
		VARIANT var;
		try {
			VariantInit(&var);
			if (!(m_attr_prop->GetValue(m_edit_cookie, range.Get(), &var) == S_OK && var.vt == VT_I4)) {
				continue;
			}
			GUID guid;
			hr = m_category_mgr->GetGUID((TfGuidAtom)var.lVal, &guid);
			if (FAILED(hr)) {
				throw hr;
			}
			ComPtr<ITfDisplayAttributeInfo> dispattrinfo;
			hr = m_attribute_mgr->GetDisplayAttributeInfo(guid, &dispattrinfo, NULL);
			if (FAILED(hr)) {
				throw hr;
			}
			TF_DISPLAYATTRIBUTE attr;
			dispattrinfo->GetAttributeInfo(&attr);
			//attrÇ…ëÆê´Ç™ì¸Ç¡ÇƒÇ¢ÇÈÇÃÇ≈ëÆê´Ç…äÓÇ√Ç¢Çƒï`âÊÇ≥ÇπÇÈ
			ComPtr<TsfDWriteDrawerEffect> effect = new TsfDWriteDrawerEffect(
				convertColor(attr.crBk, target, transparency.Get()).Get(),
				convertColor(attr.crText, target, textColor.Get()).Get(),
				attr.lsStyle == TF_LS_NONE ? std::unique_ptr<TsfDWriteDrawerEffectUnderline>() : std::make_unique<TsfDWriteDrawerEffectUnderline>(convertLineStyle(attr.lsStyle),
					static_cast<bool>(attr.fBoldLine),
					convertColor(attr.crLine, target, textColor.Get()).Get())
			);

			ComPtr<ITfRangeACP> rangeAcp;
			range.As(&rangeAcp);
			LONG start, length;
			if (FAILED(rangeAcp->GetExtent(&start, &length))) {
				continue;
			}
			DWRITE_TEXT_RANGE write_range{ static_cast<UINT32>(start), static_cast<UINT32>(length) };
			layout->SetDrawingEffect(effect.Get(), write_range);
			layout->SetUnderline(effect->underline ? TRUE : FALSE, write_range);
			VariantClear(&var);
		}
		catch (...) {
			VariantClear(&var);
			throw;
		}
	}
	ComPtr<TsfDWriteDrawerEffect> defaultEffect = new TsfDWriteDrawerEffect{
		transparency.Get(),
		textColor.Get(),
		std::unique_ptr<TsfDWriteDrawerEffectUnderline>()
	};
	auto context = std::make_unique<TsfDWriteDrawerContext>(target, defaultEffect.Get());
	hr = layout->Draw(context.get(), m_drawer.Get(), 0, 0);
	if (FAILED(hr)) {
		throw hr;
	}

}

void KsEditor::Create(HINSTANCE hinst, HWND pwnd, int x, int y, int w, int h, HMENU hmenu, ITfThreadMgr* threadmgr, TfClientId cid, ITfCategoryMgr* cate_mgr, ITfDisplayAttributeMgr* attr_mgr, ID2D1Factory* d2d, IDWriteFactory* dwrite_f, KsEditor** pr) {
	auto r = new KsEditor();
	r->AddRef();
	r->m_hInstance = hinst;
	r->m_parent = pwnd;
	r->m_threadmgr = threadmgr;
	r->m_clientId = cid;
	r->m_category_mgr = cate_mgr;
	r->m_attribute_mgr = attr_mgr;
	r->m_caret_update_time = std::chrono::steady_clock::now();
	r->Init(x, y, w, h, hmenu, d2d, dwrite_f);
	*pr = r;
}


HRESULT STDMETHODCALLTYPE KsEditor::GetWnd(
	TsViewCookie vcView,
	HWND* phwnd
)
{
	if (vcView == 1) {
		*phwnd = m_hWnd;
		return S_OK;
	}
	else {
		return E_INVALIDARG;
	}
}

HRESULT KsEditor::AdviseSink(
	REFIID   riid,
	IUnknown* punk,
	DWORD    dwMask
)
{
	if (riid != IID_ITextStoreACPSink)return E_INVALIDARG;
	if (m_sink) {
		//ä˘Ç…ìoò^Ç≥ÇÍÇƒÇ¢ÇÈÅB
		if (m_sink.Get() == punk) {
			//ìoò^Ç≥ÇÍÇƒÇÈÇÃÇ∆Ç®ÇÒÇ»Ç∂
			m_sinkmask = dwMask;
			return S_OK;
		}
		else
		{
			//ìoò^Ç≥ÇÍÇƒÇÈÇÃÇ∆à·Ç§ÇÃ
			return CONNECT_E_ADVISELIMIT;
		}
	}
	else
	{
		//ìoò^
		auto hr = punk->QueryInterface(IID_PPV_ARGS(&m_sink));
		if (FAILED(hr))return E_INVALIDARG;
		m_sinkmask = dwMask;
		return S_OK;
	}
}

HRESULT KsEditor::UnadviseSink( IUnknown* punk )
{
	if (m_sink.Get() == punk) {
		m_sink.Reset();
		m_sinkmask = NULL;
		return S_OK;
	} else {
		return CONNECT_E_NOCONNECTION;
	}
}

HRESULT KsEditor::RequestLock(DWORD dwLockFlags, HRESULT* phrSession)
{
	Log::Detail(L"KsEditor::%s()", __FUNCTIONW__);

	if (!m_sink) {
		return E_UNEXPECTED;
	}

	if ((dwLockFlags & TS_LF_SYNC) == TS_LF_SYNC) {
		Log::Detail(L"KsEditor::%s() SyncLock", __FUNCTIONW__);

		auto wr = (dwLockFlags & TS_LF_READWRITE) == TS_LF_READWRITE;
		//ìØä˙ÉçÉbÉN
		if (wr) {
			std::optional<HRESULT> r = m_lock.TryWriteLockToCallTS([this, dwLockFlags]()->HRESULT {
				return m_sink->OnLockGranted(dwLockFlags);
			});
			*phrSession = r.value_or(TS_E_SYNCHRONOUS);
		} else {
			std::optional<HRESULT> r = m_lock.TryReadLockToCallTS([this, dwLockFlags]() -> HRESULT {
				return m_sink->OnLockGranted(dwLockFlags);
			});
			*phrSession = r.value_or(TS_E_SYNCHRONOUS);
		}
	} else {
		//îÒìØä˙ÉçÉbÉN
		Log::Detail(L"KsEditor::%s() AsyncLock", __FUNCTIONW__);
		RequestAsyncLock(dwLockFlags);

		*phrSession = TS_S_ASYNC;
	}
	CallAsync();
	return S_OK;
}

HRESULT KsEditor::GetStatus( TS_STATUS* pdcs )
{
	Log::Detail(L"KsEditor::%s()", __FUNCTIONW__);

	if (pdcs == 0) {
		return E_INVALIDARG;
	}
	pdcs->dwDynamicFlags = 0;
	pdcs->dwStaticFlags = TS_SS_NOHIDDENTEXT;
	return S_OK;
}

HRESULT KsEditor::GetActiveView( TsViewCookie* pvcView )
{
	Log::Detail(L"KsEditor::%s()", __FUNCTIONW__);

	*pvcView = 1;
	return S_OK;
}

HRESULT KsEditor::QueryInsert( LONG  acpTestStart, LONG  acpTestEnd, ULONG cch, LONG* pacpResultStart, LONG* pacpResultEnd )
{
	Log::Detail(L"KsEditor::%s()", __FUNCTIONW__);

	if (acpTestStart < 0 || acpTestStart > acpTestEnd || acpTestEnd > static_cast<LONG>(m_textManager.length())) {
		return  E_INVALIDARG;
	} else {
		*pacpResultStart = acpTestStart;
		*pacpResultEnd = acpTestEnd;
		return S_OK;
	}
}

HRESULT KsEditor::GetSelection(ULONG ulIndex, ULONG ulCount, TS_SELECTION_ACP* pSelection, ULONG* pcFetched)
{
	Log::Detail(L"KsEditor::%s()", __FUNCTIONW__);
	if (!m_lock.IsLock(false)) {
		return TS_E_NOLOCK;
	}

	if (pcFetched == 0) {
		return E_INVALIDARG;
	}

	*pcFetched = 0;
	if (pSelection == 0) {
		return E_INVALIDARG;
	}

	if (ulIndex == TF_DEFAULT_SELECTION) {
		ulIndex = 0;
	} else if (ulIndex > 1) {
		return E_INVALIDARG;
	}

	pSelection[0].acpStart = static_cast<LONG>(m_pCursorManager->GetStart());
	pSelection[0].acpEnd = static_cast<LONG>(m_pCursorManager->GetEnd());
	pSelection[0].style.fInterimChar = m_pCursorManager->GetInterimChar();
	if (m_pCursorManager->GetInterimChar()) {
		pSelection[0].style.ase = TS_AE_NONE;
	} else {
		pSelection[0].style.ase = m_pCursorManager->GetActiveSelEnd();
	}
	*pcFetched = 1;

	return S_OK;
}

HRESULT KsEditor::SetSelection( ULONG ulCount, const TS_SELECTION_ACP* pSelection )
{
	Log::Detail(L"KsEditor::%s()", __FUNCTIONW__);

	if (!m_lock.IsLock(true)) {
		return TS_E_NOLOCK;
	}
	if (pSelection == 0) {
		return E_INVALIDARG;
	}

	if (ulCount > 1) {
		return E_INVALIDARG;
	}
	if (pSelection->acpStart < 0) {
		return E_INVALIDARG;
	}
	if (static_cast<size_t>(pSelection->acpEnd) > m_textManager.length()) {
		return E_INVALIDARG;
	}
	m_pCursorManager->SetNewPosition(pSelection->acpStart, pSelection->acpEnd);
	m_pCursorManager->SetInterimChar(pSelection->style.fInterimChar);
	m_pCursorManager->SetActiveSelEnd(pSelection->style.ase);

	//UpdateText();
	return S_OK;
}

HRESULT KsEditor::GetText( LONG acpStart, LONG acpEnd, WCHAR* pchPlain, ULONG cchPlainReq, ULONG* pcchPlainRet, TS_RUNINFO* prgRunInfo, ULONG cRunInfoReq, ULONG* pcRunInfoRet, LONG* pacpNext )
{
	Log::Detail(L"KsEditor::%s()", __FUNCTIONW__);

	if (!m_lock.IsLock(false)) {
		return TS_E_NOLOCK;
	}

	if ((cchPlainReq == 0) && (cRunInfoReq == 0)) {
		return S_OK;
	}

	if (acpEnd == -1) {
		acpEnd = static_cast<LONG>(m_textManager.length());
	}

	acpEnd = std::min(acpEnd, acpStart + (int)cchPlainReq);
	if (acpStart != acpEnd) {
#pragma warning(push)
#pragma warning(disable:4996)
		long long start = m_pCursorManager->GetStart();
		long long end = m_pCursorManager->GetEnd();
		m_pCursorManager->SetNewPosition(acpStart, acpEnd);
		m_textManager.Copy(pchPlain);
		m_pCursorManager->SetNewPosition(start, end);
#pragma warning(pop)
	}

	*pcchPlainRet = acpEnd - acpStart;
	if (cRunInfoReq) {
		prgRunInfo[0].uCount = acpEnd - acpStart;
		prgRunInfo[0].type = TS_RT_PLAIN;
		*pcRunInfoRet = 1;
	}

	*pacpNext = acpEnd;
	Log::Detail(L"KsEditor::%s() %s", __FUNCTIONW__, pchPlain);
	return S_OK;
}
HRESULT KsEditor::SetText( DWORD dwFlags, LONG acpStart, LONG acpEnd, const WCHAR* pchText, ULONG cch, TS_TEXTCHANGE* pChange )
{
	Log::Detail(L"KsEditor::%s() : %s & %s", __FUNCTIONW__, pchText, std::to_wstring(cch).c_str());
	LONG acpRemovingEnd;

	if (acpStart > (LONG)m_textManager.length()) {
		return E_INVALIDARG;
	}

	acpRemovingEnd = std::min(acpEnd, (LONG)m_textManager.length());
	Log::Error(L"erase count(%d-%d): %s",acpStart, acpRemovingEnd, std::to_wstring(acpRemovingEnd - acpStart).c_str());
	m_pCursorManager->SetNewPosition(acpStart, acpRemovingEnd);
	m_textManager.Erase();
	m_pCursorManager->SetNewPosition(acpStart, acpStart + cch);
	m_textManager.Insert(pchText);
	pChange->acpStart = acpStart;
	pChange->acpOldEnd = acpEnd;
	pChange->acpNewEnd = acpStart + cch;

	//UpdateText();
	return S_OK;
}
HRESULT KsEditor::GetEndACP( LONG* pacp )
{
	Log::Detail(L"KsEditor::%s()", __FUNCTIONW__);

	if (!m_lock.IsLock(false)) {
		return TS_E_NOLOCK;
	}
	*pacp = static_cast<LONG>(m_pCursorManager->GetEnd());
	return S_OK;
}
HRESULT KsEditor::FindNextAttrTransition(LONG acpStart, LONG acpHalt, ULONG cFilterAttrs, const TS_ATTRID* paFilterAttrs, DWORD dwFlags, LONG* pacpNext, BOOL* pfFound, LONG* plFoundOffset)
{
	Log::Detail(L"KsEditor::%s()", __FUNCTIONW__);

	*pacpNext = 0;
	*pfFound = FALSE;
	*plFoundOffset = 0;
	return S_OK;
}
HRESULT KsEditor::GetScreenExt( TsViewCookie vcView, RECT* prc)
{
	Log::Detail(L"KsEditor::%s()", __FUNCTIONW__);

	GetClientRect(m_hWnd, prc);
	return S_OK;
}

HRESULT KsEditor::GetTextExt( TsViewCookie vcView, LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped )
{
	Log::Detail(L"KsEditor::%s()", __FUNCTIONW__);

	if (!m_lock.IsLock(false)) {
		return TS_E_NOLOCK;
	}

	if (acpStart == acpEnd) {
		return E_INVALIDARG;
	}

	if (acpStart > acpEnd) {
		std::swap(acpStart, acpEnd);
	}

	RECT rc;
	GetClientRect(m_hWnd, &rc);
	auto layout = m_tbuilder->CreateTextLayout(m_textManager.toString(), static_cast<FLOAT> (rc.right - rc.left), static_cast<FLOAT> (rc.right - rc.left));
	UINT32 count;
	layout->HitTestTextRange(static_cast<UINT32>(m_pCursorManager->GetStart()), static_cast<UINT32>(m_pCursorManager->GetSelectLength()), 0, 0, NULL, 0, &count);

	auto mats = std::make_unique<DWRITE_HIT_TEST_METRICS[]>(count);
	HRESULT hr = layout->HitTestTextRange(static_cast<UINT32>(m_pCursorManager->GetStart()), static_cast<UINT32>(m_pCursorManager->GetSelectLength()), 0, 0, mats.get(), count, &count);
	if (FAILED(hr)) { throw hr; }
	LONG left = LONG_MAX, top = LONG_MAX, right = 0, bottom = 0;
	for (auto i = 0UL; i < count; i++) {
		left = left < mats[i].left ? left : static_cast<LONG>(mats[i].left);
		top = top < mats[i].top ? top : static_cast<LONG>(mats[i].top);
		auto r = static_cast<LONG>(mats[i].left + mats[i].width);
		right = right > r ? right : r;
		auto b = static_cast<LONG>(mats[i].top + mats[i].height);
		bottom = bottom > b ? bottom : b;
	}
	RECT localrc{ left, top, right, bottom };
	*prc = localrc;
	MapWindowPoints(m_hWnd, 0, reinterpret_cast<POINT*>(prc), 2);
	*pfClipped = FALSE;
	return S_OK;
}
HRESULT KsEditor::InsertTextAtSelection( DWORD dwFlags, const WCHAR* pchText, ULONG cch, LONG* pacpStart, LONG* pacpEnd, TS_TEXTCHANGE* pChange )
{
	Log::Detail(L"KsEditor::%s()", __FUNCTIONW__);

	if (!m_lock.IsLock(true)) {
		return TS_E_NOLOCK;
	}
	auto r = _InsertTextAtSelection(dwFlags, pchText, cch, pacpStart, pacpEnd, pChange);
	//UpdateText();
	return r;
}

HRESULT KsEditor::_InsertTextAtSelection( DWORD dwFlags, const WCHAR* pchText, ULONG cch, LONG* pacpStart, LONG* pacpEnd, TS_TEXTCHANGE* pChange )
{
	if (dwFlags & TS_IAS_QUERYONLY) {
		*pacpStart = static_cast<LONG>(m_pCursorManager->GetStart());
		*pacpEnd = static_cast<LONG>(m_pCursorManager->GetEnd());
	} else {
		LONG acpStart = static_cast<LONG>(m_pCursorManager->GetStart());
		LONG acpEnd = static_cast<LONG>(m_pCursorManager->GetEnd());
		LONG length = acpEnd - acpStart;
		m_pCursorManager->SetNewPosition(acpStart, acpEnd);
		if (length != 0) {
			m_textManager.Erase();
			acpStart -= length;
			acpEnd -= length;
		}

		m_pCursorManager->SetNewPosition(acpStart, (acpStart + cch));
		if (!pchText) {
			m_textManager.Insert(pchText);
		}

		if (pacpStart) {
			*pacpStart = acpStart;
		}

		if (pacpEnd) {
			*pacpEnd = acpStart + cch;
		}

		if (pChange) {
			pChange->acpStart = acpStart;
			pChange->acpOldEnd = acpEnd;
			pChange->acpNewEnd = acpStart + cch;
		}

	}
	return S_OK;

}

void KsEditor::RequestAsyncLock( DWORD dwLockFlags )
{
	unsigned long expect = TS_LF_READ;
	m_request_lock_async.compare_exchange_strong(expect, dwLockFlags);
	unsigned long expect2 = 0;
	m_request_lock_async.compare_exchange_strong(expect2, dwLockFlags);
}

void KsEditor::PushAsyncCallQueue( bool write, std::function<void()> fn )
{
	std::lock_guard lock(m_queue_lock);
	if (write) {
		m_write_queue.push(fn);
	} else {
		m_read_queue.push(fn);
	}
}

void KsEditor::CallAsync()
{
	std::lock_guard lock(m_queue_lock);
	m_lock.TryWriteLockToCallApp([this] {
		while (!m_write_queue.empty()) {
			auto e = m_write_queue.front();
			e();
			m_write_queue.pop();
		}
	});
	m_lock.TryReadLockToCallApp([this] {
		while (!m_read_queue.empty()) {
			auto e = m_read_queue.front();
			e();
			m_read_queue.pop();
		}
	});
	auto flag = m_request_lock_async.exchange(0);
	if (flag != 0) {
		try {
			auto fn = [this, flag] {
				try {
					m_sink->OnLockGranted(flag);
				} catch (...) {

				}
			};
			if ((flag & TS_LF_READWRITE) == TS_LF_READWRITE) {
				Log::Detail(L"KsEditor::%s() TryWriteLockToCallTS", __FUNCTIONW__);
				if (!m_lock.TryWriteLockToCallTS(fn)) {
					Log::Detail(L"KsEditor::%s() TryWriteLockToCallTS Failed", __FUNCTIONW__);
					RequestAsyncLock(flag);
				}
			}
			else {
				Log::Detail(L"KsEditor::%s() TryReadLockToCallTS", __FUNCTIONW__);
				if (!m_lock.TryReadLockToCallTS(fn)) {
					Log::Detail(L"KsEditor::%s() TryReadLockToCallTS Failed", __FUNCTIONW__);
					RequestAsyncLock(flag);
				}
			}
		} catch (...) {

		}
	}
}

/* CompositionSink */
HRESULT KsEditor::OnStartComposition(ITfCompositionView* pComposition, BOOL* pfOk)
{
	Log::Detail(L"KsEditor::%s()", __FUNCTIONW__);
	*pfOk = TRUE;
	UpdateText();
	return S_OK;
}

HRESULT KsEditor::OnUpdateComposition(ITfCompositionView* pComposition, ITfRange* pRangeNew)
{
	Log::Detail(L"KsEditor::%s()", __FUNCTIONW__);
	UpdateText();
	return S_OK;
}

HRESULT KsEditor::OnEndComposition(ITfCompositionView* pComposition)
{
	Log::Detail(L"KsEditor::%s()", __FUNCTIONW__);
	UpdateText();
	return S_OK;
}

void* KsEditor::KmfCallback(KeyboardManagerFeatureCallback::Feature feature, void* pArg, size_t size)
{
	KeyboardManagerFeatureCallback::FeatureArg* pFeature = reinterpret_cast<KeyboardManagerFeatureCallback::FeatureArg*>(pArg);
	if (feature == KeyboardManagerFeatureCallback::FEATURE_VECTOR) {
		if (pFeature->feature.vector.key == VK_UP) {
			m_pCursorManager->Up(pFeature->feature.vector.rangeSelect, pFeature->feature.vector.control);
		}
		if (pFeature->feature.vector.key == VK_DOWN) {
			m_pCursorManager->Down(pFeature->feature.vector.rangeSelect, pFeature->feature.vector.control);
		}
		if (pFeature->feature.vector.key == VK_LEFT) {
			m_pCursorManager->Left(pFeature->feature.vector.rangeSelect, pFeature->feature.vector.control);
		} else if (pFeature->feature.vector.key == VK_RIGHT) {
			m_pCursorManager->Right(pFeature->feature.vector.rangeSelect, pFeature->feature.vector.control);
		} else if (pFeature->feature.vector.key == VK_HOME) {
			m_pCursorManager->Home(pFeature->feature.vector.rangeSelect, pFeature->feature.vector.control);
		} else if (pFeature->feature.vector.key == VK_END) {
			m_pCursorManager->End(pFeature->feature.vector.rangeSelect, pFeature->feature.vector.control);
		} else if (pFeature->feature.vector.key == VK_DELETE) {
			long long start = m_pCursorManager->GetStart();
			long long end = m_pCursorManager->GetEnd();
			if (!m_pCursorManager->IsSelecting()) {
				m_pCursorManager->SetNewPosition(start, end + 1);
			}
			m_textManager.Erase();
			m_pCursorManager->SetActiveSelEnd(TS_AE_NONE);
			m_pCursorManager->SetNewPosition(start, start);
		}
	} else if (feature == KeyboardManagerFeatureCallback::FEATURE_COPY) {
		std::wstring str = m_textManager.GetSelectedString();
		Log::Info(L"copy buffer:%s", str.c_str());
		CopyToClipboard(str);
	} else if (feature == KeyboardManagerFeatureCallback::FEATURE_PASTE) {
		OnPaste();
	} else if (feature == KeyboardManagerFeatureCallback::FEATURE_ALL_SELECT) {
		m_pCursorManager->AllSelect();
	}
	return NULL;
}

