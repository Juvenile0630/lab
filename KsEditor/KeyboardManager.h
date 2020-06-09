#pragma once
#include <thread>
#include <vector>
#include "KeyboardMap.h"

class KeyboardManagerFeatureCallback
{
public:
	KeyboardManagerFeatureCallback() {};
	virtual ~KeyboardManagerFeatureCallback() {};
	enum Feature {
		FEATURE_NONE,
		FEATURE_VECTOR,
		FEATURE_SYSTEM_KEY,
		FEATURE_ALL_SELECT,
		FEATURE_COPY,
		FEATURE_PASTE,
	};
	struct FeatureArg {
		Feature m_type;
		union {
			struct None {
			} none;
			struct ChangeVector {
				int key;
				bool rangeSelect;
				bool control;
			} vector;
		} feature;
	};
	virtual void* KmfCallback( Feature feature, void* pArg, size_t size ) = 0;
};



class KeyboardManager
{
public:
	KeyboardManager();
	virtual ~KeyboardManager();
	void AddCallback(KeyboardManagerFeatureCallback* pCallback) { m_pCallbacks.push_back(pCallback); }
	void* Run(void* pArg);
	void SetActive(bool active = true);
private:
	bool m_active;
	bool m_run;
	std::thread m_thread;
	std::vector<KeyboardManagerFeatureCallback*> m_pCallbacks;
	bool isSystemKey(unsigned short* pKeyboardMap);
	bool isKeyPressed(int virtualKey, unsigned short* pKeyboardMap);
	bool setKeyBoardInfo(int virtualKey, unsigned short* pKeyboardMap);
	bool isKeyPressing(int virtualKey, unsigned short* pKeyboardMap);
	bool isKeyRepeat(int virtualKey, unsigned short* pKeyboardMap);
	KeyboardManagerFeatureCallback::Feature getFeature(unsigned short* pKeyboardMap, KeyboardManagerFeatureCallback::FeatureArg* pArg);
};

