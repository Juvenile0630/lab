#pragma once

class KsEditorTextUtility
{
public:
	KsEditorTextUtility() {};
	virtual ~KsEditorTextUtility() {};
	static bool IsHalf(wchar_t character)
	{
		return (
			(character >= 0x0 && character < 0x81) ||
			(character == 0xf8f0) ||
			(character >= 0xff61 && character < 0xffa0) ||
			(character >= 0xf8f1 && character < 0xf8f4)
		);
	}
};
