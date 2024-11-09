//---------------------------------------------------------------------------
// このコードは ExportPMD.cpp をベースに実装されています。
//    　作成したDLLは"Plugins\Export"フォルダに入れる必要がある。
// https://github.com/gameplay3d/gameplay/blob/master/gameplay/src/Bundle.cpp
//---------------------------------------------------------------------------

#define MY_PRODUCT (0xB2B2501D)
#define MY_ID (0x0ED64D3E)
#define MY_PLUGINNAME "Export GPB Copyright(C) 2024, hta393939"
#define MY_FILETYPE "HSP GPB simple(*.gpb)"
#define MY_EXT "gpb"

#define IDENVER "0.7.1"

// 0 だと無効化
#define USESCALING (0)

// $(ProjectDir)$(Platform)\$(Configuration)\
// $(OutDir)$(TargetName)$(TargetExt)

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#if __APPLE__
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFByteOrder.h>
#define _MAX_PATH PATH_MAX
#endif
#include "MQPlugin.h"
#include "MQWidget.h"
#include "MQ3DLib.h"
#include "MQExportObject.h"
#include "MQBasePlugin.h"
#include "MQSetting.h"
#include "MQBoneManager.h"
#include "MQMorphManager.h"
#include "Language.h"
//#include "Edition.h"
#include <vector>
#include <set>
#include <map>
#include <list>
#include <algorithm>
#include <assert.h>
#include "MFileUtil.h"
#include "datastruct.h"
#include <iostream>
#include <sstream>

#define GL_TRIANGLE (0x0004)
#define GL_LINES (0x0001)
#define GL_UNSIGNED_BYTE (0x1401)
#define GL_UNSIGNED_SHORT (0x1403)
#define GL_UNSIGNED_INT (0x1405)

enum {
	STRUCT_SIMPLE = 0,
	STRUCT_OBJECT = 1,
	STRUCT_SKIN = 2,
};

enum {
	FILEOUT_NO = 0,
	FILEOUT_FORCE = 1,
	FILEOUT_CONFIRM = 2,
	FILEOUT_OWFORBIDDEN = 3,
};

enum {
	FILEIN_NOTUSE = 0,
	FILEIN_USE = 1,
};


#ifdef _WIN32
HINSTANCE s_hInstance;
wchar_t s_DllPath[MAX_PATH];
#endif

#define EPS	0.00001

#define LARGEABS (800000000.0f)

#define FMES fprintf_s

struct INDEXWEIGHT {
	int sortedIndex;
	float weight;
	INDEXWEIGHT() {
		sortedIndex = 0;
		weight = 0.0f;
	}
};


static bool	MQPointFuzzyEqual(MQPoint A, MQPoint B)
{
	return ((fabs(A.x - B.x) < EPS) && (fabs(A.y - B.y) < EPS) && (fabs(A.z - B.z) < EPS));
}

static MAnsiString getMultiBytesSubstring(const MAnsiString& str, size_t maxlen)
{
	size_t p = 0;
	while(1){
		size_t np = str.next(p);
		if(np > maxlen || np == MAnsiString::kInvalid){
			if(p > 0){
				return str.substring(0, p);
			}
			break;
		}
		p = np;
	}
	return MAnsiString();
}

#ifdef _WIN32
BOOL APIENTRY DllMain( HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	// The instance will be used for a dialog box
	s_hInstance = (HINSTANCE)hModule;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		::GetModuleFileName((HMODULE)hModule, s_DllPath, MAX_PATH);
		break;
	}
	return TRUE;
}
#endif
#if __APPLE__
static MString GetResourceDir()
{
	// Get a resource directory.
	CFBundleRef bundleRef = CFBundleGetBundleWithIdentifier(CFSTR("hta393939.ExportGPB"));
	if(bundleRef == NULL) return MString();
	
	CFURLRef resourceUrl = CFBundleCopyResourcesDirectoryURL(bundleRef);
	char path[PATH_MAX+100];
	Boolean ret = CFURLGetFileSystemRepresentation(resourceUrl, TRUE, (UInt8*)path, PATH_MAX+100);
	CFRelease(resourceUrl);
	if(!ret) return MString();
	
	return MString::fromUtf8String(path);
}
#endif


// @see Node.h#L58
enum GPBNodeType {
	GPBNODE_NODE = 1,
	GPBNODE_JOINT = 2,
};

// @see Transform.h#L89
enum AnimationAttr {
	ANIMATE_ROTATE_TRANSLATE = 16,
	ANIMATE_SCALE_ROTATE_TRANSLATE = 17,
};

struct GPBBounding {
	float min[3];
	float max[3];
	float center[3];
	float radius;
	GPBBounding() {
		min[0] = LARGEABS;
		min[1] = LARGEABS;
		min[2] = LARGEABS;
		max[0] = -LARGEABS;
		max[1] = -LARGEABS;
		max[2] = -LARGEABS;
		center[0] = 0.0f;
		center[1] = 0.0f;
		center[2] = 0.0f;
		radius = 0.0f;
	}
};

struct GPBMaterial {
	// 有効かどうか
	bool enable;

	// 材質の元のインデックス。
	int orgIndex;
	// 面頂点
	std::vector<int> faceIndices;

	// 元々の名前
	MString orgName;
	// 変換後の名前
	MString convName;

	bool useLighting;
	bool useTexture;
	// .material に書く
	MString orgDiffuseTexture;
	MString convDiffuseTexture;

	// 0: repeat, 1: mirror, 2: clamp
	int wrapU;
	int wrapV;
	// 0: nearest, 1: linear
	int filter;

	BOOL isDouble;

	// RGBA
	float diffuse[4];
	float specular[3];
	float spc_pow;

	GPBMaterial() {
		enable = true;
		orgIndex = -1;
		isDouble = FALSE;
		useLighting = true;
		useTexture = false;
		wrapU = MQMATERIAL_WRAP_CLAMP;
		wrapV = MQMATERIAL_WRAP_CLAMP;
		filter = MQMATERIAL_FILTER_LINEAR;

		diffuse[0] = 1.0f;
		diffuse[1] = 1.0f;
		diffuse[2] = 1.0f;
		diffuse[3] = 1.0f;
		specular[0] = 0.0f;
		specular[1] = 0.0f;
		specular[2] = 0.0f;
		spc_pow = 5.0f;
	}
};


/// <summary>
/// 参照テーブル構造体
/// </summary>
struct GPBRef {
	/// <summary>
	/// チャンク名
	/// </summary>
	MString name;
	DWORD type;
	/// <summary>
	/// データの開始位置
	/// </summary>
	DWORD offset;
	/// <summary>
	/// テーブルの書き出し位置
	/// </summary>
	int writeOffset;
	GPBRef() {
		name = L"";
		type = 0;
		offset = 0;
		writeOffset = 0;
	}
};

struct GPBBoneParam {
	// ボーンID. 0 は無効のような扱い
	UINT id;
	MQMatrix mtx;
	MQMatrix base_mtx;
	// 親ボーンID. 0 だと親は無い
	UINT parent;
	/// <summary>
	/// 子ボーンの個数. bone_manager から取得する
	/// </summary>
	int child_num;
	/// 位置
	MQPoint org_pos;
	/// 位置
	MQPoint def_pos;
	// スケール
	MQPoint scale;

	MString name;
	// ダミーならtrue
	bool dummy;
	/// bone_num に対する index. not bone ID
	std::vector<UINT> children;
	/// <summary>
	/// 親ボーン関係ソート後のボーン配列でのインデックス
	/// </summary>
	int sortedIndex;

	MString name_jp;
	MString name_en;

	/// <summary>
	/// refTable における対応インデックス
	/// </summary>
	int refIndex;

	/// 常に GPBNODE_JOINT
	BYTE nodeType;

	GPBBoneParam() {
		id = 0;
		parent = 0;
		child_num = 0;
		dummy = false;
		sortedIndex = -1;

		scale = MQPoint(1.0f, 1.0f, 1.0f);

		refIndex = -1;
		nodeType = GPBNODE_JOINT;
	}
};



class ExportGPBPlugin : public MQExportPlugin
{
public:
	ExportGPBPlugin();
	~ExportGPBPlugin();

	// Get a plug-in ID
	// プラグインIDを返す。
	void GetPlugInID(DWORD *Product, DWORD *ID) override;

	// Get a plug-in name
	// プラグイン名を返す。
	const char *GetPlugInName(void) override;

	// Get a file type for importing
	// 入力出力可能なファイルタイプを返す。
	const char *EnumFileType(int index) override;

	// Get a file extension for importing
	// 入力可能な拡張子を返す。
	const char *EnumFileExt(int index) override;

	// Export a file
	// ファイルの読み込み
	BOOL ExportFile(int index, const wchar_t *filename, MQDocument doc) override;


private:
	struct BoneNameSetting {
		MString jp;
		MString en;
	};

	BoneNameSetting m_RootBoneName;
	std::vector<BoneNameSetting> m_BoneNameSetting;
	bool LoadBoneSettingFile();

	int makeMaterial(FILE* fhMaterial,
		const std::vector<GPBMaterial>& materials,
		const MString& option,
		int jointNum);

	/// <summary>
	/// プレビューコードを書き出す。
	/// NOTE: アニメ有りの場合、gpact にするかどうか??
	/// </summary>
	/// <param name="f"></param>
	/// <param name="bounding"></param>
	/// <param name="name"></param>
	/// <param name="boneNames"></param>
	/// <returns></returns>
	int makeHSP(FILE* f,
		const GPBBounding& bounding,
		const MString& name,
		const std::vector<MString>& boneNames);

	/// <summary>
	/// ノード一つ分
	/// </summary>
	/// <param name="fh">ファイルハンドル</param>
	/// <param name="index">ボーンインデックス</param>
	/// <returns></returns>
	int writeJoint(FILE* fh,
		std::vector<GPBBoneParam>& bone_param,
		std::vector<GPBRef>& refTable,
		int index,
		std::map<UINT,int>& bone_id_index,
		float scaling);

	/// 
	int writeAnimations(FILE* fh,
		const ANIMATIONS& animations);

	/// <summary>
	/// .xml から読み取る
	/// </summary>
	/// <param name="animationFile"></param>
	/// <param name="animations"></param>
	/// <returns></returns>
	int loadAnimation(MString& animationFile,
		ANIMATIONS& animations);
};


ExportGPBPlugin::ExportGPBPlugin()
{
}

ExportGPBPlugin::~ExportGPBPlugin()
{
}


void ExportGPBPlugin::GetPlugInID(DWORD *Product, DWORD *ID)
{
	*Product = MY_PRODUCT;
	*ID      = MY_ID;
}

const char *ExportGPBPlugin::GetPlugInName(void)
{
	return MY_PLUGINNAME;
}

const char *ExportGPBPlugin::EnumFileType(int index)
{
	if(index == 0){
		return MY_FILETYPE;
	}
	return NULL;
}

const char *ExportGPBPlugin::EnumFileExt(int index)
{
	if(index == 0){
		return MY_EXT;
	}
	return NULL;
}

class GPBOptionDialog : public MQDialog
{
public:
	MQCheckBox *check_visible;
	MQComboBox *combo_bone;
	MQEdit *edit_textureprefix;
	//MQMemo *memo_comment; // 広いのが Memo か...

	MQComboBox* combo_mtlfile;
	MQComboBox* combo_hspfile;
	MQComboBox* combo_xmlanimfile;

	int canceled = 0;

	GPBOptionDialog(int id, int parent_frame_id, ExportGPBPlugin *plugin, MLanguage& language);

	// 
	BOOL ComboBoneChanged(MQWidgetBase *sender, MQDocument doc);

	BOOL OnCloseQuery(MQWidgetBase* sender, MQDocument doc, MQWidgetCloseQueryParam& p);
};

/// <summary>
/// ダイアログ コンストラクタ
/// </summary>
/// <param name="id"></param>
/// <param name="parent_frame_id"></param>
/// <param name="plugin"></param>
/// <param name="language"></param>
GPBOptionDialog::GPBOptionDialog(int id, int parent_frame_id, ExportGPBPlugin *plugin, MLanguage& language) : MQDialog(id)
{
	MQFrame parent(parent_frame_id);

	MQGroupBox *group = CreateGroupBox(&parent, language.Search("Option"));

	MQFrame *hframe;

	check_visible = CreateCheckBox(group, language.Search("VisibleOnly"));

	hframe = CreateHorizontalFrame(group);
	CreateLabel(hframe, language.Search("MtlFile"));
	this->combo_mtlfile = CreateComboBox(hframe);
	combo_mtlfile->AddItem(language.Search("FileOutNo"));
	combo_mtlfile->AddItem(language.Search("FileOutForce"));
	combo_mtlfile->AddItem(language.Search("FileOutConfirm"));
	combo_mtlfile->AddItem(language.Search("FileOutOWForbidden"));
	combo_mtlfile->SetHintSizeRateX(8);
	combo_mtlfile->SetFillBeforeRate(1);

	hframe = CreateHorizontalFrame(group);
	CreateLabel(hframe, language.Search("TexturePrefix"));
	this->edit_textureprefix = CreateEdit(hframe);
	//edit_textureprefix->SetMaxAnsiLength(20);
	edit_textureprefix->SetHorzLayout(LAYOUT_FILL);

	hframe = CreateHorizontalFrame(group);
	CreateLabel(hframe, language.Search("HspFile"));
	this->combo_hspfile = CreateComboBox(hframe);
	combo_hspfile->AddItem(language.Search("FileOutNo"));
	combo_hspfile->AddItem(language.Search("FileOutForce"));
	combo_hspfile->AddItem(language.Search("FileOutConfirm"));
	combo_hspfile->AddItem(language.Search("FileOutOWForbidden"));
	combo_hspfile->SetHintSizeRateX(8);
	combo_hspfile->SetFillBeforeRate(1);

	hframe = CreateHorizontalFrame(group);
	CreateLabel(hframe, language.Search("Bone"));

	{
		this->combo_bone = CreateComboBox(hframe);
		combo_bone->AddItem(language.Search("Disable"));
		combo_bone->AddItem(language.Search("Enable"));
		combo_bone->SetHintSizeRateX(8);
		combo_bone->SetFillBeforeRate(1);

		combo_bone->AddChangedEvent(this, &GPBOptionDialog::ComboBoneChanged);
	}

	{
		hframe = CreateHorizontalFrame(group);
		CreateLabel(hframe, language.Search("XmlAnimationFile"));
		this->combo_xmlanimfile = CreateComboBox(hframe);
		combo_xmlanimfile->AddItem(language.Search("NotUse"));
		combo_xmlanimfile->AddItem(language.Search("Use"));
		combo_xmlanimfile->SetHintSizeRateX(8);
		combo_xmlanimfile->SetFillBeforeRate(1);
	}

#if 0
	CreateLabel(group, language.Search("Comment"));
	memo_comment = CreateMemo(group);
	memo_comment->SetMaxLength(256);
#endif
}

BOOL GPBOptionDialog::ComboBoneChanged(MQWidgetBase *sender, MQDocument doc)
{
	this->combo_xmlanimfile->SetCurrentIndex(0);
	this->combo_xmlanimfile->SetEnabled(this->combo_bone->GetCurrentIndex() != 0);
	return FALSE;
}

BOOL GPBOptionDialog::OnCloseQuery(MQWidgetBase* sender, MQDocument doc, MQWidgetCloseQueryParam& p)
{
	if (this->combo_xmlanimfile->GetCurrentIndex() != 0) {
		p.Canceled = true;
	}
	return FALSE;
}


struct CreateDialogOptionParam
{
	ExportGPBPlugin *plugin;
	GPBOptionDialog *dialog;
	MLanguage *lang;

	bool visible_only;
	/// <summary>
	/// ボーンが1つ以上かどうか
	/// </summary>
	bool bone_exists;

	int struct_mode;
	/// <summary>
	/// UI上の 1: 有効, 0: 無効
	/// </summary>
	int output_bone;

	int mtlfile;
	std::wstring texture_prefix;
	int hspfile;

	/// <summary>
	/// UI上の 1: 使う, 0: 使わない
	/// </summary>
	int input_xmlanim;

	int additive_info = 0;
};

/// <summary>
/// オプションダイアログを生成する
/// </summary>
/// <param name="init">true だとoption値で初期化する。false だと option に値を書き出す</param>
/// <param name="param"></param>
/// <param name="ptr"></param>
static void CreateDialogOption(bool init, MQFileDialogCallbackParam *param, void *ptr)
{
	CreateDialogOptionParam *option = (CreateDialogOptionParam*)ptr;

	if (init)
	{
		GPBOptionDialog *dialog = new GPBOptionDialog(param->dialog_id, param->parent_frame_id, option->plugin, *option->lang);
		option->dialog = dialog;

		//dialog->SetCloseButton(false); // 左上のアイコンが一緒に消える
		dialog->AddCloseQueryEvent(dialog, &GPBOptionDialog::OnCloseQuery);

		dialog->check_visible->SetChecked(option->visible_only);

		dialog->combo_mtlfile->SetEnabled(true);
		dialog->combo_mtlfile->SetCurrentIndex(option->mtlfile);

		dialog->edit_textureprefix->SetEnabled(true);
		dialog->edit_textureprefix->SetText(option->texture_prefix);

		dialog->combo_hspfile->SetEnabled(true);
		dialog->combo_hspfile->SetCurrentIndex(option->hspfile);

		dialog->combo_bone->SetEnabled(option->bone_exists);
		dialog->combo_bone->SetCurrentIndex(
			(option->output_bone != 0 && option->bone_exists != 0) ? 1 : 0
		);

		dialog->combo_xmlanimfile->SetEnabled(true); // NOTE: 
		dialog->combo_xmlanimfile->SetCurrentIndex(option->input_xmlanim);
	}
	else
	{
		option->visible_only = option->dialog->check_visible->GetChecked();

		option->mtlfile = option->dialog->combo_mtlfile->GetCurrentIndex();
		option->hspfile = option->dialog->combo_hspfile->GetCurrentIndex();
#if 0
		//option->modelname = getMultiBytesSubstring(MString(option->dialog->edit_modelname->GetText()).toAnsiString(), 20);
#endif
		option->texture_prefix = option->dialog->edit_textureprefix->GetText();
		option->output_bone = option->dialog->combo_bone->GetCurrentIndex();
		option->input_xmlanim = option->dialog->combo_xmlanimfile->GetCurrentIndex();

		// NOTE: ここでどのボタンで閉じたか取得できないものか
		option->additive_info = option->dialog->canceled;

		delete option->dialog;
	}
}




#pragma pack(push,1)

/// <summary>
/// 一つのキーの時刻と値
/// </summary>
struct GPBKey {
	int msec;
	float q[4];
	float p[3];
	GPBKey() {
		msec = 0;
		q[0] = 0.0f;
		q[1] = 0.0f;
		q[2] = 0.0f;
		q[3] = 1.0f;
		p[0] = 0.0f;
		p[1] = 0.0f;
		p[2] = 0.0f;
	}
};

struct GPBScene {
	MString cameraName;
	float ambient[3];
	GPBScene() {
		cameraName = MString(L"");
		ambient[0] = 0.17205810546875f;
		ambient[1] = 0.17205810546875f;
		ambient[2] = 0.17205810546875f;
	}
};

#pragma pack(pop)


enum CodeType
{
	CODE_UTF8 = 0,
	CODE_CONVASCII,
	CODE_SJIS,
};

enum RefType
{
	REF_SCENE = 1,
	REF_NODE = 2,
	REF_ANIMATIONS = 3,
	REF_MESH = 34, // 0x22
};

/// <summary>
/// from VertexFormat.h
/// </summary>
enum AttrType
{
	ATTR_POSITION = 1,
	ATTR_NORMAL = 2,
	ATTR_COLOR = 3,
	ATTR_TANGENT = 4,
	ATTR_BINORMAL = 5,
	ATTR_BLENDWEIGHTS = 6,
	ATTR_BLENDINDICES = 7,
	ATTR_TEXCOORD0 = 8,
	ATTR_TEXCOORD1 = 9,
	ATTR_TEXCOORD2 = 10,
	ATTR_TEXCOORD3 = 11,
	ATTR_TEXCOORD4 = 12,
	ATTR_TEXCOORD5 = 13,
	ATTR_TEXCOORD6 = 14,
	ATTR_TEXCOORD7 = 15,
};

/// <summary>
/// 最大と最小から中心と半径を計算する
/// </summary>
/// <param name="bounding"></param>
static void calcRadius(GPBBounding& bounding) {
	float half[3];
	for (int j = 0; j < 3; ++j) {
		bounding.center[j] = (bounding.min[j] + bounding.max[j]) * 0.5;
		half[j] = bounding.max[j] - bounding.center[j];
	}
	bounding.radius = sqrtf(half[0] * half[0] + half[1] * half[1] + half[2] * half[2]);
}

/// <summary>
/// U+007F より大きいコードが存在したら1を返す。
/// 半角記号も1を返す
/// </summary>
/// <param name="text"></param>
/// <returns></returns>
static int checkOver(const MString& text) {
	for (const wchar_t* ptr = text.c_str() + text.length(); ptr > text.c_str(); ) {
		ptr = text.prev(ptr);
		auto wc = *ptr;
		if (wc > 0x7f) {
			return 1;
		}

		switch (wc) {
		case '-':
		case '_':
		case '.':
		case '(':
		case ')':
		case '[':
		case ']':
			continue;
		}

		if ('0' <= wc && wc <= '9') {
			continue;
		}
		if ('A' <= wc && wc <= 'Z') {
			continue;
		}
		if ('a' <= wc && wc <= 'z') {
			continue;
		}

		return 1; // 半角記号の残り
	}
	return 0;
}


int ExportGPBPlugin::writeJoint(FILE* fh,
	std::vector<GPBBoneParam>& bone_param,
	std::vector<GPBRef>& refTable,
	int index,
	std::map<UINT, int>& bone_index_id,
	float scaling) {
	float material[16] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	GPBBoneParam& curBone = bone_param[index];

	{
		auto offset = ftell(fh);
		int refIndex = curBone.refIndex;
		refTable[refIndex].offset = offset;
		refTable[refIndex].name = curBone.name;

		GPBBoneParam* pParent = nullptr;
		if (curBone.parent != 0) {
			pParent = &bone_param[bone_index_id[curBone.parent]];
		}
		if (pParent) { // 前進差分
			material[12] = (curBone.org_pos.x - pParent->org_pos.x) * scaling;
			material[13] = (curBone.org_pos.y - pParent->org_pos.y) * scaling;
			material[14] = (curBone.org_pos.z - pParent->org_pos.z) * scaling;
		}

		DWORD nodeType = GPBNODE_JOINT;
		fwrite(&nodeType, sizeof(DWORD), 1, fh);
		fwrite(&material, sizeof(float), 16, fh);

		MString parentName((pParent != nullptr) ? pParent->name : L"");
		MAnsiString parentNameStr = parentName.toAnsiString();
		DWORD parentByteNum = parentNameStr.length();
		fwrite(&parentByteNum, sizeof(DWORD), 1, fh);
		fwrite(parentNameStr.c_str(), sizeof(char), parentByteNum, fh);

		// 子ジョイント数
		DWORD childNum = curBone.children.size();
		fwrite(&childNum, sizeof(DWORD), 1, fh);
		for (int i = 0; i < childNum; ++i) {
			this->writeJoint(fh,
				bone_param, refTable,
				curBone.children[i],
				bone_index_id,
				scaling);
		}

		BYTE camlight[2] = { 0, 0 }; // camera, light
		fwrite(&camlight, sizeof(BYTE), 2, fh);

		{ // model
			MAnsiString meshName("");
			DWORD meshNameByteNum = meshName.length();
			fwrite(&meshNameByteNum, sizeof(DWORD), 1, fh);
			fwrite(meshName.c_str(), sizeof(char), meshNameByteNum, fh);
		}
	}

	return 1;
}

/// <summary>
/// バイナリファイルを書き出す
/// </summary>
/// <param name="index"></param>
/// <param name="filename">書き出しファイル名</param>
/// <param name="doc"></param>
/// <returns></returns>
BOOL ExportGPBPlugin::ExportFile(int index, const wchar_t *filename, MQDocument doc)
{
	std::wstring lang = GetSettingValue(MQSETTINGVALUE_LANGUAGE);
#ifdef _WIN32
	// DLLフォルダ
	MString dir = MFileUtil::extractDirectory(s_DllPath);
#else
	MString dir = GetResourceDir();
#endif
	MString path = MFileUtil::combinePath(dir, L"ExportGPB.resource.xml");
	MLanguage language;
	language.Load(lang, path.c_str());

	// Load bone setting
	LoadBoneSettingFile();

	MString outputFiles = MString(filename);

	MString onlyName = MFileUtil::extractFileNameOnly(filename);
	{
		auto result = checkOver(onlyName);
		if (result) {
			MQWindow mainwin = MQWindow::GetMainWindow();
			MString message = MString(language.Search("InvalidModelChar"))
				+ L"\n"
				+ filename;
			MQDialog::MessageWarningBox(mainwin,
				message.c_str(),
				GetResourceString("Error"));
			return FALSE;
		}
	}

	MString keepName = L"unknown";

	MQBoneManager bone_manager(this, doc);
	std::vector<GPBMaterial> materials;

	// ボーン処理をトータルで有効にするかどうか
	bool outputBone = false;

	int indexMesh = 0;
	int indexScene = 1;
	int indexAnimations = 2;
	int indexNode = 3;
	std::vector<GPBRef> refTable;
	for (int i = 0; i < 4; ++i) {
		GPBRef ref;
		ref.offset = 0x00363534; // for check
		switch (i) {
		case 0:
			ref.type = REF_MESH;
			ref.name = MString(L"n0_Mesh");
			break;
		case 1:
			ref.type = REF_SCENE;
			ref.name = MString(L"__SCENE__");
			break;
		case 2:
			ref.type = REF_ANIMATIONS;
			ref.name = MString(L"__Animations__");
			break;
		case 3:
			ref.type = REF_NODE;
			ref.name = MString(L"n0");
			break;
		}
		refTable.push_back(ref);
	}



	GPBScene scene;

	// Query a number of bones ボーン数
	int bone_num = bone_manager.GetBoneNum();
	int bone_object_num = bone_manager.GetSkinObjectNum();

	// Enum bones ボーンのIDリスト
	std::vector<UINT> bone_id;
	std::vector<GPBBoneParam> bone_param;
	if (bone_num > 0) {
		bone_id.resize(bone_num); // 領域確保する
		bone_manager.EnumBoneID(bone_id); // IDリストを取得

		bone_param.resize(bone_num); // 領域確保する
		for (int i = 0; i < bone_num; i++) {
			bone_param[i].id = bone_id[i];

			std::wstring name;
			// ボーンID指定して受け取り変数を指定する
			bone_manager.GetParent(bone_id[i], bone_param[i].parent);
			// 子ボーン個数
			bone_manager.GetChildNum(bone_id[i], bone_param[i].child_num);

			// 位置(相対?global?)
			bone_manager.GetBasePos(bone_id[i], bone_param[i].org_pos);
			// 変形後位置(相対?global?)
			bone_manager.GetDeformPos(bone_id[i], bone_param[i].def_pos);

			bone_manager.GetBaseMatrix(bone_id[i], bone_param[i].base_mtx);
			bone_manager.GetDeformMatrix(bone_id[i], bone_param[i].mtx);

			//bone_manager.GetBaseScale();
			bone_manager.GetDeformScale(bone_id[i], bone_param[i].scale);

			bone_manager.GetName(bone_id[i], name);
			bone_manager.GetDummy(bone_id[i], bone_param[i].dummy);

			bone_param[i].name = MString(name);
			//std::vector<UINT> children;
			//bone_manager.GetChildren(bone_id[i], children);
		}
	}


	// Show a dialog for converting axes
	// 座標軸変換用ダイアログの表示
	// スケーリング倍率 初期デフォルト値
	float scaling = 1;

	CreateDialogOptionParam option;
	option.plugin = this;
	option.lang = &language;
	option.visible_only = false;
	option.mtlfile = FILEOUT_CONFIRM;

	option.bone_exists = (bone_num > 0);
	option.output_bone = 0;

	option.hspfile = FILEOUT_CONFIRM;
	option.struct_mode = STRUCT_SIMPLE;
	// ファイル名だけ取り出して20文字に制限
	//option.modelname = getMultiBytesSubstring(MFileUtil::extractFileNameOnly(filename).toAnsiString(), 20);
	option.texture_prefix = std::wstring(L"res/");

	option.input_xmlanim = FILEIN_NOTUSE;

	// Load a setting. 存在する場合はその値を使う
	MQSetting *setting = OpenSetting();
	if (setting != NULL) {
		setting->Load("VisibleOnly", option.visible_only, option.visible_only);
		setting->Load("MtlFile", option.mtlfile, option.mtlfile);
		setting->Load("HspFile", option.hspfile, option.hspfile);
		setting->Load("TexturePrefix", option.texture_prefix, option.texture_prefix);
		setting->Load("OutputBone", option.output_bone, option.output_bone);
		setting->Load("InputXmlAnimFile", option.input_xmlanim, option.input_xmlanim);
	}
	MQFileDialogInfo dlginfo;
	memset(&dlginfo, 0, sizeof(dlginfo));
	dlginfo.dwSize = sizeof(dlginfo);
	dlginfo.scale = scaling;
	// 軸の関係性は変更させない
	dlginfo.hidden_flag = MQFileDialogInfo::HIDDEN_AXIS | MQFileDialogInfo::HIDDEN_INVERT_FACE;
#if (USESCALING != 0)
#else
	dlginfo.hidden_flag |= MQFileDialogInfo::HIDDEN_SCALE;
#endif

	dlginfo.axis_x = MQFILE_TYPE_RIGHT;
	dlginfo.axis_y = MQFILE_TYPE_UP;
	// PMD では FRONT 指定してあった
	dlginfo.axis_z = MQFILE_TYPE_BACK;
	dlginfo.softname = "";
	dlginfo.dialog_callback = CreateDialogOption;
	dlginfo.dialog_callback_ptr = &option;

#if 1
	MQ_ShowFileDialog("GPB Export", &dlginfo);

	{
		keepName += MString::format(L", additive_info, %d",
			option.additive_info);
	}

#else
	MQFileDialogCallbackParam dialogParam;
	dialogParam.dialog_id = 0; // どうやって算出するの??
	dialogParam.parent_frame_id = 0; // どうやって算出するの??
	//dialogParam.parent_frame_id = MQWidgetBase::GetSystemWidgetID(MQSystemWidget::MainWindow); // NOTE: 
	CreateDialogOption(true, &dialogParam, &option);
	auto optionResult = option.dialog->Execute();
	if (optionResult == MQDialog::DIALOG_CANCEL) {
		return FALSE;
	}
	CreateDialogOption(false, &dialogParam, &option);
#endif


	// Save a setting.
	scaling = dlginfo.scale;
	if(setting != NULL){
		setting->Save("VisibleOnly", option.visible_only);
		setting->Save("MtlFile", option.mtlfile);
		setting->Save("HspFile", option.hspfile);
		setting->Save("TexturePrefix", option.texture_prefix);
		if (option.bone_exists) {
			setting->Save("OutputBone", option.output_bone);
		}
		setting->Save("InputXmlAnimFile", option.input_xmlanim);
		CloseSetting(setting);
	}

	outputBone = (option.output_bone != 0);
	if (!outputBone) { // 無効化する
		bone_num = 0;
		bone_object_num = 0;
	}

	if (!outputBone) {
		GPBRef ref;
		ref.offset = 0x00363534; // for check
		ref.type = REF_NODE;
		ref.name = MString(L"n0_Joint");
		refTable.push_back(ref);
	}


	//// 処理後半

	// サブパスは取れる
	MString contentDir = MFileUtil::extractDirectory(filename);
	MString materialPath = MFileUtil::changeExtension(filename, L".material");
	MString xmlAnimPath = MFileUtil::changeExtension(filename, L".xml");

	// \\ できれいにつながる
	//MString hspPath = MFileUtil::extractDirectory(filename) + onlyName + L".hsp";
	MString hspPath = MFileUtil::extractDirectory(contentDir.substring(0, contentDir.length() - 1)) + onlyName + L".hsp";

	
	int numObj = doc->GetObjectCount();
	int numMat = doc->GetMaterialCount();

	// 頂点をひとまとめにする（単一オブジェクトしか扱えないので）
	std::vector<MQExportObject*> expobjs(numObj, nullptr);
	std::vector<std::vector<int>> orgvert_vert(numObj);
	std::vector<int> vert_orgobj;
	std::vector<int> vert_expvert;
	// 法線
	std::vector<MQPoint> vert_normal;
	// UV
	std::vector<MQCoordinate> vert_coord;
	// 分解して全部足した後の頂点数になる
	int total_vert_num = 0;

	for(int oi=0; oi<numObj; oi++)
	{
		MQObject org_obj = doc->GetObject(oi);
		if(org_obj == NULL)
			continue;

		if(option.visible_only && org_obj->GetVisible() == 0)
			continue;

		MQExportObject::MSeparateParam separate;
		separate.SeparateNormal = true;
		separate.SeparateUV = true;
		separate.SeparateVertexColor = false;
		MQExportObject *eobj = new MQExportObject(org_obj, separate);
		expobjs[oi] = eobj;

		int vert_num = eobj->GetVertexCount();
		orgvert_vert[oi].resize(vert_num, -1);
		for (int evi=0; evi<vert_num; evi++) {
			orgvert_vert[oi][evi] = total_vert_num;

			vert_orgobj.push_back(oi); // 元のオブジェクトIDを追加する
			vert_expvert.push_back(evi); // 元のオブジェクト内での頂点インデックスを追加する

			MQPoint nrm = eobj->GetVertexNormal(evi);
			MQCoordinate uv = eobj->GetVertexCoordinate(evi);

			vert_normal.push_back(nrm);
			vert_coord.push_back(uv);
			total_vert_num ++;
		}
	}

	// ID から bone_num の index を引く
	std::map<UINT, int> bone_id_index;
	// Initialize bones.
	if(bone_num > 0)
	{
		for(int i=0; i<bone_num; i++) {
			bone_id_index[bone_param[i].id] = i;
		}

		// Check the parent
		for (int i=0; i<bone_num; i++) {
			if (bone_param[i].parent != 0) {
				// キーで探して見つからなかったら 0 を代入
				if (bone_id_index.end() == bone_id_index.find(bone_param[i].parent)) {
					assert(0);
					bone_param[i].parent = 0;
				}
			}
		}

		{ // 親参照がある順に並べる
			std::list<GPBBoneParam> bone_param_temp(bone_param.begin(), bone_param.end());
			bone_param.clear();
			bone_id_index.clear();
			while (!bone_param_temp.empty()) {
				bool done = false;
				for (auto it = bone_param_temp.begin(); it != bone_param_temp.end(); ) {
					if ((*it).parent != 0) { // 親がある場合
						if (bone_id_index.end() != bone_id_index.find((*it).parent)) {
							// 親が bone_id_index から見つかる場合
							bone_id_index[(*it).id] = (int)bone_param.size();
							bone_param.push_back(*it);
							it = bone_param_temp.erase(it);
							done = true;
						}
						else {
							// 親が bone_id_index にまだ追加されてない場合は候補ではない
							++it; // try next
						}
					}
					else {
						// 親が無い場合

						// インデックスを指定して追加して元から削除して1個以上あるフラグを立てる
						bone_id_index[(*it).id] = (int)bone_param.size();
						bone_param.push_back(*it);
						it = bone_param_temp.erase(it);
						done = true;
					}
				}

				assert(done);
				if (!done) {
					// 辻褄があわず1個も見つからなかった場合，残り全部の親を無効化して全部登録
					for (auto it = bone_param_temp.begin(); it != bone_param_temp.end(); ++it) {
						bone_id_index[(*it).id] = (int)bone_param.size();
						(*it).parent = 0;
						bone_param.push_back(*it);
					}
					break;
				}
			}
		}

		// Enum children. 親が有効だった場合に自分を親の子リストにソート後インデックスを追加する
		for (int i=0; i<bone_num; i++) {
			if (bone_param[i].parent != 0) {
				if (bone_id_index.end() != bone_id_index.find(bone_param[i].parent)) {
					bone_param[bone_id_index[bone_param[i].parent]].children.push_back(i);
				} else {
					assert(0);
					bone_param[i].parent = 0;
				}
			}
		}

		for (int i = 0; i < bone_num; ++i) {
			bone_param[i].sortedIndex = i;

			// name. en もこれでいいのか??
			bone_param[i].name_jp = bone_param[i].name;
			bone_param[i].name_en = bone_param[i].name;
			// 設定を全部見て設定の jp or en のどちらかと一致したら設定で上書きする
			for (auto it = m_BoneNameSetting.begin(); it != m_BoneNameSetting.end(); ++it) {
				if ((*it).jp == bone_param[i].name || (*it).en == bone_param[i].name) {
					bone_param[i].name_jp = (*it).jp;
					bone_param[i].name_en = (*it).en;
					break;
				}
			}

			GPBRef ref;
			ref.type = REF_NODE;
			ref.name = bone_param[i].name_en;
			bone_param[i].refIndex = refTable.size();
			refTable.push_back(ref);
		}

		{
			keepName += MString::format(L", refTable, %d, %d, %s",
				refTable.size(),
				bone_param.size(),
				outputBone ? L"true" : L"false");
		}
	}

	for (int m = 0; m <= numMat; ++m) {
		GPBMaterial material;
		material.orgIndex = m;

		BOOL isDouble = FALSE;
		MQColor col(1, 1, 1);
		float dif = 0.8f;
		float alpha = 1.0f;
		float spc_pow = 5.0f;
		MQColor spc_col(0, 0, 0);
		MQColor amb_col(0.6f, 0.6f, 0.6f);
		MString texture;

		if (m < numMat) {
			MQMaterial mat = doc->GetMaterial(m);
			if (mat != NULL) {
				material.orgName = mat->GetNameW();

				isDouble = mat->GetDoubleSided();
				//int vertexColor = mat->GetVertexColor();

				col = mat->GetColor();
				dif = mat->GetDiffuse();
				alpha = mat->GetAlpha();
				spc_pow = mat->GetPower();
				spc_col = mat->GetSpecularColor();
				amb_col = mat->GetAmbientColor();
				wchar_t path[_MAX_PATH];
				mat->GetTextureNameW(path, _MAX_PATH);
				// foo.png だけ取り出す
				texture = MFileUtil::extractFilenameAndExtension(MString(path));
				if (texture.length() > 0) {
					material.useTexture = true;

					material.wrapU = mat->GetWrapModeU();
					material.wrapV = mat->GetWrapModeV();
					{ // 境界処理の制限
						if (material.wrapU != MQMATERIAL_WRAP_REPEAT
							&& material.wrapU != MQMATERIAL_WRAP_CLAMP) {
							material.wrapU = MQMATERIAL_WRAP_REPEAT;
						}
						if (material.wrapV != MQMATERIAL_WRAP_REPEAT
							&& material.wrapV != MQMATERIAL_WRAP_CLAMP) {
							material.wrapV = MQMATERIAL_WRAP_REPEAT;
						}
					}

					material.filter = mat->GetMappingFilter();
				}

				// MQPlugin.h
				int shader = mat->GetShader();
				switch (shader) {
				case MQMATERIAL_SHADER_CONSTANT:
					material.useLighting = false;
					dif = 1.0f;
					break;
				case MQMATERIAL_SHADER_HLSL:
					MAnsiString shader_name = mat->GetShaderName();
					// Constant ->"", Phong -> "", "pmd", "vrm", "glTF", "PBRTransparent"
						//toon = mat->GetShaderParameterIntValue("Toon", 0);
					if (shader_name == "vrm") {
						material.useLighting = false;
					}
					break;
				}

			}
		}
		else {
			material.orgName = L"__material__";
			material.convName = material.orgName;
		}

		// dr, dg, db // 減衰色
		material.diffuse[0] = col.r * dif;
		material.diffuse[1] = col.g * dif;
		material.diffuse[2] = col.b * dif;
		material.diffuse[3] = alpha;

		material.isDouble = isDouble;

		// sr, sg, sb // 光沢色
		material.specular[0] = sqrtf(spc_col.r);
		material.specular[1] = sqrtf(spc_col.g);
		material.specular[2] = sqrtf(spc_col.b);

		float ambient_color[3]; // mr, mg, mb // 環境色(ambient)
		ambient_color[0] = amb_col.r;
		ambient_color[1] = amb_col.g;
		ambient_color[2] = amb_col.b;

		//DWORD face_vert_count = material_used[i] * 3;

		//MAnsiString texture_str = getMultiBytesSubstring(texture, 20);
		//char texture_file_name[20];
		//memcpy(texture_file_name, texture_str.c_str(), texture_str.length());
		//fwrite(texture_file_name, 20, 1, fh);

		material.orgDiffuseTexture = texture;

		material.convName = material.orgName;
		if (material.useTexture) {
			material.convDiffuseTexture = MString(option.texture_prefix) + material.orgDiffuseTexture;
		}

		materials.push_back(material);
	}

	// Face's vertices list 面頂点リストを生成する
	DWORD face_vert_count = 0;
	std::vector<int> material_used(numMat + 1, 0);
	for (int i = 0; i < numObj; i++) {
		MQObject obj = doc->GetObject(i);
		if (obj == NULL)
			continue;

		if (option.visible_only && obj->GetVisible() == 0)
			continue;

		int num_face = obj->GetFaceCount();
		for (int fi = 0; fi < num_face; fi++) {
			int n = obj->GetFacePointCount(fi);
			if (n >= 3) {
				face_vert_count += (n - 2) * 3;

				int mi = obj->GetFaceMaterial(fi);
				if (mi < 0 || mi >= numMat) mi = numMat;
				material_used[mi] += (n - 2);
			}
		}
	}

	int output_face_vert_count = 0;
	for (int m = 0; m <= numMat; m++)
	{
		if (material_used[m] == 0) {
			materials[m].enable = false;
			continue;
		}

		for (int i = 0; i < numObj; i++) {
			MQObject obj = doc->GetObject(i);
			if (obj == NULL)
				continue;

			if (option.visible_only && obj->GetVisible() == 0)
				continue;

			MQExportObject* eobj = expobjs[i];
			if (eobj == nullptr)
				continue;

			int num_face = obj->GetFaceCount();
			for (int fi = 0; fi < num_face; fi++) {
				// 面の材質(インデックス)を取得する
				int mi = obj->GetFaceMaterial(fi);
				// 存在する材質でない場合は，特別材質扱いとする
				if (mi < 0 || mi >= numMat) mi = numMat;

				int n = eobj->GetFacePointCount(fi);
				if (n >= 3 && mi == m) { // 面であって今回のループに該当する材質だった場合のみ
					std::vector<int> vi(n);
					std::vector<MQPoint> p(n);

					eobj->GetFacePointArray(fi, vi.data());
					for (int j = 0; j < n; j++) {
						p[j] = obj->GetVertex(eobj->GetOriginalVertex(vi[j]));
					}
					std::vector<int> tri((n - 2) * 3);
					doc->Triangulate(p.data(), n, tri.data(), (n - 2) * 3);
					// 面頂点
					for (int j = 0; j < n - 2; j++) {
						int tvi[3];
						tvi[0] = orgvert_vert[i][vi[tri[j * 3]]];
						tvi[1] = orgvert_vert[i][vi[tri[j * 3 + 1]]];
						tvi[2] = orgvert_vert[i][vi[tri[j * 3 + 2]]];

						materials[mi].faceIndices.push_back(tvi[0]);
						materials[mi].faceIndices.push_back(tvi[2]);
						materials[mi].faceIndices.push_back(tvi[1]);

						output_face_vert_count += 3;
					}
				}
			}
		}
	}
	assert(face_vert_count == output_face_vert_count);

	// 実際に有効な材質の個数カウント
	DWORD enableMaterialNum = 0;
	for (const auto& material : materials) {
		if (material.enable) {
			enableMaterialNum += 1;

			{
				auto result = checkOver(material.convName);
				if (result) {
					MQWindow mainwin = MQWindow::GetMainWindow();
					MString message = MString(language.Search("InvalidMaterialChar"))
						+ L"\n"
						+ material.convName;
					MQDialog::MessageWarningBox(mainwin,
						message.c_str(),
						GetResourceString("Error"));
					for (size_t i = 0; i < expobjs.size(); i++) {
						delete expobjs[i];
					}
					return FALSE;
				}
			}

			if (material.useTexture) {
				auto result = checkOver(material.convDiffuseTexture);
				if (result) {
					MQWindow mainwin = MQWindow::GetMainWindow();
					MString message = MString(language.Search("InvalidTextureChar"))
						+ L"\n"
						+ material.convDiffuseTexture
						+ L" in "
						+ material.orgName;
					MQDialog::MessageWarningBox(mainwin,
						message.c_str(),
						GetResourceString("Error"));

					for (size_t i = 0; i < expobjs.size(); i++) {
						delete expobjs[i];
					}
					return FALSE;
				}
			}
		}
	}


	// ジョイント名リスト
	int rootJointNum = 0;
	std::vector<MString> jointNames;
	if (outputBone) {
		for (const auto& bone : bone_param) {
			if (bone.parent == 0) {
				rootJointNum += 1;
			}
			jointNames.push_back(bone.name_en);
		}
	}
	else {
		rootJointNum = 1;
		jointNames.push_back(L"n0_Joint");
	}

	for (const MString& jointName : jointNames) {
		auto result = checkOver(jointName);
		if (!result) {
			continue;
		}
		MQWindow mainwin = MQWindow::GetMainWindow();
		MString message = MString(language.Search("InvalidBoneChar"))
			+ L"\n"
			+ jointName;
		MQDialog::MessageWarningBox(mainwin,
			message.c_str(),
			GetResourceString("Error"));
		return FALSE;
	}

	// 1つのアニメーションチャンクのデータ
	ANIMATIONS animations;

	if (outputBone && option.input_xmlanim) {
		// 0個になってもそのまま
		auto result = this->loadAnimation(xmlAnimPath,
			animations);
	}


	//// Open a file.
	FILE *fh;
	errno_t err = _wfopen_s(&fh, filename, L"wb");
	if(err != 0) {
		for(size_t i=0; i<expobjs.size(); i++) {
			delete expobjs[i];
		}
		return FALSE;
	}

	FILE* fhMaterial = nullptr;
	if (option.mtlfile != FILEOUT_NO) {
		bool tryWrite = false;
		switch (option.mtlfile) {
		case FILEOUT_CONFIRM:
		case FILEOUT_OWFORBIDDEN:
			//access();
			//PathFileExist();
			err = _wfopen_s(&fhMaterial, materialPath.c_str(), L"r");
			if (err) { // 存在しないので書き出す
				tryWrite = true;
				fhMaterial = nullptr;
			}
			else {
				fclose(fhMaterial);
				fhMaterial = nullptr;
				tryWrite = false;
				if (option.mtlfile == FILEOUT_CONFIRM) {
					MQWindow mainwin = MQWindow::GetMainWindow();
					MString message = MString(language.Search("OverwriteConfirm"))
						+ L"\n"
						+ materialPath;
					const auto result = MQDialog::MessageOkCancelBox(mainwin,
						message.c_str(),
						language.Search("Option"));
					tryWrite = (result == MQDialog::DIALOG_RESULT::DIALOG_OK);
				}
			}
			break;
		case FILEOUT_FORCE:
			tryWrite = true;
			break;
		}

		if (tryWrite) {
			err = _wfopen_s(&fhMaterial, materialPath.c_str(), L"w");
			if (err != 0) {
				fhMaterial = nullptr;
			}

			if (fhMaterial) {
				outputFiles += L"\n" + materialPath;
			}
		}
	}

	FILE* fhHsp = nullptr;
	if (option.hspfile != FILEOUT_NO) {
		bool tryWrite = false;
		switch (option.hspfile) {
		case FILEOUT_CONFIRM:
		case FILEOUT_OWFORBIDDEN:
			//access();
			//PathFileExist();
			err = _wfopen_s(&fhHsp, hspPath.c_str(), L"r");
			if (err) { // 存在しないので書き出す
				tryWrite = true;
				fhHsp = nullptr;
			}
			else {
				fclose(fhHsp);
				fhHsp = nullptr;
				tryWrite = false;
				if (option.hspfile == FILEOUT_CONFIRM) {
					MQWindow mainwin = MQWindow::GetMainWindow();
					MString message = MString(language.Search("OverwriteConfirm"))
						+ L"\n"
						+ hspPath;
					const auto result = MQDialog::MessageOkCancelBox(mainwin,
						message.c_str(),
						language.Search("Option"));
					tryWrite = (result == MQDialog::DIALOG_RESULT::DIALOG_OK);
				}
			}
			break;
		case FILEOUT_FORCE:
			tryWrite = true;
			break;
		}

		if (tryWrite) {
			err = _wfopen_s(&fhHsp, hspPath.c_str(), L"w");
			if (err != 0) {
				fhHsp = nullptr;
			}

			if (fhHsp) {
				outputFiles += L"\n" + hspPath;
			}
		}
	}

	//// Headerの書き出し
	BYTE major = 1;
	BYTE minor = 5;
	fwrite("\xabGPB\xbb\x0d\x0a\x1a\x0a", 9, 1, fh);
	fwrite(&major, sizeof(BYTE), 1, fh);
	fwrite(&minor, sizeof(BYTE), 1, fh);

	//// 参照テーブルの書き出し
	std::vector<DWORD> checkValues;
	checkValues.push_back(0x39393901);

	DWORD refNum = refTable.size();
	fwrite(&refNum, sizeof(DWORD), 1, fh);
	for (auto& ref : refTable) // 書き込み有り
	{
		DWORD type = ref.type;
		// この時点のオフセットはダミー
		DWORD offset = ref.offset;
		// バイト 名前
		MAnsiString nameStr = ref.name.toAnsiString();
		DWORD byteNum = nameStr.length(); // size_t は大きすぎる
		fwrite(&byteNum, sizeof(DWORD), 1, fh);
		fwrite(nameStr.c_str(), sizeof(char), byteNum, fh);
		// タイプ
		fwrite(&type, sizeof(DWORD), 1, fh);
		// オフセット位置
		ref.writeOffset = ftell(fh); // 保持する必要がある
		fwrite(&offset, sizeof(DWORD), 1, fh);
	}

	//// メッシュ
	GPBBounding wholeBounding;

	DWORD meshNum = 1;
	fwrite(&meshNum, sizeof(DWORD), 1, fh);
	for (int i = 0; i < meshNum; i++)
	{
		refTable[indexMesh].offset = ftell(fh);

		GPBBounding bounding;

		// 属性タイプと数値数の配列 position, 3 など
		DWORD attrNum = outputBone ? 5 : 3;
		fwrite(&attrNum, sizeof(DWORD), 1, fh);
		DWORD attr[10] = {
			ATTR_POSITION, 3,
			ATTR_NORMAL, 3,
			ATTR_TEXCOORD0, 2,
			ATTR_BLENDWEIGHTS, 4,
			ATTR_BLENDINDICES, 4,
		};
		fwrite(&attr, sizeof(DWORD), attrNum * 2, fh);

		DWORD attrFloatNum = 3 + 3 + 2 + (outputBone ? (4 + 4) : 0);
		DWORD vertexByteCount = total_vert_num * attrFloatNum * sizeof(float);
		fwrite(&vertexByteCount, sizeof(DWORD), 1, fh);

		for (int j = 0; j < total_vert_num; ++j) {
			float pos[3];
			float nrm[3];
			float uv[2];

			MQObject obj = doc->GetObject(vert_orgobj[j]);
			MQExportObject* eobj = expobjs[vert_orgobj[j]];
			MQPoint v = obj->GetVertex(eobj->GetOriginalVertex(vert_expvert[j]));
			pos[0] = v.x * scaling;
			pos[1] = v.y * scaling;
			pos[2] = v.z * scaling;

			nrm[0] = vert_normal[j].x;
			nrm[1] = vert_normal[j].y;
			nrm[2] = vert_normal[j].z;
			// 元の順
			//uv[0] = vert_coord[j].u;
			//uv[1] = vert_coord[j].v;

			// gpb は多分 v 反転
			uv[0] = vert_coord[j].u;
			uv[1] = 1.0f - vert_coord[j].v;

			/*
			int bone_index[4];
			float bone_weight;

			if (weight_num >= 2) {
				int max_bone1 = -1;
				float max_weight1 = 0.0f;
				for (int n = 0; n < weight_num; n++) {
					if (max_weight1 < weights[n]) {
						max_weight1 = weights[n];
						max_bone1 = n;
					}
				}
				int max_bone2 = -1;
				float max_weight2 = 0.0f;
				for (int n = 0; n < weight_num; n++) {
					if (n == max_bone1) continue;
					if (max_weight2 < weights[n]) {
						max_weight2 = weights[n];
						max_bone2 = n;
					}
				}
				float total_weights = max_weight1 + max_weight2;

				// ルート側のノードにウェイトを割り当てる
				int bi1 = bone_id_index[vert_bone_id[max_bone1]];
				int bi2 = bone_id_index[vert_bone_id[max_bone2]];
				bone_index[0] = bone_param[bi1].sortedIndex;
				bone_index[1] = bone_param[bi2].sortedIndex;
				if (bone_index[0] != bone_index[1]) {
					bone_weight = max_weight1 / total_weights;
				}
				else {
					// index が一致したら1つに統合する
					bone_weight = 1.0f;
				}
			}
			else if (weight_num == 1) { // 1個の場合
				int bi = bone_id_index[vert_bone_id[0]];
				bone_index[0] = bone_param[bi].sortedIndex;
				bone_index[1] = bone_index[0];
				bone_weight = 1.0f;
			}
			else { // 0個の場合
				// 0ボーンに1.0f
				// Do nothing.
			}
			indices[0] = bone_index[0];
			weight[0] = bone_weight;
			*/

			for (int index = 0; index < 3; ++index) {
				bounding.max[index] = fmaxf(bounding.max[index], pos[index]);
				bounding.min[index] = fminf(bounding.min[index], pos[index]);
			}

			fwrite(&pos, sizeof(float), 3, fh);
			fwrite(&nrm, sizeof(float), 3, fh);
			fwrite(&uv, sizeof(float), 2, fh);

			if (outputBone) {

				std::vector<INDEXWEIGHT> iws;
				iws.resize(16);

				// 受け取り用(最大16個まで)
				UINT vert_bone_id[16];
				// 受け取り用
				float weights[16];
				// ウエイトの個数
				int weight_num = 0;
				if (bone_num > 0) { // ボーンが1個以上存在する場合
					UINT vert_id = obj->GetVertexUniqueID(eobj->GetOriginalVertex(vert_expvert[j]));
					// 1頂点にたいして最大16個まで要求する
					int max_num = 16;
					weight_num = bone_manager.GetVertexWeightArray(obj, vert_id, max_num, vert_bone_id, weights);

					for (int k = 0; k < weight_num; ++k) {
						int bi = bone_id_index[vert_bone_id[k]];
						iws[k].sortedIndex = bone_param[bi].sortedIndex;
						iws[k].weight = weights[k];
					}
					std::sort(iws.begin(), iws.end(),
						[](auto const& a, auto const& b) { return (a.weight > b.weight); });
					if (weight_num == 0) {
						iws[0].weight = 1.0f;
					}
				}

				float indices[4] = { 0, 0, 0, 0 };
				float weight[4] = { 1.0, 0.0, 0.0, 0.0 };
				for (int k = 0; k < 4; ++k) {
					indices[k] = iws[k].sortedIndex;
					weights[k] = iws[k].weight;
				}

				fwrite(&weight, sizeof(float), 4, fh);
				fwrite(&indices, sizeof(float), 4, fh);
			}
		}

		calcRadius(bounding);

		fwrite(&bounding.min, sizeof(float), 3, fh);
		fwrite(&bounding.max, sizeof(float), 3, fh);
		fwrite(&bounding.center, sizeof(float), 3, fh);
		fwrite(&bounding.radius, sizeof(float), 1, fh);

		fwrite(&enableMaterialNum, sizeof(DWORD), 1, fh);
		for (const auto& material : materials) {
			if (!material.enable) {
				continue;
			}
			DWORD type = GL_TRIANGLE; // TRI or LINE
			DWORD format = GL_UNSIGNED_INT; // u16 or u32
			DWORD byteNum = material.faceIndices.size() * sizeof(unsigned int);
			fwrite(&type, sizeof(DWORD), 1, fh);
			fwrite(&format, sizeof(DWORD), 1, fh);
			fwrite(&byteNum, sizeof(DWORD), 1, fh);
			// 面頂点
			for (const auto& index : material.faceIndices) {
				unsigned int index32 = static_cast<unsigned int>(index);
				fwrite(&index32, sizeof(unsigned int), 1, fh);
			}
		}

		for (int j = 0; j < 3; ++j) {
			wholeBounding.max[j] = fmaxf(wholeBounding.max[j], bounding.max[j]);
			wholeBounding.min[j] = fminf(wholeBounding.min[j], bounding.min[j]);
		}
	}

	calcRadius(wholeBounding);


	DWORD elementNum = 2;
	fwrite(&elementNum, sizeof(DWORD), 1, fh);

	//// シーンの書き出し
	refTable[indexScene].offset = ftell(fh);

	float identity[16] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};


	DWORD nodeNum = 1 + rootJointNum;
	fwrite(&nodeNum, sizeof(DWORD), 1, fh);

	{ // mesh を持つ node node
		auto offset = ftell(fh);
		DWORD nodeType = GPBNODE_NODE;
		refTable[indexNode].offset = offset;

		fwrite(&nodeType, sizeof(DWORD), 1, fh);
		fwrite(&identity, sizeof(float), 16, fh);

		MAnsiString parentStr("");
		DWORD parentByteNum = parentStr.length();
		fwrite(&parentByteNum, sizeof(DWORD), 1, fh);
		fwrite(parentStr.c_str(), sizeof(char), parentByteNum, fh);

		DWORD childNum = 0;
		fwrite(&childNum, sizeof(DWORD), 1, fh);

		BYTE camlight[2] = { 0, 0 }; // camera, light
		fwrite(&camlight, sizeof(BYTE), 2, fh);

		// model
		DWORD zero = 0;

		MAnsiString meshName("#n0_Mesh");
		DWORD meshNameByteNum = meshName.length();
		fwrite(&meshNameByteNum, sizeof(DWORD), 1, fh);
		fwrite(meshName.c_str(), sizeof(char), meshNameByteNum, fh);

		BYTE hasSkin = 1;
		fwrite(&hasSkin, sizeof(BYTE), 1, fh);
		if (hasSkin) {
			fwrite(&identity, sizeof(float), 16, fh); // bindShape

			DWORD jointCount = jointNames.size();
			fwrite(&jointCount, sizeof(DWORD), 1, fh);
			for (const auto& jointName : jointNames) {
				MAnsiString boneNameRef = MString(L"#" + jointName).toAnsiString();
				DWORD boneNameByteNum = boneNameRef.length();
				fwrite(&boneNameByteNum, sizeof(DWORD), 1, fh);
				fwrite(boneNameRef.c_str(), sizeof(char), boneNameByteNum, fh);
			}

			DWORD inverseNum = jointCount * 16;
			fwrite(&inverseNum, sizeof(DWORD), 1, fh);
			for (int i = 0; i < jointCount; ++i) {
				// TODO: グローバル位置の負
				if (outputBone) {
					identity[12] = -bone_param[i].org_pos.x * scaling;
					identity[13] = -bone_param[i].org_pos.y * scaling;
					identity[14] = -bone_param[i].org_pos.z * scaling;
				}
				fwrite(&identity, sizeof(float), 16, fh);
			}
		}

		fwrite(&enableMaterialNum, sizeof(DWORD), 1, fh);
		for (const auto& material : materials) {
			if (!material.enable) {
				continue;
			}
			MAnsiString materialName(material.convName.toAnsiString());
			DWORD nameByteNum = materialName.length();
			fwrite(&nameByteNum, sizeof(DWORD), 1, fh);
			fwrite(materialName.c_str(), sizeof(char), nameByteNum, fh);
		}

	}

	if (outputBone) {
		// 結果的に rootJointNum だけ採用される
		for (int i = 0; i < bone_num; ++i) {
			if (bone_param[i].parent != 0) {
				continue;
			}
			this->writeJoint(fh,
				bone_param,
				refTable, i,
				bone_id_index,
				scaling);
		}
	} else {

		auto offset = ftell(fh);
		DWORD nodeType = GPBNODE_JOINT;
		// TODO: 4 でいいのか??
		refTable[4].offset = offset;

		fwrite(&nodeType, sizeof(DWORD), 1, fh);
		fwrite(&identity, sizeof(float), 16, fh);

		MAnsiString parentStr("");
		DWORD parentByteNum = parentStr.length();
		fwrite(&parentByteNum, sizeof(DWORD), 1, fh);
		fwrite(parentStr.c_str(), sizeof(char), parentByteNum, fh);

		DWORD childNum = 0;
		fwrite(&childNum, sizeof(DWORD), 1, fh);

		BYTE camlight[2] = { 0, 0 }; // camera, light
		fwrite(&camlight, sizeof(BYTE), 2, fh);

		// model
		DWORD zero = 0;
		fwrite(&zero, sizeof(DWORD), 1, fh);
	}


	DWORD cameraNameLength = scene.cameraName.length();
	fwrite(&cameraNameLength, sizeof(DWORD), 1, fh);
	fwrite(scene.cameraName.c_str(), sizeof(char), cameraNameLength, fh);

	fwrite(&scene.ambient, sizeof(float), 3, fh);

	//// アニメーションの書き出し
	refTable[indexAnimations].offset = ftell(fh);
	this->writeAnimations(fh, animations);


	// 動作チェック用の追加書き出し
	//for (const auto& val : checkValues) {
	//	fwrite(&val, sizeof(DWORD), 1, fh);
	//}

	//// オフセットの書き戻し
	for (const auto& ref : refTable) {
		fseek(fh, ref.writeOffset, SEEK_SET);
		fwrite(&ref.offset, sizeof(int), 1, fh);
	}


	//// バイナリ出力ここまで

		//// 材質の書き出し
	if (fhMaterial) {
		//keepName = L"";
		std::vector<MString> defs;
		this->makeMaterial(fhMaterial, materials,
			keepName,
			bone_num);
	}
	if (fhHsp) {
		MString name = MString(L"res/") + MFileUtil::extractFileNameOnly(filename);
		if (!outputBone) {
			jointNames.clear();
		}
		this->makeHSP(fhHsp, wholeBounding, name,
			jointNames);
	}


	for(size_t i=0; i<expobjs.size(); i++) {
		delete expobjs[i];
	}

	if (fhMaterial) {
		err = fclose(fhMaterial);
	}
	if (fhHsp) {
		err = fclose(fhHsp);
	}

	if(fclose(fh) != 0){
		return FALSE;
	}
	
	{
		MQWindow mainwin = MQWindow::GetMainWindow();
		MString message = MString(language.Search("DoneOutput")) + L"\n" + outputFiles;
		const auto result = MQDialog::MessageInformationBox(mainwin,
			message.c_str(),
			language.Search("Option"));
	}

	return TRUE;
}

int ExportGPBPlugin::writeAnimations(FILE* fh,
	const ANIMATIONS& animations) {

	DWORD animationNum = animations.anims.size();
	fwrite(&animationNum, sizeof(DWORD), 1, fh);
	for (int i = 0; i < animationNum; ++i) {
		const auto anim = animations.anims[i];

		MAnsiString animationName = MString(anim.id).toAnsiString(); // "animations"; // この名前であることが必要
		DWORD animationNameByteNum = animationName.length();
		fwrite(&animationNameByteNum, sizeof(DWORD), 1, fh);
		fwrite(animationName.c_str(), sizeof(char), animationNameByteNum, fh);

		DWORD channelNum = anim.channels.size();
		fwrite(&channelNum, sizeof(DWORD), 1, fh);
		for (int j = 0; j < channelNum; ++j) {
			const auto ch = anim.channels[j];
			MAnsiString targetName = MString(ch.targetId).toAnsiString();

			DWORD targetNameLength = targetName.length();
			fwrite(&targetNameLength, sizeof(DWORD), 1, fh);
			fwrite(targetName.c_str(), sizeof(char), targetNameLength, fh);
			DWORD valType = ch.attribVal; // tamane2 は 16 回転と移動
			fwrite(&valType, sizeof(DWORD), 1, fh);

			// キー配列
			DWORD keyNum = ch.keytimes.size();
			fwrite(&keyNum, sizeof(DWORD), 1, fh);
			for (const auto& keyval : ch.keytimes) {
				fwrite(&keyval, sizeof(DWORD), 1, fh);
			}

			/*
			// 値配列 個数
			DWORD valNum = keyNum * (4 + 3);
			fwrite(&valNum, sizeof(DWORD), 1, fh);
			for (const auto& keyval : ch.values) {
				if (false) {
					fwrite(&keyval.sx, sizeof(float), 1, fh);
					fwrite(&keyval.sy, sizeof(float), 1, fh);
					fwrite(&keyval.sz, sizeof(float), 1, fh);
				}
				if (true) {
					fwrite(&keyval.qx, sizeof(float), 1, fh);
					fwrite(&keyval.qy, sizeof(float), 1, fh);
					fwrite(&keyval.qz, sizeof(float), 1, fh);
					fwrite(&keyval.qw, sizeof(float), 1, fh);
				}
				if (true) {
					fwrite(&keyval.tx, sizeof(float), 1, fh);
					fwrite(&keyval.ty, sizeof(float), 1, fh);
					fwrite(&keyval.tz, sizeof(float), 1, fh);
				}
			}
			*/
			{
				// 値配列 個数
				DWORD valNum = ch.values.size();
				fwrite(&valNum, sizeof(DWORD), 1, fh);
				for (const auto& val : ch.values) {
					fwrite(&val, sizeof(float), 1, fh);
				}
			}

			{
				DWORD tin = 0;
				fwrite(&tin, sizeof(DWORD), 1, fh);
			}
			{
				DWORD tout = 0;
				fwrite(&tout, sizeof(DWORD), 1, fh);
			}
			{
				DWORD inum = 1;
				fwrite(&inum, sizeof(DWORD), 1, fh);
				for (int k = 0; k < inum; ++k) {
					// tamane2 では type 1 BSPLINE が格納されているが
					// Curve::LINEAR しか対応してないらしい
					DWORD itype = 1;
					fwrite(&itype, sizeof(DWORD), 1, fh);
				}
			}

		}
	}
	return 1;
}

bool ExportGPBPlugin::LoadBoneSettingFile()
{
#ifdef _WIN32
	MString dir = MFileUtil::extractDirectory(s_DllPath);
#else
	MString dir = GetResourceDir();
#endif
	MString filename = MFileUtil::combinePath(dir, L"ExportGPBBoneSetting.xml");
	return false; // 非対応


	MQXmlDocument doc;
	auto result = doc->LoadFile(filename.toAnsiString().c_str());
	if (result != TRUE) {
#ifdef _WIN32
		MString buf = MString::format(L"Failed to load '%s'.\n", filename.c_str());
#endif
		doc->DeleteThis();
		return false;
	}

	const auto root = doc->GetRootElement();
	if(root != NULL) {
		const auto bones = root->FirstChildElement("bones");
		if(bones != NULL) {
			auto elem = bones->FirstChildElement("bone");
			while (elem != NULL) {
				const auto jp = elem->GetAttribute("jp");
				const auto en = elem->GetAttribute("en");
				const auto root = elem->GetAttribute("root");

				BoneNameSetting setting;
				setting.jp = MString::fromUtf8String(jp.c_str());
				setting.en = MString::fromUtf8String(en.c_str());

				m_BoneNameSetting.push_back(setting);

				if(root != nullptr && MAnsiString(root).toInt() != 0){
					m_RootBoneName = setting;
				}

				elem = elem->NextChildElement("bone", elem);
			}
		}

	}

	doc->DeleteThis();
	return true;
}

int ExportGPBPlugin::loadAnimation(MString& animationFile, ANIMATIONS& animations) {
	animations.anims.clear();

	MQXmlDocument doc;
	auto result = doc->LoadFile(animationFile.toAnsiString().c_str());
	if (result != TRUE) {
		doc->DeleteThis();
		return 0;
	}

	const auto root = doc->GetRootElement();
	if (root == NULL) {
		doc->DeleteThis();
		return 0;
	}

	int ret = 0;

	const auto anis = root->FirstChildElement("Animations");
	if (anis == NULL) {
		doc->DeleteThis();
		return 0;
	}

	// NOTE: chunk の都合でこのDLLで強制的な値に設定する
	//const auto anisid = anis->GetAttribute("id");
	animations.id = L"__Animations__";

	auto animelem = anis->FirstChildElement("Animation");
	while (animelem != NULL) {
		ANIMATION anim;

		std::wstring animid;
		animelem->GetAttribute(L"id", animid);
		anim.id = animid;
		if (animid == L"animations") {
			ret = 1;
		}

		auto chel = animelem->FirstChildElement("AnimationChannel");
		while (chel != NULL) {
			ANIMATIONCHANNEL ch;

			const auto tidElem = chel->FirstChildElement("targetId");
			const auto targetId = tidElem->GetTextW();

			const auto attrElem = chel->FirstChildElement("targetAttrib");
			const auto targetAttribText = attrElem->GetTextW();

			const auto ksElem = chel->FirstChildElement("keytimes");
			const auto keytimesText = ksElem->GetTextW();

			const auto vsElem = chel->FirstChildElement("values");
			const auto valuesText = vsElem->GetTextW();

			const auto itpsElem = chel->FirstChildElement("interpolations");
			const auto intersText = itpsElem->GetTextW();

			// "targetId"
			// "targetAttrib"
			// "keytimes" count=""
			// "values" count=""
			// "tangentsIn" count="0"
			// "tangentsOut" count="0"
			// "interpolations" count="1"

			ch.targetId = targetId;
			{
				MString str = MString(targetAttribText);
				auto strs = str.split(L" ");

				auto vstr = strs[0];
				if (vstr.canParseInt()) {
					auto v = vstr.toInt();
					ch.attribVal = v;
				}
			}

			{
				MString str = MString(keytimesText);
				auto strs = str.split(L" ");
				for (auto vstr : strs) {
					if (!vstr.canParseFloat()) {
						continue;
					}
					auto v = vstr.toFloat();
					ch.keytimes.push_back(v);
				}
			}

			{
				MString str = MString(valuesText);
				auto strs = str.split(L" ");
				for (auto vstr : strs) {
					if (!vstr.canParseFloat()) {
						continue;
					}
					auto v = vstr.toFloat();
					ch.values.push_back(v);
				}
			}

			{
				MString str = MString(intersText);
				auto strs = str.split(L" ");
				for (const auto& vstr : strs) {
					if (!vstr.toInt()) {
						continue;
					}
					auto v = vstr.toInt();
					ch.interpolations.push_back(v);
				}
			}

			anim.channels.push_back(ch);

			// NOTE: chel は親を指定するのではないのか?
			chel = chel->NextChildElement("AnimationChannel", chel);
		}

		/*
		BoneNameSetting setting;
		//setting.jp = MString::fromUtf8String(jp.c_str());

		m_BoneNameSetting.push_back(setting);

		if (root != nullptr) {
			m_RootBoneName = setting;
		}*/

		animations.anims.push_back(anim);
	}

	doc->DeleteThis();
	return ret;
}


int ExportGPBPlugin::makeMaterial(FILE* f,
	const std::vector<GPBMaterial>& materials,
	const MString& option,
	int jointNum) {

	std::vector<MString> wraps;
	wraps.push_back(L"REPEAT");
	wraps.push_back(L"MIRROR");
	wraps.push_back(L"CLAMP");

	std::vector<MString> magFilters;
	magFilters.push_back(L"NEAREST");
	magFilters.push_back(L"LINEAR");

	std::vector<MString> minFilters;
	minFilters.push_back(L"NEAREST_MIPMAP_NEAREST");
	minFilters.push_back(L"LINEAR_MIPMAP_LINEAR");

	if (option.length() > 0) {
		FMES(f, "// %s\n", option.toAnsiString().c_str());
	}

	FMES(f, "\
material colored\n\
{\n\
	u_worldViewProjectionMatrix = WORLD_VIEW_PROJECTION_MATRIX\n\
	u_cameraPosition = CAMERA_WORLD_POSITION\n\
	u_inverseTransposeWorldViewMatrix = INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX\n\
}\n\
material textured\n\
{\n\
	u_worldViewProjectionMatrix = WORLD_VIEW_PROJECTION_MATRIX\n\
	u_cameraPosition = CAMERA_WORLD_POSITION\n\
	u_inverseTransposeWorldViewMatrix = INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX\n\
}\n\
");

	for (const auto& material : materials) {
		if (!material.enable) {
			continue;
		}

		bool use_spc = !(material.specular[0] <= 0.0f && material.specular[1] <= 0.0f && material.specular[2] <= 0.0f);

		std::vector<MString> defs;
		if (jointNum > 0) {
			defs.push_back(L"SKINNING");
			defs.push_back(MString::format(L"SKINNING_JOINT_COUNT %d", jointNum));
		}
		if (material.useLighting) {
			defs.push_back(L"DIRECTIONAL_LIGHT_COUNT 1");
			if (use_spc) {
				defs.push_back(L"SPECULAR");
			}
		}

		MString def = MString();
		for (int i = 0; i < defs.size(); ++i) {
			if (i != 0) {
				def += L";";
			}
			def += defs[i];
		}
		if (def.length() > 0) {
			def = L"defines = " + def;
		}

		const auto name = material.convName.toAnsiString();
		FMES(f, "material %s : %s\n{\n",
			name.c_str(), material.useTexture ? "textured" : "colored");

		FMES(f, "\
	u_specularExponent = %.6f\n\
", material.spc_pow);

		if (jointNum > 0) {
			FMES(f, "\
	u_matrixPalette = MATRIX_PALETTE\n\
");
		}

		FMES(f, "\
	renderState\n\
	{\n\
		cullFace = %s\n\
		depthTest = true\n\
	}\n\
", material.isDouble ? "false" : "true");

		if (material.useTexture) { // テクスチャ使用

			FMES(f, "\
	sampler u_diffuseTexture\n\
	{\n\
		path = %s\n\
		wrapS = %s\n\
		wrapT = %s\n\
		mipmap = true\n\
		magFilter = %s\n\
		minFilter = %s\n\
	}\n\
", material.convDiffuseTexture.toAnsiString().c_str(),
	wraps[material.wrapU].toAnsiString().c_str(),
	wraps[material.wrapV].toAnsiString().c_str(),
	magFilters[material.filter].toAnsiString().c_str(),
	minFilters[material.filter].toAnsiString().c_str());

			FMES(f, "\
	technique\n\
	{\n\
		pass\n\
		{\n\
			vertexShader = res/shaders/textured.vert\n\
			fragmentShader = res/shaders/textured.frag\n\
			%s\n\
		}\n\
	}\n\
", def.toAnsiString().c_str());

		}
		else { // テクスチャ不使用

			FMES(f, "\
	u_diffuseColor = %.6f, %.6f, %.6f, %.6f\n\
", material.diffuse[0], material.diffuse[1], material.diffuse[2], material.diffuse[3]);

			FMES(f, "\
	technique\n\
	{\n\
		pass\n\
		{\n\
			vertexShader = res/shaders/colored.vert\n\
			fragmentShader = res/shaders/colored.frag\n\
			%s\n\
		}\n\
	}\n\
", def.toAnsiString().c_str());

		}

		FMES(f, "}\n");
	}

	return 0;
}


// .hsp は res フォルダの一つ上に配置しないといけない
int ExportGPBPlugin::makeHSP(FILE* f, const GPBBounding& bounding, const MString& name,
	const std::vector<MString>& boneNames) {
	float fov = 45.0f * 3.141592f / 180.0f;
	float width = bounding.max[0] - bounding.min[0];
	float height = bounding.max[1] - bounding.min[1];
	float thick = bounding.max[2] - bounding.min[2];
	// 横長の場合のみ正確
	float dist = fmaxf(width, height) * 1.125f * 0.5f / tanf(fov * 0.5f) + thick * 0.5f;

	int boneNum = boneNames.size();
	int boneIndex = (boneNum >= 2) ? 1 : 0;

	FMES(f, "\
#include \"hgimg4.as\"\n\
	ddim vals, 4\n\
	vals(0) = %.6f, %.6f, %.6f, %.6f\n\
",
bounding.center[0], bounding.center[1], bounding.center[2], dist);

	FMES(f, "\
	name = \"%s\"\n\
	verstr = \"%s\"\n\
",
name.toAnsiString().c_str(), IDENVER);

	if (boneNum > 0) {
		FMES(f, "\n\
	sdim bone_names, 260, %d\n\
	gosub *set_bones\n\
	bone_num = length(bone_names)\n\
", boneNum);
	}

	FMES(f, "\
	w = ginfo(12)\n\
	h = ginfo(13)\n\
	setcls 1, 0xf0f0ff\n\
	gpload id, name\n\
	gpnull camera_id\n\
	far = vals(3) * 2.0\n\
	if far < 768.0 : far = 768.0\n\
	gpcamera camera_id, 45, double(w) / double(h), 0.002, far\n\
	gpusecamera camera_id\n\
	setpos camera_id, vals(0), vals(1), vals(2) + vals(3)\n\
	gplookat camera_id, vals(0), vals(1), vals(2)\n\
*main\n\
	getreq time, SYSREQ_TIMER\n\
	val = sin(double(time \\ 10000) / 10000.0 * M_PI * 2.0) / 8.0\n\
	redraw 0\n\
	repeat bone_num\n\
		gpnodeinfo result, id, GPNODEINFO_NODE, bone_names(cnt)\n\
		setangy result, val, val, val\n\
	loop\n\
	gpdraw\n\
	pos 8, 8\n\
	mes verstr\n\
	if id < 0 : mes \"gpload error\"\n\
	redraw 1\n\
	await 1000 / 60\n\
	goto *main\n\
");

	if (boneNum > 0) {
		FMES(f, "\n\
*set_bones\n\
");
		for (int i = 0; i < boneNum; ++i) {
			FMES(f, "\
	bone_names(%d) = \"%s\"\n\
", i, boneNames[i].toAnsiString().c_str());
		}

		FMES(f, "\
	return\n\
");
	}

	return 0;
}


//---------------------------------------------------------------------------
//  GetPluginClass
//    プラグインのベースクラスを返す
//---------------------------------------------------------------------------
MQBasePlugin *GetPluginClass()
{
	static ExportGPBPlugin plugin;
	return &plugin;
}

