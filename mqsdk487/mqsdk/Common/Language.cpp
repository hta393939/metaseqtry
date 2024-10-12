#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
#include "MQPlugin.h"
#include "Language.h"
#include <assert.h>


static std::wstring convertStringCode(const std::wstring& src_text)
{
	std::wstring text;
	for(size_t i=0; i<src_text.length(); i++){
		if(src_text[i] == L'\\'){
			if(i+1 < src_text.length()){
				switch(src_text[i+1]){
				case L'\\': text += L'\\'; break;
				case L'n': text += L'\n'; break;
				case L't': text += L'\t'; break;
				default: assert(0); break;
				}
				i++;
			}
		}else{
			text += src_text[i];
		}
	}
	return text;
}

static int toInt(const std::string& str)
{
	if(str.length() > 0){
		return atoi(str.c_str());
	}
	return 0;
}

//---------------------------------------------------------------------------
//   class MLanguage
//---------------------------------------------------------------------------
MLanguage::MLanguage()
{
}

MLanguage::~MLanguage()
{
}

bool MLanguage::Load(const std::wstring& language, const std::wstring& filename)
{
	mLanguage = language;
	Data.clear();


	MQXmlDocument doc = MQCXmlDocument::Create();
	if(!doc->LoadFile(filename.c_str())){
		doc->DeleteThis();
		return false;
	}

	MQXmlElement root = doc->GetRootElement();
	if(root != NULL){
		bool found = false;
		for(int iter=0; iter<2 && !found; iter++){
			MQXmlElement res_root = root->FirstChildElement("resource");
			while(res_root != NULL){
				std::wstring lang_val = res_root->GetAttribute(L"language");
				int def_val = toInt(res_root->GetAttribute("default"));
				if(language == lang_val || (iter==1 && def_val)){
					MQXmlElement elem = res_root->FirstChildElement("string");
					while(elem != NULL){
						std::string key(elem->GetAttribute("id"));
						std::wstring xmltext = elem->GetTextW();
						std::wstring text = convertStringCode(xmltext);

						std::map<std::string,std::wstring>::iterator it = Data.find(key);
						if(it == Data.end()){
							Data[key] = text;
						}else{
							// 重複データが万一あれば消してしまう
							Data.erase(it);
						}

						elem = res_root->NextChildElement("string", elem);
					}
					doc->DeleteThis();
					return true;
				}

				res_root = root->NextChildElement("resource", res_root);
			}
		}
	}

	doc->DeleteThis();
	return false;
}

const wchar_t *MLanguage::Search(const char *Key)
{
	std::map<std::string,std::wstring>::iterator it = Data.find(std::string(Key));
	if(it != Data.end()){
		return (*it).second.c_str();
	}
	return L"";
}


