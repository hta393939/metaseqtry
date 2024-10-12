//---------------------------------------------------------------------------
#ifndef LanguageH
#define LanguageH

#include <string>
#include <map>

class MLanguage
{
private:
	std::map<std::string, std::wstring> Data;
	std::wstring mLanguage;

public:
	MLanguage();
	~MLanguage();

	bool Contains() const				{ return !Data.empty(); }
	std::wstring GetLanguage() const	{ return mLanguage; }
	bool Load(const std::wstring& language, const std::wstring& filename);
	const wchar_t *Search(const char *Key);
};



#endif
