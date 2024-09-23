//---------------------------------------------------------------------------
// このコードは ExportPMD.cpp をベースに実装されています。
//    　作成したDLLは"Plugins\Export"フォルダに入れる必要がある。
// https://github.com/gameplay3d/gameplay/blob/master/gameplay/src/Bundle.cpp
//---------------------------------------------------------------------------

#define MY_PRODUCT (0xB2B2501D)
#define MY_ID (0x0ED64D3E)
#define MY_PLUGINNAME "Export GPB Copyright(C) 2024, hta39393"
#define MY_FILETYPE "HSP GPB simple(*.gpb)"
#define MY_EXT "gpb"

#define IDENVER "0.1.1"

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
#include "tinyxml2.h" // Download TinyXML2 from https://github.com/leethomason/tinyxml2

#define GL_TRIANGLE (0x0004)
#define GL_LINES (0x0001)
#define GL_UNSIGNED_SHORT (0x1403)

enum {
	STRUCT_SIMPLE = 0,
	STRUCT_OBJECT = 1,
	STRUCT_SKIN = 2,
};

#ifdef _WIN32
HINSTANCE s_hInstance;
wchar_t s_DllPath[MAX_PATH];
#endif

#define EPS	0.00001

#define LARGEABS (800000000.0f)

#define FMES fprintf_s

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

	bool useTexture;
	// .material に書く
	MString orgDiffuseTexture;
	MString convDiffuseTexture;

	BOOL isDouble;

	// RGBA
	float diffuse[4];

	GPBMaterial() {
		enable = true;
		orgIndex = -1;
		isDouble = FALSE;
		useTexture = false;
		diffuse[0] = 1.0f;
		diffuse[1] = 1.0f;
		diffuse[2] = 1.0f;
		diffuse[3] = 1.0f;
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
		MString group;
	};
	struct BoneGroupSetting {
		MString jp;
		MString en;
	};

	BoneNameSetting m_RootBoneName;
	std::vector<BoneNameSetting> m_BoneNameSetting;
	std::vector<BoneGroupSetting> m_BoneGroupSetting;
	bool LoadBoneSettingFile();

	int makeMaterial(FILE* fhMaterial, const std::vector<GPBMaterial>& materials);
	// 
	int makeHSP(FILE* f, const GPBBounding& bounding);
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
	//MQComboBox *combo_bone;
	//MQComboBox *combo_ikend;
	//MQComboBox *combo_facial;
	//MQEdit *edit_modelname;
	//MQMemo *memo_comment;

	GPBOptionDialog(int id, int parent_frame_id, ExportGPBPlugin *plugin, MLanguage& language);

	BOOL ComboBoneChanged(MQWidgetBase *sender, MQDocument doc);
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

#if 0
	hframe = CreateHorizontalFrame(group);
	CreateLabel(hframe, language.Search("Bone"));
	this->combo_bone = CreateComboBox(hframe);
	combo_bone->AddItem(language.Search("Disable"));
	combo_bone->AddItem(language.Search("Enable"));
	combo_bone->SetHintSizeRateX(8);
	combo_bone->SetFillBeforeRate(1);

	hframe = CreateHorizontalFrame(group);
	CreateLabel(hframe, language.Search("IKEnd"));
	this->combo_ikend = CreateComboBox(hframe);
	combo_ikend->AddItem(language.Search("Disable"));
	combo_ikend->AddItem(language.Search("Enable"));
	combo_ikend->SetHintSizeRateX(8);
	combo_ikend->SetFillBeforeRate(1);

	// モーフの出力可否のGUI作成
	hframe = CreateHorizontalFrame(group);
	CreateLabel(hframe, language.Search("Facial"));
	this->combo_facial = CreateComboBox(hframe);
	combo_facial->AddItem(language.Search("Disable"), STRUCT_SIMPLE);
	combo_facial->AddItem(language.Search("Enable"), STRUCT_OBJECT);
	combo_facial->AddItem(language.Search("Enable"), STRUCT_SKIN);
	combo_facial->SetHintSizeRateX(8);
	combo_facial->SetFillBeforeRate(1);
#endif

#if 0
	hframe = CreateHorizontalFrame(group);
	CreateLabel(hframe, language.Search("ModelName"));
	this->edit_modelname = CreateEdit(hframe); 
	edit_modelname->SetMaxAnsiLength(20);
	edit_modelname->SetHorzLayout(LAYOUT_FILL);

	CreateLabel(group, language.Search("Comment"));
	memo_comment = CreateMemo(group);
	memo_comment->SetMaxLength(256);
#endif
}

BOOL GPBOptionDialog::ComboBoneChanged(MQWidgetBase *sender, MQDocument doc)
{
	//this->combo_ikend->SetEnabled(combo_bone->GetCurrentIndex() == 1);
	return FALSE;
}


struct CreateDialogOptionParam
{
	ExportGPBPlugin *plugin;
	GPBOptionDialog *dialog;
	MLanguage *lang;

	bool visible_only;
	bool bone_exists;
	bool facial_exists;
	int struct_mode;
	bool output_bone;
	bool output_hsp;
	//MAnsiString modelname;
	MAnsiString comment;

};

/// <summary>
/// オプションダイアログを生成する
/// </summary>
/// <param name="init">true だと初期値で初期化する</param>
/// <param name="param"></param>
/// <param name="ptr"></param>
static void CreateDialogOption(bool init, MQFileDialogCallbackParam *param, void *ptr)
{
	CreateDialogOptionParam *option = (CreateDialogOptionParam*)ptr;

	if(init)
	{
		GPBOptionDialog *dialog = new GPBOptionDialog(param->dialog_id, param->parent_frame_id, option->plugin, *option->lang);
		option->dialog = dialog;

		dialog->check_visible->SetChecked(option->visible_only);

#if 0
		dialog->combo_bone->SetEnabled(option->bone_exists);
		dialog->combo_bone->SetCurrentIndex(option->output_bone ? 1 : 0);
		dialog->combo_ikend->SetEnabled(option->bone_exists && option->output_bone);
		dialog->combo_ikend->SetCurrentIndex(option->output_ik_end ? 1 : 0);

		dialog->combo_facial->SetCurrentIndex(0);

		dialog->edit_modelname->SetText(MString::fromAnsiString(option->modelname).c_str());
#endif
	}
	else
	{
		option->visible_only = option->dialog->check_visible->GetChecked();
#if 0
		option->output_bone = option->dialog->combo_bone->GetCurrentIndex() == 1;
		option->output_ik_end = option->dialog->combo_ikend->GetCurrentIndex() == 1;
		option->output_facial = option->dialog->combo_facial->GetCurrentIndex() == 1;
		//option->modelname = getMultiBytesSubstring(MString(option->dialog->edit_modelname->GetText()).toAnsiString(), 20);
		option->comment = getMultiBytesSubstring(MString(option->dialog->memo_comment->GetText()).toAnsiString(), 256);
#endif
		delete option->dialog;
	}
}


// @see Node.h#L58
enum NodeType {
	NODE_NODE = 1,
	NODE_JOINT = 2,
};

// @see Transform.h#L89
enum AnimationAttr {
	ANIMATE_ROTATE_TRANSLATE = 16,
	ANIMATE_SCALE_ROTATE_TRANSLATE = 17,
};

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
	MAnsiString cameraName;
	float ambient[3];
	GPBScene() {
		cameraName = MAnsiString("");
		ambient[0] = 0.1f;
		ambient[1] = 0.2f;
		ambient[2] = 0.3f;
	}
};

#pragma pack(pop)

struct PMDBoneParam {
	UINT id;
	MQMatrix mtx;
	MQMatrix base_mtx;
	UINT parent;
	int child_num;
	MQPoint org_pos;
	MQPoint def_pos;
	//MQPoint scale;
	MString name;
	int ikchain;
	UINT ikparent;
	bool ikparent_isik;
	bool dummy;
	bool twist;
	std::vector<UINT> children;

	int pmd_index;

	MString name_jp;
	MString name_en;
	int root_group;
	int group;

	bool movable;
	UINT group_id;
	UINT tip_id;
	MString ik_name;
	MString ik_tip_name;

	MString ik_name_jp;
	MString ik_tip_name_jp;
	MString ik_name_en;
	MString ik_tip_name_en;
	PMDBoneParam(){
		id = 0;
		parent = 0;
		child_num = 0;
		ikchain = 0;
		ikparent = 0;
		ikparent_isik = false;
		dummy = false;
		twist = false;
		pmd_index = -1;
		root_group = -1;
		group = -1;

		movable = false;
		group_id = 0;
		tip_id = 0;
	}
};

/// <summary>
/// 参照テーブル構造体
/// </summary>
struct GPBRef {
	MAnsiString name;
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
		name = "";
		type = 0;
		offset = 0;
		writeOffset = 0;
	}
};

struct GPBNode {
	BYTE type; // node, joint
	MQMatrix mtx;
	MQMatrix base_mtx;
	MQPoint org_pos;
	MQPoint def_pos;
	MAnsiString name;
	std::vector<GPBNode> children;

	GPBNode() {
		name = MAnsiString("");
	}
};


//Facial
enum MorphType
{
	MORPH_BASE = 0,
	MORPH_BROW,
	MORPH_EYE,
	MORPH_LIP,
	MORPH_OTHER,
};

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

struct PMDMorphInputParam
{
	MQObject	base;
	std::vector<std::pair<MQObject, MorphType> >	target;
};

static bool containsTargetObject(std::vector<PMDMorphInputParam> &list, MQObject obj)
{
	assert(obj != nullptr);

	auto end = list.end();
	for(auto ite = list.begin(); ite != end; ++ite)
	{
		for(auto tIte = ite->target.begin(); tIte != ite->target.end(); ++tIte)
			if(tIte->first == obj)
				return true;
	}

	return false;
}

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
	//MString path = MFileUtil::combinePath(dir, L"ExportGPB.resource.xml");
	MString path = MFileUtil::combinePath(dir, L"exportgpb.resource.xml");
	MLanguage language;
	language.Load(lang, path.c_str());

	// Load bone setting
	LoadBoneSettingFile();

	MQBoneManager bone_manager(this, doc);
	std::vector<GPBMaterial> materials;

	int indexMesh = 0;
	int indexScene = 1;
	int indexAnimations = 2;
	int indexNode = 3;
	int indexJoint = 4;
	std::vector<GPBRef> refTable;
	for (int i = 0; i < 5; ++i) {
		GPBRef ref;
		ref.offset = 0x363534; // for check
		switch (i) {
		case 0:
			ref.type = REF_MESH;
			ref.name = MAnsiString("Mesh");
			break;
		case 3:
			ref.type = REF_NODE;
			ref.name = MAnsiString("Bone0");
			break;
		case 2:
			ref.type = REF_ANIMATIONS;
			ref.name = MAnsiString("__Animations__");
			break;
		case 1:
			ref.type = REF_SCENE;
			ref.name = MAnsiString("__SCENE__");
			break;
		case 4:
			ref.type = REF_NODE;
			ref.name = MAnsiString("Joint0");
			break;
		}
		refTable.push_back(ref);
	}

	GPBScene scene;
	std::vector<GPBKey> keyvals;

	for (int i = 0; i < 2; ++i) {
		GPBKey keyval;
		keyval.msec = 60 * 1000 * i;
		switch (i) {
		case 0:
			keyval.p[0] = 0.0f;
			keyval.p[1] = -1.0f;
			keyval.p[2] = 1.0f;
			break;
		case 1:
			keyval.p[0] = 1.0f;
			keyval.p[1] = 1.0f;
			keyval.p[2] = 0.0f;
			break;
		}
		keyvals.push_back(keyval);
	}

	//GroupList
	std::map<UINT,std::wstring> groups;
	std::vector<UINT> group_id;
	int group_num = bone_manager.EnumGroups(group_id);
	if(group_num > 0){
		for(int i = 0;i < group_num;i++){
			std::wstring name;
			bone_manager.GetGroupName(group_id[i], name);
			if(!name.empty() && name.back() == 0x0A)
				name.pop_back(); // remove return
			groups.insert(std::make_pair(group_id[i], name));
		}
	}
	// Query a number of bones
	int bone_num = bone_manager.GetBoneNum();
	int bone_object_num = bone_manager.GetSkinObjectNum();
	// Enum bones
	std::vector<UINT> bone_id;
	std::vector<PMDBoneParam> bone_param;
	if(bone_num > 0){
		bone_id.resize(bone_num);
		bone_manager.EnumBoneID(bone_id);

		bone_param.resize(bone_num);
		for(int i=0; i<bone_num; i++){
			bone_param[i].id = bone_id[i];

			std::wstring name;
			bone_manager.GetParent(bone_id[i], bone_param[i].parent);
			bone_manager.GetChildNum(bone_id[i], bone_param[i].child_num);
			bone_manager.GetBasePos(bone_id[i], bone_param[i].org_pos);
			bone_manager.GetDeformPos(bone_id[i], bone_param[i].def_pos);
			bone_manager.GetDeformMatrix(bone_id[i], bone_param[i].mtx);
			bone_manager.GetBaseMatrix(bone_id[i], bone_param[i].base_mtx);
			//bone_manager.GetDeformScale(bone_id[i], bone_param[i].scale);
			bone_manager.GetName(bone_id[i], name);
			bone_manager.GetIKChain(bone_id[i], bone_param[i].ikchain);
			bone_manager.GetDummy(bone_id[i], bone_param[i].dummy);

			bone_param[i].name = MString(name);
			{
				MQAngle angle_min, angle_max;
				bone_manager.GetAngleMin(bone_id[i], angle_min);
				bone_manager.GetAngleMax(bone_id[i], angle_max);
				bone_param[i].twist = (angle_max.bank == 0 && angle_min.bank == 0 && angle_max.pitch == 0 && angle_min.pitch == 0);
			}
			{
				MQBoneManager::LINK_PARAM param;
				bone_manager.GetLink(bone_id[i], param);
				//bone_param[i].link_id = param.link_bone_id;
				//bone_param[i].link_rate = (int)param.rotate;
			}
			bone_manager.GetMovable(bone_id[i], bone_param[i].movable);
			bone_manager.GetGroupID(bone_id[i], bone_param[i].group_id);
			{
				std::vector<UINT> children;
				bone_manager.GetChildren(bone_id[i], children);
				if(!children.empty())
					bone_param[i].tip_id = children.front();
			}
			if(bone_param[i].ikchain > 0){
				std::wstring name;
				std::wstring tip_name;
				bone_manager.GetIKName(bone_id[i], name, tip_name);
				bone_param[i].ik_name = name;
				bone_param[i].ik_tip_name = tip_name;
				
				bone_manager.GetIKParent(bone_id[i], bone_param[i].ikparent, bone_param[i].ikparent_isik);
			}
		}
	}


	// Show a dialog for converting axes
	// 座標軸変換用ダイアログの表示
	float scaling = 1;
	CreateDialogOptionParam option;
	option.plugin = this;
	option.lang = &language;
	option.visible_only = false;
	option.bone_exists = (bone_num > 0);
	option.output_bone = true;
	option.output_hsp = false;
	option.struct_mode = STRUCT_SIMPLE;
	// ファイル名だけ取り出して20文字に制限
	//option.modelname = getMultiBytesSubstring(MFileUtil::extractFileNameOnly(filename).toAnsiString(), 20);
	option.comment = MAnsiString();
	// Load a setting.
	MQSetting *setting = OpenSetting();
	if(setting != NULL){
		setting->Load("VisibleOnly", option.visible_only, option.visible_only);
		setting->Load("OutputHsp", option.output_hsp, option.output_hsp);
	}
	MQFileDialogInfo dlginfo;
	memset(&dlginfo, 0, sizeof(dlginfo));
	dlginfo.dwSize = sizeof(dlginfo);
	dlginfo.scale = scaling;
	dlginfo.hidden_flag = MQFileDialogInfo::HIDDEN_AXIS | MQFileDialogInfo::HIDDEN_INVERT_FACE;
	//dlginfo.hidden_flag = MQFileDialogInfo::HIDDEN_AXIS | MQFileDialogInfo::HIDDEN_INVERT_FACE;
	dlginfo.axis_x = MQFILE_TYPE_RIGHT;
	dlginfo.axis_y = MQFILE_TYPE_UP;
	// PMD では FRONT 指定してあった
	dlginfo.axis_z = MQFILE_TYPE_BACK;
	dlginfo.softname = "";
	dlginfo.dialog_callback = CreateDialogOption;
	dlginfo.dialog_callback_ptr = &option;
	MQ_ShowFileDialog("GPB Export", &dlginfo);

	// Save a setting.
	scaling = dlginfo.scale;
	if(setting != NULL){
		setting->Save("VisibleOnly", option.visible_only);
		//setting->Save("Bone", option.output_bone);
		//setting->Save("IKEnd", option.output_ik_end);
		//setting->Save("Facial", option.output_facial);
		CloseSetting(setting);
	}

	//// 処理後半

	// サブパスは取れる
	MString contentDir = MFileUtil::extractDirectory(filename);
	MString materialPath = MFileUtil::changeExtension(filename, L".material");
	MString csvPath = MFileUtil::changeExtension(filename, L".csv");

	MString onlyName = MFileUtil::extractFileNameOnly(filename);
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
		for(int evi=0; evi<vert_num; evi++){
			orgvert_vert[oi][evi] = total_vert_num;

			vert_orgobj.push_back(oi); // 元のオブジェクトIDを追加する
			vert_expvert.push_back(evi); // 元のオブジェクト内での頂点インデックスを追加する

			MQPoint nrm = eobj->GetVertexNormal(evi);
			MQCoordinate uv = eobj->GetVertexCoordinate(evi);

			vert_normal.push_back(nrm);
			vert_coord.push_back(uv);
			total_vert_num++;

			if(total_vert_num > 65535){
				MQWindow mainwin = MQWindow::GetMainWindow();
				MQDialog::MessageWarningBox(mainwin, language.Search("TooManyVertices"), GetResourceString("Error"));
				for(size_t i=0; i<expobjs.size(); i++){
					delete expobjs[i];
				}
				return FALSE;
			}
		}
	}

	std::map<UINT, int> bone_id_index;
	// Initialize bones.
	if(bone_num > 0)
	{
		for(int i=0; i<bone_num; i++){
			bone_id_index[bone_param[i].id] = i;
		}

		// Check the parent
		for(int i=0; i<bone_num; i++){
			if(bone_param[i].parent != 0){
				if(bone_id_index.end() == bone_id_index.find(bone_param[i].parent)){
					assert(0);
					bone_param[i].parent = 0;
				}
			}
		}

		// Sort by hierarchy
		{
			std::list<PMDBoneParam> bone_param_temp(bone_param.begin(), bone_param.end());
			bone_param.clear();
			bone_id_index.clear();
			while(!bone_param_temp.empty()){
				bool done = false;
				for(auto it = bone_param_temp.begin(); it != bone_param_temp.end(); ){
					if((*it).parent != 0){
						if(bone_id_index.end() != bone_id_index.find((*it).parent)){
							if(bone_param[bone_id_index[(*it).parent]].tip_id == 0 || bone_id_index[(*it).parent] != bone_param.size()-1){
								bone_id_index[(*it).id] = (int)bone_param.size();
								bone_param.push_back(*it);
								it = bone_param_temp.erase(it);
								done = true;
							}else if(bone_param[bone_id_index[(*it).parent]].tip_id == (*it).id){
								bone_id_index[(*it).id] = (int)bone_param.size();
								bone_param.push_back(*it);
								it = bone_param_temp.erase(it);
								done = true;
							}else{
								++it; // try next
							}
						}else{
							++it; // try next
						}
					}else{
						bone_id_index[(*it).id] = (int)bone_param.size();
						bone_param.push_back(*it);
						it = bone_param_temp.erase(it);
						done = true;
					}
				}

				assert(done);
				if(!done){
					for(auto it = bone_param_temp.begin(); it != bone_param_temp.end(); ++it){
						bone_id_index[(*it).id] = (int)bone_param.size();
						(*it).parent = 0;
						bone_param.push_back(*it);
					}
					break;
				}
			}
		}

		// Enum children
		for(int i=0; i<bone_num; i++){
			if(bone_param[i].parent != 0){
				if(bone_id_index.end() != bone_id_index.find(bone_param[i].parent)){
					bone_param[bone_id_index[bone_param[i].parent]].children.push_back(i);
				}else{
					assert(0);
					bone_param[i].parent = 0;
				}
			}
			if(bone_param[i].twist){
				bone_param[i].twist = false;
				bone_param[bone_id_index[bone_param[i].parent]].twist = true;
			}
		}
	}
	int pmdbone_num = 0;
	std::vector<int> ik_chain_end_list;

	for(int i=0; i<bone_num; i++){
		// Determine PMD bone index.
		bone_param[i].pmd_index = pmdbone_num++;
	}
	for(int i=0; i<bone_num; i++){
		if(bone_param[i].tip_id==0)continue;
		UINT tip_parent_id = bone_param[bone_id_index[bone_param[i].tip_id]].parent;
		assert(bone_param[i].id == tip_parent_id);
	}
	// Construct IK chain

	for(int i=0; i<bone_num; i++){
		// name
		bone_param[i].name_jp = bone_param[i].name;
		bone_param[i].name_en = bone_param[i].name;
		for(auto it = m_BoneNameSetting.begin(); it != m_BoneNameSetting.end(); ++it){
			if((*it).jp == bone_param[i].name || (*it).en == bone_param[i].name){
				bone_param[i].name_jp = (*it).jp;
				bone_param[i].name_en = (*it).en;
				break;
			}
		}

		// IK tip name
		bone_param[i].ik_tip_name_jp = bone_param[i].ik_tip_name;
		bone_param[i].ik_tip_name_en = bone_param[i].ik_tip_name;
		for(auto it = m_BoneNameSetting.begin(); it != m_BoneNameSetting.end(); ++it){
			if((*it).jp == bone_param[i].ik_tip_name || (*it).en == bone_param[i].ik_tip_name){
				bone_param[i].ik_tip_name_jp = (*it).jp;
				bone_param[i].ik_tip_name_en = (*it).en;
				break;
			}
		}
	}
	std::map<int,std::pair<MString,MString> > group_names;
	for(auto it = groups.begin();it != groups.end();it++){
		group_names[it->first] = std::make_pair(it->second, it->second);
		for(auto nit = m_BoneGroupSetting.begin(); nit != m_BoneGroupSetting.end(); ++nit){
			if((*nit).jp == it->second || (*nit).en == it->second){
				group_names[it->first] = std::make_pair((*nit).jp, (*nit).en);
				break;
			}
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
				}

				/*
				int shader = mat->GetShader();
				if (shader == MQMATERIAL_SHADER_HLSL) {
					MAnsiString shader_name = mat->GetShaderName();
					if (shader_name == "pmd") {
						//toon = mat->GetShaderParameterIntValue("Toon", 0);
						//edge = mat->GetShaderParameterBoolValue("Edge", 0);
					}
				}
				*/
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

		float specular_color[3]; // sr, sg, sb // 光沢色
		specular_color[0] = sqrtf(spc_col.r);
		specular_color[1] = sqrtf(spc_col.g);
		specular_color[2] = sqrtf(spc_col.b);

		float ambient_color[3]; // mr, mg, mb // 環境色(ambient)
		ambient_color[0] = amb_col.r;
		ambient_color[1] = amb_col.g;
		ambient_color[2] = amb_col.b;

		//DWORD face_vert_count = material_used[i] * 3;

		//MAnsiString texture_str = getMultiBytesSubstring(texture, 20);
		//char texture_file_name[20];
		//memcpy(texture_file_name, texture_str.c_str(), texture_str.length());
		//fwrite(texture_file_name, 20, 1, fh);

		// TODO: 
		material.orgDiffuseTexture = texture;

		material.convName = material.orgName;
		if (material.useTexture) {
			material.convDiffuseTexture = MString(L"res/") + material.orgDiffuseTexture;
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


	std::vector<int> faceVertexIndices;

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
					// NOTE: 面頂点
					for (int j = 0; j < n - 2; j++) {
						WORD tvi[3];
						tvi[0] = (WORD)orgvert_vert[i][vi[tri[j * 3]]];
						tvi[1] = (WORD)orgvert_vert[i][vi[tri[j * 3 + 1]]];
						tvi[2] = (WORD)orgvert_vert[i][vi[tri[j * 3 + 2]]];
						//fwrite(tvi, 2, 3, fh);
						faceVertexIndices.push_back(tvi[0]);
						faceVertexIndices.push_back(tvi[1]);
						faceVertexIndices.push_back(tvi[2]);

						// 元の順
						//materials[mi].faceIndices.push_back(tvi[0]);
						//materials[mi].faceIndices.push_back(tvi[1]);
						//materials[mi].faceIndices.push_back(tvi[2]);

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
		}
	}


	//// Open a file.
	FILE *fh;
	errno_t err = _wfopen_s(&fh, filename, L"wb");
	//errno_t err = fopen_s(&fh, filename, "w");
	if(err != 0){
		for(size_t i=0; i<expobjs.size(); i++){
			delete expobjs[i];
		}
		return FALSE;
	}

	FILE* fhMaterial = nullptr;
	err = _wfopen_s(&fhMaterial, materialPath.c_str(), L"w");
	if (err != 0) {
		fhMaterial = nullptr;
	}
	// debug
	//FMES(fhMaterial, hspPath.toAnsiString().c_str());

	FILE* fhHSP = nullptr;
	if (option.output_hsp) {
		err = _wfopen_s(&fhHSP, hspPath.c_str(), L"w");
		if (err != 0) {
			fhHSP = nullptr;
		}
	}

#if 0
	FILE* fhInAnim = nullptr;
	err = _wfopen_s(&fhInAnim, csvPath.c_str(), L"r");
	if (err != 0) {
		fhInAnim = nullptr;
	}
#endif

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
	fwrite(&refNum, 4, 1, fh);
	for (auto& ref : refTable) // 書き込み有り
	{
		DWORD type = ref.type;
		DWORD offset = ref.offset;
		// バイト 名前
		MAnsiString name = ref.name;
		DWORD byteNum = name.length(); // size_t は大きすぎる
		fwrite(&byteNum, sizeof(DWORD), 1, fh);
		fwrite(name.c_str(), sizeof(char), byteNum, fh);
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
		DWORD attrNum = 5;
		fwrite(&attrNum, sizeof(DWORD), 1, fh);
		DWORD attr[10] = {
			ATTR_POSITION, 3,
			ATTR_NORMAL, 3,
			ATTR_TEXCOORD0, 2,
			ATTR_BLENDWEIGHTS, 4,
			ATTR_BLENDINDICES, 4,
		};
		fwrite(&attr, sizeof(DWORD), 10, fh);

		DWORD vertexByteCount = total_vert_num * 16 * sizeof(float);
		fwrite(&vertexByteCount, sizeof(DWORD), 1, fh);

		for (int j = 0; j < total_vert_num; ++j) {
			float pos[3];
			float nrm[3];
			float uv[2];
			// どこから調達??
			float weight[4] = { 1.0, 0.0, 0.0, 0.0 };
			// どこから調達??
			float indices[4] = { 0, 0, 0, 0 };

			// NOTE: meshNum でどうして頂点ループするの?? コード削除しすぎた??

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


			UINT vert_bone_id[16];
			float weights[16];
			int weight_num = 0;
			if (bone_num > 0) { // ボーンが1個以上存在する場合
				UINT vert_id = obj->GetVertexUniqueID(eobj->GetOriginalVertex(vert_expvert[j]));
				int max_num = 16;
				weight_num = bone_manager.GetVertexWeightArray(obj, vert_id, max_num, vert_bone_id, weights);
			}

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
				bone_index[0] = bone_param[bi1].pmd_index;
				bone_index[1] = bone_param[bi2].pmd_index;
				if (bone_index[0] != bone_index[1]) {
					bone_weight = max_weight1 / total_weights;
				}
				else {
					bone_weight = 1.0f;
				}
			}
			else if (weight_num == 1) { // 1個の場合
				int bi = bone_id_index[vert_bone_id[0]];
				bone_index[0] = bone_param[bi].pmd_index;
				bone_index[1] = bone_index[0];
				bone_weight = 1.0f;
			}
			else { // 0個の場合
				bone_index[0] = 0;
				bone_index[1] = 0;
				bone_weight = 1.0f;
			}
			indices[0] = bone_index[0];
			//weight[0] = bone_weight;

			for (int index = 0; index < 3; ++index) {
				bounding.max[index] = fmaxf(bounding.max[index], pos[index]);
				bounding.min[index] = fminf(bounding.min[index], pos[index]);
			}

			fwrite(&pos, sizeof(float), 3, fh);
			fwrite(&nrm, sizeof(float), 3, fh);
			fwrite(&uv, sizeof(float), 2, fh);

			fwrite(&weight, sizeof(float), 4, fh);
			fwrite(&indices, sizeof(float), 4, fh);
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
			DWORD format = GL_UNSIGNED_SHORT; // u16 or u32
			DWORD byteNum = material.faceIndices.size() * sizeof(unsigned short);
			fwrite(&type, sizeof(DWORD), 1, fh);
			fwrite(&format, sizeof(DWORD), 1, fh);
			fwrite(&byteNum, sizeof(DWORD), 1, fh);
			// 面頂点
			for (const auto& index : material.faceIndices) {
				unsigned short index16 = static_cast<unsigned short>(index);
				fwrite(&index16, sizeof(unsigned short), 1, fh);
			}
		}

		for (int j = 0; j < 3; ++j) {
			wholeBounding.max[j] = fmaxf(wholeBounding.max[j], bounding.max[j]);
			wholeBounding.min[j] = fminf(wholeBounding.min[j], bounding.min[j]);
		}
	}

	calcRadius(wholeBounding);


#if 0
	std::vector<int>bone_group;
	// Bone list
	if(bone_num == 0){
		//WORD dw_bone_num = 1;
		//fwrite(&dw_bone_num, 2, 1, fh);

		//static const char bone_name[20] = "default";
		//fwrite(bone_name, 20, 1, fh);
	}else{
		WORD dw_bone_num = (WORD)pmdbone_num;
		//fwrite(&dw_bone_num, 2, 1, fh);
		int pmdbone_index = 0;
		for(int i=0; i<bone_num; i++){
			if(bone_param[i].pmd_index >= 0 && bone_param[i].pmd_index >= pmdbone_index){
				assert(bone_param[i].pmd_index == pmdbone_index);
				MAnsiString subname = getMultiBytesSubstring(bone_param[i].name_jp.toAnsiString(), 20);
				char bone_name[20];
				memset(bone_name, 0, 20);
				memcpy(bone_name, subname.c_str(), subname.length());
				//fwrite(bone_name, 20, 1, fh);

				WORD parent_bone_index = 0xFFFF;
				if(bone_param[i].parent != 0){
					auto parent_it = bone_id_index.find(bone_param[i].parent);
					if(parent_it != bone_id_index.end()){
						parent_bone_index = (WORD)bone_param[(*parent_it).second].pmd_index;
					}
				}
				//fwrite(&parent_bone_index, 2, 1, fh);

				// tail_pos_bone_index 
				WORD tail_pos_bone_index = 0;
				if(!bone_param[i].children.empty()){
					auto& child_param = bone_param[bone_param[i].children.front()];
					tail_pos_bone_index = (WORD)child_param.pmd_index; // tail位置のボーン番号(チェーン末端の場合は0xFFFF 0 →補足2) // 親：子は1：多なので、主に位置決め用
					// 捩れボーンの場合、捩れ先ボーンを優先
					if(bone_param[i].twist){
						for(auto it = bone_param[i].children.begin(); it != bone_param[i].children.end(); ++it){
							if(bone_param[*it].dummy && bone_param[*it].child_num == 0){
								tail_pos_bone_index = (WORD)bone_param[*it].pmd_index;
								break;
							}
						}
					}
					// 子が捩れボーンの場合、孫のボーンまで接続
					if(child_param.twist && child_param.child_num != 0){
						auto& tail_param = bone_param[child_param.children.front()];
						tail_pos_bone_index = (WORD)tail_param.pmd_index;
						for(auto it = child_param.children.begin(); it != child_param.children.end(); ++it){
							if(!bone_param[*it].dummy && bone_param[*it].child_num != 0){
								tail_pos_bone_index = (WORD)bone_param[*it].pmd_index;
								break;
							}
						}
					}
				}
				//fwrite(&tail_pos_bone_index, 2, 1, fh);

				// bone_type
				//   0:回転 1:回転と移動 2:IK 3:不明 4:IK影響下 5:回転影響下 6:IK接続先 7:非表示 8:捩れ 9:回転連動
				BYTE bone_type = (bone_param[i].parent == 0) ? 1 : 0;
				if(bone_param[i].tip_id == 0 && bone_param[i].child_num != 0) bone_type = 7;
				else if(bone_param[i].child_num == 0) bone_type = 7;
				if(bone_type <= 1){
					bone_type = bone_param[i].movable ? 1 : 0;
				}
				//fwrite(&bone_type, 1, 1, fh);

				float bone_head_pos[3];
				bone_head_pos[0] = bone_param[i].org_pos.x * scaling;
				bone_head_pos[1] = bone_param[i].org_pos.y * scaling;
				bone_head_pos[2] = bone_param[i].org_pos.z * scaling;
				//fwrite(&bone_head_pos, 4, 3, fh);

				pmdbone_index++;
			}
		}
	}

				//MAnsiString subname = getMultiBytesSubstring(bone_param[i].name_en.toAnsiString(), 20);
				//memset(bone_name, 0, 20);
				//memcpy(bone_name, subname.c_str(), subname.length());

		//MAnsiString n = MAnsiString::format("toon%02d.bmp", i+1);
#endif

	//// 材質の書き出し
	if (fhMaterial) {
		this->makeMaterial(fhMaterial, materials);
	}
	if (fhHSP) {
		this->makeHSP(fhHSP, wholeBounding);
	}

	DWORD elementNum = 2;
	fwrite(&elementNum, sizeof(DWORD), 1, fh);

	//// シーンの書き出し
	refTable[indexScene].offset = ftell(fh);
	DWORD nodeNum = 2;
	fwrite(&nodeNum, sizeof(DWORD), 1, fh);
	for (int i = 0; i < nodeNum; ++i) {
		auto offset = ftell(fh);
		DWORD nodeType = NODE_NODE;
		switch (i) {
		case 0:
			nodeType = NODE_JOINT;
			refTable[indexJoint].offset = offset;
			break;
		case 1:
			nodeType = NODE_NODE;
			refTable[indexNode].offset = offset;
			break;
		}

		fwrite(&nodeType, sizeof(DWORD), 1, fh);
		float transform[16] = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		};
		fwrite(&transform, sizeof(float), 16, fh);

		MAnsiString parent("");
		DWORD parentByteNum = parent.length();
		fwrite(&parentByteNum, sizeof(DWORD), 1, fh);
		fwrite(parent.c_str(), sizeof(char), parentByteNum, fh);

		DWORD childNum = 0;
		fwrite(&childNum, sizeof(DWORD), 1, fh);

		BYTE camlight[2] = { 0, 0 }; // camera, light
		fwrite(&camlight, sizeof(BYTE), 2, fh);

		// model
		DWORD zero = 0;
		switch (i) {
		case 0:
			fwrite(&zero, sizeof(DWORD), 1, fh);
			break;
		case 1:
			{
				MAnsiString meshName("#Mesh");
				DWORD meshNameByteNum = meshName.length();
				fwrite(&meshNameByteNum, sizeof(DWORD), 1, fh);
				fwrite(meshName.c_str(), sizeof(char), meshNameByteNum, fh);

				BYTE hasSkin = 0;
				fwrite(&hasSkin, sizeof(BYTE), 1, fh);
				if (hasSkin) {
					// Not implemented
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
			break;
		}

	}

	DWORD cameraNameLength = scene.cameraName.length();
	fwrite(&cameraNameLength, sizeof(DWORD), 1, fh);
	fwrite(scene.cameraName.c_str(), sizeof(char), cameraNameLength, fh);

	fwrite(&scene.ambient, sizeof(float), 3, fh);

	//// アニメーションの書き出し
	refTable[indexAnimations].offset = ftell(fh);
	DWORD animationNum = 1;
	fwrite(&animationNum, sizeof(DWORD), 1, fh);

	MAnsiString animationName = "animations"; // この名前であることが必要
	DWORD animationNameByteNum = animationName.length();
	fwrite(&animationNameByteNum, sizeof(DWORD), 1, fh);
	fwrite(animationName.c_str(), sizeof(char), animationNameByteNum, fh);

	DWORD channelNum = 1;
	fwrite(&channelNum, sizeof(DWORD), 1, fh);
	for (int i = 0; i < channelNum; ++i) {
		MAnsiString targetName = "Joint0";
		DWORD targetNameLength = targetName.length();
		fwrite(&targetNameLength, sizeof(DWORD), 1, fh);
		fwrite(targetName.c_str(), sizeof(char), targetNameLength, fh);
		DWORD valType = ANIMATE_ROTATE_TRANSLATE; // tamane2 は 16 回転と移動
		fwrite(&valType, sizeof(DWORD), 1, fh);

		// キー配列
		DWORD keyNum = keyvals.size();
		fwrite(&keyNum, sizeof(DWORD), 1, fh);
		for (const auto& keyval : keyvals) {
			fwrite(&keyval.msec, sizeof(DWORD), 1, fh);
		}
		// 値配列 個数
		DWORD valNum = keyNum * 7;
		fwrite(&valNum, sizeof(DWORD), 1, fh);
		for (const auto& keyval : keyvals) {
			fwrite(&keyval.q, sizeof(float), 4, fh);
			fwrite(&keyval.p, sizeof(float), 3, fh);
		}

		DWORD tin = 0;
		fwrite(&tin, sizeof(DWORD), 1, fh);
		DWORD tout = 0;
		fwrite(&tout, sizeof(DWORD), 1, fh);
		DWORD inum = 1;
		fwrite(&inum, sizeof(DWORD), 1, fh);
		for (int j = 0; j < inum; ++j) {
			// tamane2 では type 1 BSPLINE が格納されているが
			// Curve::LINEAR しか対応してないらしい
			DWORD itype = 1;
			fwrite(&itype, sizeof(DWORD), 1, fh);
		}
	}


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

	for(size_t i=0; i<expobjs.size(); i++){
		delete expobjs[i];
	}

	if (fhMaterial) {
		err = fclose(fhMaterial);
	}
	if (fhHSP) {
		err = fclose(fhHSP);
	}

#if 0
	if (fhInAnim) {
		err = fclose(fhInAnim);
	}
#endif

	if(fclose(fh) != 0){
		return FALSE;
	}
	return TRUE;
}

bool ExportGPBPlugin::LoadBoneSettingFile()
{
#ifdef _WIN32
	MString dir = MFileUtil::extractDirectory(s_DllPath);
#else
	MString dir = GetResourceDir();
#endif
	MString filename = MFileUtil::combinePath(dir, L"ExportPMDBoneSetting.xml");

	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError err = doc.LoadFile(filename.toAnsiString().c_str());
	if(err != tinyxml2::XML_SUCCESS){
#ifdef _WIN32
		MString buf = MString::format(L"Failed to load '%s'. (code %d)\n", filename.c_str(), err);
		OutputDebugStringW(buf.c_str());
#endif
		return false;
	}

	const tinyxml2::XMLElement *root = doc.RootElement();
	if(root != NULL){
		const tinyxml2::XMLElement *bones = root->FirstChildElement("bones");
		if(bones != NULL){
			const tinyxml2::XMLElement *elem = bones->FirstChildElement("bone");
			while(elem != NULL){
				const char *jp = elem->Attribute("jp");
				const char *en = elem->Attribute("en");
				const char *group = elem->Attribute("group");
				const char *root = elem->Attribute("root");

				BoneNameSetting setting;
				setting.jp = MString::fromUtf8String(jp);
				setting.en = MString::fromUtf8String(en);
				if(group != nullptr){
					setting.group = MString::fromUtf8String(group);
				}
				m_BoneNameSetting.push_back(setting);

				if(root != nullptr && MAnsiString(root).toInt() != 0){
					m_RootBoneName = setting;
				}

				elem = elem->NextSiblingElement("bone");
			}
		}

		const tinyxml2::XMLElement *groups = root->FirstChildElement("groups");
		if(groups != NULL){
			const tinyxml2::XMLElement *elem = groups->FirstChildElement("group");
			while(elem != NULL){
				const char *jp = elem->Attribute("jp");
				const char *en = elem->Attribute("en");

				BoneGroupSetting setting;
				setting.jp = MString::fromUtf8String(jp);
				setting.en = MString::fromUtf8String(en);
				m_BoneGroupSetting.push_back(setting);

				elem = elem->NextSiblingElement("group");
			}
		}
	}

	return true;
}

int ExportGPBPlugin::makeMaterial(FILE* f, const std::vector<GPBMaterial>& materials) {

	FMES(f, "\
material colored\n\
{\n\
	u_worldViewProjectionMatrix = WORLD_VIEW_PROJECTION_MATRIX\n\
	technique\n\
	{\n\
		pass\n\
		{\n\
			vertexShader = res/shaders/colored.vert\n\
			fragmentShader = res/shaders/colored.frag\n\
		}\n\
	}\n\
}\n\
material textured\n\
{\n\
	u_worldViewProjectionMatrix = WORLD_VIEW_PROJECTION_MATRIX\n\
	sampler u_diffuseTexture\n\
	{\n\
		mipmap = true\n\
		wrapS = CLAMP\n\
		wrapT = CLAMP\n\
		minFilter = LINEAR_MIPMAP_LINEAR\n\
		magFilter = LINEAR\n\
	}\n\
	technique\n\
	{\n\
		pass\n\
		{\n\
			vertexShader = res/shaders/textured.vert\n\
			fragmentShader = res/shaders/textured.frag\n\
		}\n\
	}\n\
}\n\
");

	for (const auto& material : materials) {
		if (!material.enable) {
			continue;
		}

		const auto name = material.convName.toAnsiString();
		FMES(f, "material %s : ", name.c_str());
		if (material.useTexture) {
			FMES(f, "textured\n\
{\n\
	u_matrixPalette = MATRIX_PALETTE\n\
	sampler u_diffuseTexture\n\
	{\n\
		path = %s\n\
		wrapS = REPEAT\n\
		wrapT = REPEAT\n\
	}\n\
", material.convDiffuseTexture.toAnsiString().c_str());

		}
		else {
			FMES(f, "colored\n\
{\n\
	u_diffuseColor = %.6f, %.6f, %.6f, %.6f\n\
", material.diffuse[0], material.diffuse[1], material.diffuse[2], material.diffuse[3]);
		}

		FMES(f, "\
	renderState\n\
	{\n\
		cullFace = %s\n\
		depthTest = true\n\
	}\n\
", material.isDouble ? "false" : "true");

		FMES(f, "}\n");
	}

	return 0;
}


// .hsp は res フォルダの一つ上に配置しないといけない
int ExportGPBPlugin::makeHSP(FILE* f, const GPBBounding& bounding) {
	float fov = 45.0f * 3.141592f * 2.0f / 180.0f;
	float dist = bounding.radius / tanf(fov * 0.5f) + bounding.radius;
	FMES(f, "\
#include \"hgimg4.as\"\n\
ddim vals, 4\n\
vals(0) = %.6f, %.6f, %.6f, %.6f\n\
gpload id, \"res/%s\"\n\
//gpaddanim id, \"whole\", 0, -1, 0\n\
//gpact id, \"whole\", GPACT_PLAY\n\
setpos GPOBJ_CAMERA, vals(0), vals(1), vals(2) + vals(3)\n\
gplookat GPOBJ_CAMERA, vals(0), vals(1), vals(2)\n\
repeat\n\
  redraw 0\n\
  gpdraw\n\
  pos 8, 8\n\
  mes \"%s, \" + dist\n\
  redraw 1\n\
  await 1000 / 60\n\
loop\n\
",
		bounding.center[0], bounding.center[1], bounding.center[2], dist,
		"try01", IDENVER);
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

