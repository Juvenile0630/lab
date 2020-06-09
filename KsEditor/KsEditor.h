#pragma once

#include <Windows.h>
#include <memory>
#include <functional>
#include <numeric>
#include <atomic>
#include <shared_mutex>
#include <msctf.h>
#include <optional>
#include <queue>
#include <olectl.h>
#include <thread>
#include <msctf.h>
#include <wrl.h>
#include <chrono>
#include "TextBuilder.h"
#include "KsEditor.h"
#include "Mutex.h"
#include "Direct2D.h"
#include "TSFDWriteDrawer.h"
#include "KsEditorTextManager.h"
#include "KeyboardManager.h"
#include "KsEditorCursorManager.h"
#include "KsEditorScrollBar.h"

class KsEditor : public ITextStoreACP, ITfContextOwnerCompositionSink, KeyboardManagerFeatureCallback
{
public:
	// void OnSize(ConsoleWindow*, LPARAM);
	static HRESULT RegisterConsoleWindowClass(HINSTANCE hinst);//call once.but automatic call when create.
	static void Create(HINSTANCE, HWND, int x, int y, int w, int h, HMENU, ITfThreadMgr* threadmgr, TfClientId, ITfCategoryMgr* cate_mgr, ITfDisplayAttributeMgr* attr_mgr, ID2D1Factory*, IDWriteFactory*, KsEditor**);
	inline static void Create(HWND parent, int x, int y, int w, int h, HMENU menu, ITfThreadMgr* threadmgr, TfClientId cid, ITfCategoryMgr* cate_mgr, ITfDisplayAttributeMgr* attr_mgr, ID2D1Factory* d2d_f, IDWriteFactory* dwrite_f, KsEditor** pr) {
		return Create(reinterpret_cast<HINSTANCE>(GetWindowLongPtr(parent, GWLP_HINSTANCE)), parent, x, y, w, h, menu, threadmgr, cid, cate_mgr, attr_mgr, d2d_f, dwrite_f, pr);
	}
	const HWND GetParentHwnd()
	{
		return m_parent;
	}
	const HWND GetHWnd()
	{
		return m_hWnd;
	}

	/*
	ItextStoreAcp
	*/
	ULONG STDMETHODCALLTYPE AddRef() override
	{
		m_ref_cnt++;
		return m_ref_cnt;
	}

	ULONG STDMETHODCALLTYPE Release() override
	{
		m_ref_cnt--;
		if (m_ref_cnt <= 0) {
			delete this;
			return 0;
		}
		return m_ref_cnt;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppv) override
	{
		*ppv = NULL;
		if (riid == IID_IUnknown || riid == IID_ITextStoreACP) {
			*ppv = dynamic_cast<ITextStoreACP*>(this);
			AddRef();
			return S_OK;
		} else if (riid == IID_ITfContextOwnerCompositionSink) {
			*ppv = dynamic_cast<ITfContextOwnerCompositionSink*>(this);
			AddRef();
			return S_OK;
		} else {
			ppv = nullptr;
			return E_NOINTERFACE;
		}
	}

	HRESULT STDMETHODCALLTYPE GetWnd(TsViewCookie vcView, HWND* phwnd);
	HRESULT STDMETHODCALLTYPE AdviseSink(REFIID riid, IUnknown* io_unknown_cp, DWORD i_mask);
	HRESULT STDMETHODCALLTYPE UnadviseSink(IUnknown* punk);
	HRESULT STDMETHODCALLTYPE RequestLock(DWORD dwLockFlags, HRESULT* phrSession);
	HRESULT STDMETHODCALLTYPE GetStatus(TS_STATUS* pdcs);
	HRESULT STDMETHODCALLTYPE GetActiveView(TsViewCookie* pvcView);
	HRESULT STDMETHODCALLTYPE QueryInsert(LONG acpTestStart, LONG acpTestEnd, ULONG cch, LONG* pacpResultStart, LONG* pacpResultEnd);
	HRESULT STDMETHODCALLTYPE GetSelection(ULONG ulIndex, ULONG ulCount, TS_SELECTION_ACP* pSelection, ULONG* pcFetched);
	HRESULT STDMETHODCALLTYPE SetSelection(ULONG ulCount, const TS_SELECTION_ACP* pSelection);
	HRESULT STDMETHODCALLTYPE GetText(LONG acpStart, LONG acpEnd, WCHAR* pchPlain, ULONG cchPlainReq, ULONG* pcchPlainRet, TS_RUNINFO* prgRunInfo, ULONG cRunInfoReq, ULONG* pcRunInfoRet, LONG* pacpNext);
	HRESULT STDMETHODCALLTYPE SetText(DWORD dwFlags, LONG acpStart, LONG acpEnd, const WCHAR* pchText, ULONG cch, TS_TEXTCHANGE* pChange);
	HRESULT STDMETHODCALLTYPE GetEndACP(LONG* pacp);
	HRESULT STDMETHODCALLTYPE GetScreenExt(TsViewCookie vcView, RECT* prc);
	HRESULT STDMETHODCALLTYPE FindNextAttrTransition(LONG acpStart, LONG acpHalt, ULONG cFilterAttrs, const TS_ATTRID* paFilterAttrs, DWORD dwFlags, LONG* pacpNext, BOOL* pfFound, LONG* plFoundOffset);
	HRESULT STDMETHODCALLTYPE GetTextExt(TsViewCookie vcView, LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped);
	HRESULT STDMETHODCALLTYPE InsertTextAtSelection(DWORD dwFlags, const WCHAR* pchText, ULONG cch, LONG* pacpStart, LONG* pacpEnd, TS_TEXTCHANGE* pChange);
	HRESULT STDMETHODCALLTYPE GetFormattedText(LONG acpStart, LONG acpEnd, IDataObject** ppDataObject) { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE GetEmbedded(LONG acpPos, REFGUID rguidService, REFIID riid, IUnknown** ppunk) { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE InsertEmbedded(DWORD dwFlags, LONG acpStart, LONG acpEnd, IDataObject* pDataObject, TS_TEXTCHANGE* pChange) { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE RetrieveRequestedAttrs( ULONG ulCount, TS_ATTRVAL* paAttrVals, ULONG* pcFetched) { *pcFetched = 0; return S_OK; }
	HRESULT STDMETHODCALLTYPE RequestSupportedAttrs(DWORD dwFlags, ULONG cFilterAttrs, const TS_ATTRID* paFilterAttrs) { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE RequestAttrsAtPosition(LONG acpPos, ULONG cFilterAttrs, const TS_ATTRID* paFilterAttrs, DWORD dwFlags) { return S_OK; }
	HRESULT STDMETHODCALLTYPE RequestAttrsTransitioningAtPosition(LONG acpPos, ULONG cFilterAttrs, const TS_ATTRID* paFilterAttrs, DWORD dwFlags) { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE InsertEmbeddedAtSelection(DWORD dwFlags, IDataObject* pDataObject, LONG* pacpStart, LONG* pacpEnd, TS_TEXTCHANGE* pChange) { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE GetACPFromPoint(TsViewCookie vcView, const POINT* pt, DWORD dwFlags, LONG* pacp) { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE QueryInsertEmbedded(const GUID* pguidService, const FORMATETC* pFormatEtc, BOOL* pfInsertable) { return E_NOTIMPL; }
	/*
		ITfContextOwnerCompositionSink
	*/
	STDMETHODIMP OnStartComposition(ITfCompositionView* pComposition, BOOL* pfOk);
	STDMETHODIMP OnEndComposition(ITfCompositionView* pComposition);
	STDMETHODIMP OnUpdateComposition( ITfCompositionView* pComposition, ITfRange* pRangeNew);
	/*
		original functions
	*/
	void RequestAsyncLock(DWORD);
	void PushAsyncCallQueue(bool write, std::function<void()>);
	template <class R>
	R CallWithAppLock(bool write, std::function<R()> fn)
	{
		if (write) {
			return m_lock.WriteLockToCallApp<R>(fn);
		} else {
			return m_lock.ReadLockToCallApp<R>(fn);
		}
	}
	void CallWithAppLock(bool write, std::function<void()> fn)
	{
		if (write) {
			return m_lock.WriteLockToCallApp(fn);
		} else {
			return m_lock.ReadLockToCallApp(fn);
		}
	}
	void CopyToClipboard(std::wstring copyString);
	virtual ~KsEditor();
	void* KmfCallback(Feature feature, void* pArg, size_t size);
private:
	KsEditor();
	static constexpr UINT_PTR CallAsyncTimerId = 0x01;
	static constexpr LPCWSTR className = L"KsEditor";
	std::unique_ptr<Direct2DWithHWnd> m_d2d;
	std::unique_ptr<TextBuilder> m_tbuilder;
	Microsoft::WRL::ComPtr<TsfDWriteDrawer> m_drawer;
	Microsoft::WRL::ComPtr<ITfDocumentMgr> m_docmgr;
	Microsoft::WRL::ComPtr<ITfProperty> m_attr_prop;
	TfEditCookie m_edit_cookie;
	Microsoft::WRL::ComPtr<ITfCategoryMgr> m_category_mgr;
	Microsoft::WRL::ComPtr<ITfDisplayAttributeMgr> m_attribute_mgr;
	Microsoft::WRL::ComPtr<ITfContext> m_context;
	Microsoft::WRL::ComPtr<ITfThreadMgr> m_threadmgr;
	TfClientId m_clientId;
	HINSTANCE m_hInstance;
	HWND m_parent;
	HWND m_hWnd;
	std::atomic<DWORD> m_request_lock_async;
	std::recursive_mutex m_queue_lock;
	std::queue<std::function<void()>> m_write_queue;
	std::queue<std::function<void()>> m_read_queue;
	bool m_caret_display;
	std::chrono::steady_clock::time_point m_caret_update_time;
	Microsoft::WRL::ComPtr<ITextStoreACPSink> m_sink;
	KsEditorTextManager m_textManager;
	DWORD m_sinkmask;
	ULONG m_ref_cnt;
	Lock m_lock;
	bool m_composition;
	KeyboardManager* m_pKeyboardManager;
	KsEditorCursorManager* m_pCursorManager;
	KsEditorScrollBar* m_pScrollBar;

	void DrawString(ID2D1RenderTarget* target, std::wstring string, Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> textColor, bool drawCaret = false);
	void Init(int x, int y, int w, int h, HMENU m, ID2D1Factory* d2d_f, IDWriteFactory* dwrite_f);
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	void CallAsync();
	void OnSetFocus();
	void OnPaint();
	void OnSize();
	void OnChar(WPARAM);
	void OnTimer();
	void OnKeyDown(WPARAM);
	void UpdateText();
	void OnPaste();
	void CaretUpdate();
	void OnMouseMove(int x, int y);
	void OnLButtonUp(int x, int y);
	void OnLButtonDown(int x, int y, bool shift);
	void GetViewMatrix(OUT DWRITE_MATRIX* matrix) const;
	HRESULT _InsertTextAtSelection( DWORD dwFlags, const WCHAR* pchText, ULONG cch, LONG* pacpStart, LONG* pacpEnd, TS_TEXTCHANGE* pChange );

};


