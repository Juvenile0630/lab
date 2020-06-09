#include "pch.h"
#include "KeyboardManager.h"
#include <windows.h>
#include "Log.h"
#include <thread>


KeyboardManager::KeyboardManager() :
	m_pCallbacks(),
	m_run(true)
{
	m_thread = std::thread([this] {Run(this); });
}

KeyboardManager::~KeyboardManager()
{
	m_run = false;
	m_thread.join();
}

void* KeyboardManager::Run(void* pArg)
{
	KeyboardManager* pThis = reinterpret_cast<KeyboardManager*>(pArg);
	WORD keyboard[256] = {0};
	while (m_run) {
		::Sleep(1);
		if (m_active == false) {
			continue;
		}
		pThis->isSystemKey(keyboard);
		KeyboardManagerFeatureCallback::FeatureArg arg;
		KeyboardManagerFeatureCallback::Feature feature = pThis->getFeature(keyboard, &arg);
		for (auto itr = pThis->m_pCallbacks.begin(); itr != pThis->m_pCallbacks.end(); ++itr) {
			KeyboardManagerFeatureCallback* pCallback = (*itr);
			if (pCallback != NULL) {
				pCallback->KmfCallback(feature, &arg, sizeof(arg));
			}
		}
	}
	return NULL;
}

void KeyboardManager::SetActive(bool active)
{
	Log::Info(L"KeyboardManager::%s(%d)", __FUNCTIONW__, active);
	m_active = active;
}

bool KeyboardManager::isSystemKey(unsigned short* pKeyboardMap)
{
	bool ret = false;

	if (setKeyBoardInfo(VK_CONTROL, pKeyboardMap)) {
		Log::Debug(L"[Ctrl] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_SPACE, pKeyboardMap)) {
		Log::Debug(L"[Space] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_RETURN, pKeyboardMap)) {
		Log::Debug(L"[Return] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_SHIFT, pKeyboardMap)) {
		Log::Debug(L"[Shift] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_LBUTTON, pKeyboardMap)) {
		Log::Debug(L"[Mouse L Button] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_RBUTTON, pKeyboardMap)) {
		Log::Debug(L"[Mouse R Button] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_CANCEL, pKeyboardMap)) {
		Log::Debug(L"[Calncel] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_MBUTTON, pKeyboardMap)) {
		Log::Debug(L"[Mouse Middle Button] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_BACK, pKeyboardMap)) {
		Log::Debug(L"[Back Space] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_TAB, pKeyboardMap)) {
		Log::Debug(L"[Tab] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_CLEAR, pKeyboardMap)) {
		Log::Debug(L"[Clear] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_MENU, pKeyboardMap)) {
		Log::Debug(L"[Alt] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_PAUSE, pKeyboardMap)) {
		Log::Debug(L"[Pause] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_CAPITAL, pKeyboardMap)) {
		Log::Debug(L"[Caps Lock] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_KANA, pKeyboardMap)) {
		Log::Debug(L"[IMEかな] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_JUNJA, pKeyboardMap)) {
		Log::Debug(L"[IME Junja] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_FINAL, pKeyboardMap)) {
		Log::Debug(L"[IMEファイナル] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_KANJI, pKeyboardMap)) {
		Log::Debug(L"[IME漢字] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_ESCAPE, pKeyboardMap)) {
		Log::Debug(L"[ESC] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_CONVERT, pKeyboardMap)) {
		Log::Debug(L"[IME 変換] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_NONCONVERT, pKeyboardMap)) {
		Log::Debug(L"[IME 無変換] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_ACCEPT, pKeyboardMap)) {
		Log::Debug(L"[IME 使用可能] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_PRIOR, pKeyboardMap)) {
		Log::Debug(L"[PageUp] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_NEXT, pKeyboardMap)) {
		Log::Debug(L"[PageDown] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_END, pKeyboardMap)) {
		Log::Debug(L"[End] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_HOME, pKeyboardMap)) {
		Log::Debug(L"[Home] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_LEFT, pKeyboardMap)) {
		Log::Debug(L"[←] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_UP, pKeyboardMap)) {
		Log::Debug(L"[↑] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_RIGHT, pKeyboardMap)) {
		Log::Debug(L"[→] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_DOWN, pKeyboardMap)) {
		Log::Debug(L"[↓] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_SELECT, pKeyboardMap)) {
		Log::Debug(L"[Select] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_PRINT, pKeyboardMap)) {
		Log::Debug(L"[Print] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_EXECUTE, pKeyboardMap)) {
		Log::Debug(L"[Execute] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_SNAPSHOT, pKeyboardMap)) {
		Log::Debug(L"[Print Screen] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_INSERT, pKeyboardMap)) {
		Log::Debug(L"[Insert] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_DELETE, pKeyboardMap)) {
		Log::Debug(L"[Delete] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_HELP, pKeyboardMap)) {
		Log::Debug(L"[Help] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_LWIN, pKeyboardMap)) {
		Log::Debug(L"[Left Windows] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_RWIN, pKeyboardMap)) {
		Log::Debug(L"[Right Windows] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_APPS, pKeyboardMap)) {
		Log::Debug(L"[Application] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_SLEEP, pKeyboardMap)) {
		Log::Debug(L"[Sleep] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_F1, pKeyboardMap)) {
		Log::Debug(L"[F1] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_F2, pKeyboardMap)) {
		Log::Debug(L"[F2] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_F3, pKeyboardMap)) {
		Log::Debug(L"[F3] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_F4, pKeyboardMap)) {
		Log::Debug(L"[F4] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_F5, pKeyboardMap)) {
		Log::Debug(L"[F5] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_F6, pKeyboardMap)) {
		Log::Debug(L"[F6] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_F7, pKeyboardMap)) {
		Log::Debug(L"[F7] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_F8, pKeyboardMap)) {
		Log::Debug(L"[F8] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_F9, pKeyboardMap)) {
		Log::Debug(L"[F9] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_F10, pKeyboardMap)) {
		Log::Debug(L"[F10] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_F11, pKeyboardMap)) {
		Log::Debug(L"[F11] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_F12, pKeyboardMap)) {
		Log::Debug(L"[F12] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_NUMLOCK, pKeyboardMap)) {
		Log::Debug(L"[Num Lock] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_SCROLL, pKeyboardMap)) {
		Log::Debug(L"[Scroll Lock] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_A, pKeyboardMap)) {
		Log::Debug(L"[A] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_C, pKeyboardMap)) {
		Log::Debug(L"[C] Key Down"); ret = true;
	}
	if (setKeyBoardInfo(VK_V, pKeyboardMap)) {
		Log::Debug(L"[C] Key Down"); ret = true;
	}
	return ret;
}

bool KeyboardManager::setKeyBoardInfo(int virtualKey, unsigned short* pKeyboardMap)
{
	bool ret = false;
	LPWORD pKey = &pKeyboardMap[virtualKey];
	if (GetAsyncKeyState(virtualKey) & 0x8000) {
		if ((*pKey & 0x8000) == 0) {
			ret = true;
			// 押されたとき
			*pKey |= 0x8000;
			Log::Debug(L"Pressed:%X", *pKey);
		} else if ((*pKey & 0x4000) == 0 && (*pKey & 0x3FFF) < 0x100) {
			// 押され続けているとき
			ret = false;
			*pKey += 1;
			Log::Debug(L"Pressing1:%X", *pKey);
		} else if ((*pKey & 0x4000) == 0 && (*pKey & 0x3FFF) >= 0x100) {
			// リピート
			ret = false;
			*pKey = 0x4001;
			Log::Debug(L"Repeat1:%X", *pKey);
		} else if ((*pKey & 0x4000) != 0 && (*pKey & 0x3FFF) < 0x10) {
			ret = false;
			*pKey += 1;
			Log::Debug(L"Pressing2:%X", *pKey);
		} else if ((*pKey & 0x4000) != 0 && (*pKey & 0x3FFF) >= 0x10) {
			ret = false;
			*pKey = 0x4001;
			Log::Debug(L"Repeat2:%X", *pKey);
		} else {
			//			*pKey = 0x00;
			//			ret = true;
		}
	} else {
		// 離されたとき
		*pKey = 0x00;
	}
	return ret;
}

bool KeyboardManager::isKeyPressed(int virtualKey, unsigned short* pKeyboardMap)
{
	LPWORD pKey = &pKeyboardMap[virtualKey];
	if (*pKey == 0x8000) {
		return true;
	}
	return false;
}

bool KeyboardManager::isKeyPressing(int virtualKey, unsigned short* pKeyboardMap)
{
	LPWORD pKey = &pKeyboardMap[virtualKey];
	if ((*pKey & 0x3FFF) > 0) {
		return true;
	}
	return false;
}

bool KeyboardManager::isKeyRepeat(int virtualKey, unsigned short* pKeyboardMap)
{
	LPWORD pKey = &pKeyboardMap[virtualKey];
	if (*pKey  == 0x4001) {
		return true;
	}
	return false;
}

KeyboardManagerFeatureCallback::Feature KeyboardManager::getFeature(unsigned short* pKeyboardMap, KeyboardManagerFeatureCallback::FeatureArg* pArg)
{
	KeyboardManagerFeatureCallback::Feature ret = KeyboardManagerFeatureCallback::FEATURE_NONE;

	if (isKeyPressed(VK_LEFT, pKeyboardMap) || isKeyRepeat(VK_LEFT, pKeyboardMap)) {
		Log::Info(L"[←] Key Down");
		ret = KeyboardManagerFeatureCallback::FEATURE_VECTOR;
		pArg->feature.vector.key = VK_LEFT;
	} else if (isKeyPressed(VK_UP, pKeyboardMap) || isKeyRepeat(VK_UP, pKeyboardMap)) {
		Log::Info(L"[↑] Key Down");
		ret = KeyboardManagerFeatureCallback::FEATURE_VECTOR;
		pArg->feature.vector.key = VK_UP;
	} else if (isKeyPressed(VK_RIGHT, pKeyboardMap) || isKeyRepeat(VK_RIGHT, pKeyboardMap)) {
		Log::Info(L"[→] Key Down");
		ret = KeyboardManagerFeatureCallback::FEATURE_VECTOR;
		pArg->feature.vector.key = VK_RIGHT;
	} else if (isKeyPressed(VK_DOWN, pKeyboardMap) || isKeyRepeat(VK_DOWN, pKeyboardMap)) {
		Log::Info(L"[↓] Key Down");
		ret = KeyboardManagerFeatureCallback::FEATURE_VECTOR;
		pArg->feature.vector.key = VK_DOWN;
	} else if (isKeyPressed(VK_HOME, pKeyboardMap) || isKeyRepeat(VK_HOME, pKeyboardMap)) {
		Log::Info(L"[Home] Key Down");
		ret = KeyboardManagerFeatureCallback::FEATURE_VECTOR;
		pArg->feature.vector.key = VK_HOME;
	} else if (isKeyPressed(VK_END, pKeyboardMap) || isKeyRepeat(VK_END, pKeyboardMap)) {
		Log::Info(L"[End] Key Down");
		ret = KeyboardManagerFeatureCallback::FEATURE_VECTOR;
		pArg->feature.vector.key = VK_END;
	} else if (isKeyPressed(VK_DELETE, pKeyboardMap) || isKeyRepeat(VK_DELETE, pKeyboardMap)) {
		Log::Info(L"[Delete] Key Down");
		ret = KeyboardManagerFeatureCallback::FEATURE_VECTOR;
		pArg->feature.vector.key = VK_DELETE;
	} else if (isKeyPressed(VK_CONTROL, pKeyboardMap) || isKeyPressing(VK_CONTROL, pKeyboardMap)) {
		if (isKeyPressed(VK_A, pKeyboardMap) || isKeyRepeat(VK_A, pKeyboardMap)) {
			Log::Info(L"[Ctrl+A] Key Down");
			ret = KeyboardManagerFeatureCallback::FEATURE_ALL_SELECT;
		}
	}
	if (ret == KeyboardManagerFeatureCallback::FEATURE_VECTOR) {
		if (isKeyPressed(VK_SHIFT, pKeyboardMap) || isKeyPressing(VK_SHIFT, pKeyboardMap)) {
			pArg->feature.vector.rangeSelect = true;
		} else {
			pArg->feature.vector.rangeSelect = false;
		}
		if (isKeyPressed(VK_CONTROL, pKeyboardMap) || isKeyPressing(VK_CONTROL, pKeyboardMap)) {
			pArg->feature.vector.control = true;
		} else {
			pArg->feature.vector.control = false;
		}
	} else {
		if (isKeyPressed(VK_CONTROL, pKeyboardMap) || isKeyPressing(VK_CONTROL, pKeyboardMap)) {
			if (isKeyPressed(VK_C, pKeyboardMap) || isKeyRepeat(VK_C, pKeyboardMap)) {
				ret = KeyboardManagerFeatureCallback::FEATURE_COPY;
			} else if (isKeyPressed(VK_V, pKeyboardMap) || isKeyRepeat(VK_V, pKeyboardMap)) {
				ret = KeyboardManagerFeatureCallback::FEATURE_PASTE;
			}
		}
	}

	return ret;
}

