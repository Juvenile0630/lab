// KsEditor.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "pch.h"
#include "framework.h"
#include "main.h"
#include "KsEditor.h"
#include "KsEditorManager.h"
#include "Log.h"

#pragma comment(lib, "DWrite.lib")
#pragma comment(lib, "D2d1.lib")

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	Log::SetLogLevel(Log::LOG_INFO);
	CoInitialize(NULL);

	auto r = KsEditorManager::Main(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	CoUninitialize();
	return r;

}

HWND KsEditorManager::m_hWnd;
HINSTANCE KsEditorManager::appInstance;
Microsoft::WRL::ComPtr<ITfThreadMgr> KsEditorManager::m_thread_mgr;
TfClientId KsEditorManager::m_clientId;
Microsoft::WRL::ComPtr<ID2D1Factory> KsEditorManager::m_d2d_factory;
Microsoft::WRL::ComPtr<IDWriteFactory> KsEditorManager::m_dwrite_factory;
Microsoft::WRL::ComPtr<KsEditor> KsEditorManager::m_console;
Microsoft::WRL::ComPtr<ITfCategoryMgr> KsEditorManager::m_category_mgr;
Microsoft::WRL::ComPtr<ITfDisplayAttributeMgr> KsEditorManager::m_attribute_mgr;

LRESULT CALLBACK KsEditorManager::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;
		case WM_SETFOCUS:
			if (m_console != NULL) {
				if (SetFocus(m_console->GetHWnd()) == NULL) {
					DestroyWindow(hWnd);
					break;
				}
			}
			break;
		case WM_ACTIVATE:
		case WM_SIZING:
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
		case WM_LBUTTONDOWN:
			if (m_console != NULL) {
				SendMessage(m_console->GetHWnd(), message, wParam, lParam);
			}
			break;
		case WM_SIZE:
			if (m_console != NULL) {
				RECT rect;
				GetClientRect(m_hWnd, &rect);
				SetWindowPos(m_console->GetHWnd(), 0, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOOWNERZORDER);
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int KsEditorManager::Main(HINSTANCE hInstance,
	HINSTANCE,
	LPTSTR,
	int)
{
	appInstance = hInstance;
	WNDCLASS wc;

	wc.style = 0;
	wc.lpfnWndProc = KsEditorManager::WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = appInstance;
	wc.hCursor = NULL;
	wc.hIcon = NULL;
	wc.hbrBackground = 0;
	wc.lpszClassName = className;
	wc.lpszMenuName = NULL;

	if (!RegisterClass(&wc)) {
		return -1;
	}
	m_hWnd = CreateWindowEx(
		0,
		className,
		L"Editor",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		appInstance,
		NULL
	);

	HRESULT hr;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&m_d2d_factory));
	if (FAILED(hr)) { throw hr; }

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dwrite_factory);
	if (FAILED(hr)) { throw hr; }

	hr = CoCreateInstance(CLSID_TF_ThreadMgr, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_thread_mgr));
	if (FAILED(hr)) { throw hr; }

	hr = CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_category_mgr));
	if (FAILED(hr)) { throw hr; }

	hr = CoCreateInstance(CLSID_TF_DisplayAttributeMgr, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_attribute_mgr));
	if (FAILED(hr)) { throw hr; }
	//m_attribute_mgr->AddRef();

	m_thread_mgr->Activate(&m_clientId);


	RECT rect;
	GetWindowRect(m_hWnd, (LPRECT)&rect);

	KsEditor::Create(m_hWnd, 0, 0, rect.right - rect.left, rect.bottom - rect.top, (HMENU)0x20, m_thread_mgr.Get(), m_clientId, m_category_mgr.Get(), m_attribute_mgr.Get(), m_d2d_factory.Get(), m_dwrite_factory.Get(), &m_console);
	ShowWindow(m_hWnd, TRUE);
	UpdateWindow(m_hWnd);
	auto r = Run();

	m_thread_mgr->Deactivate();
	return r;
}

int KsEditorManager::Run()
{
	Microsoft::WRL::ComPtr<ITfKeystrokeMgr> keyMgr;
	Microsoft::WRL::ComPtr<ITfMessagePump> msgPump;

	if (FAILED(m_thread_mgr.As(&keyMgr))) {
		return 1;
	}
	if (FAILED(m_thread_mgr.As(&msgPump))) {
		return 1;
	}
	for (;;) {
		MSG msg;
		BOOL fResult;
		BOOL fEaten;
		BOOL focus;
		m_thread_mgr->IsThreadFocus(&focus);
		OutputDebugStringA(focus ? "" : "");
		try {
			if (FAILED(msgPump->GetMessage(&msg, 0, 0, 0, &fResult))) {
				return -1;
			} else if (msg.message == WM_KEYDOWN) {
				if (keyMgr->TestKeyDown(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten &&
					keyMgr->KeyDown(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten) {
					continue;
				}
			} else if (msg.message == WM_KEYUP) {
				if (keyMgr->TestKeyUp(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten &&
					keyMgr->KeyUp(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten) {
					continue;
				}
			}

			if (fResult == 0) {
				return static_cast<int>(msg.wParam);
			} else if (fResult == -1) {
				return -1;
			}
		} catch (...) {
			//MS_IMEはキーを長押しし続けるとTestKeyDownでアクセス違反を引き起こすので握りつぶす。
		}
		::Sleep(1);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
//static fields