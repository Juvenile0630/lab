#include "pch.h"
#include "KsEditorManager.h"


void KsEditorManager::Initialize(HWND hWnd)
{

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
	GetWindowRect(hWnd, (LPRECT)&rect);

	KsEditor::Create(hWnd, 0, 0, rect.right - rect.left, rect.bottom - rect.top, (HMENU)0x20, m_thread_mgr.Get(), m_clientId, m_category_mgr.Get(), m_attribute_mgr.Get(), m_d2d_factory.Get(), m_dwrite_factory.Get(), &m_console);
}

void KsEditorManager::Finalize()
{
	m_thread_mgr->Deactivate();
}
