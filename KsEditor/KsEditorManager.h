#pragma once

#include <Windows.h>
#include <wrl.h>
#include "KsEditor.h"


class KsEditorManager
{
public:
	static constexpr const TCHAR* className = _T("KsEditorManager");
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static int Main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
	static int Run();
	static void Initialize(HWND hWnd);
	static void Finalize();
private:
	static HWND m_hWnd;
	static HINSTANCE appInstance;
	static Microsoft::WRL::ComPtr<KsEditor> m_console;
	static Microsoft::WRL::ComPtr<ITfThreadMgr> m_thread_mgr;
	static Microsoft::WRL::ComPtr<ID2D1Factory> m_d2d_factory;
	static Microsoft::WRL::ComPtr<IDWriteFactory> m_dwrite_factory;
	static TfClientId m_clientId;
	static Microsoft::WRL::ComPtr<ITfCategoryMgr> m_category_mgr;
	static Microsoft::WRL::ComPtr<ITfDisplayAttributeMgr> m_attribute_mgr;
};