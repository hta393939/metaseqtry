//---------------------------------------------------------------------------
//
//   ExportPMD
//
//          Copyright(C) 1999-2024, tetraface Inc.
//
//     Sample for MQBoneManager.
//     Created DLL must be installed in "Plugins\Export" directory.
//     This is just a sample. It is slightly little different from the 
//     standard ExportPMD plug-in.
//
//    　MQBoneManagerのためのサンプル。
//    　作成したDLLは"Plugins\Export"フォルダに入れる必要がある。
//      あくまでサンプルで、標準のExportPMDプラグインとは若干異なる。
//
//---------------------------------------------------------------------------

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


#ifdef _WIN32
HINSTANCE s_hInstance;
wchar_t s_DllPath[MAX_PATH];
#endif

#define EPS	0.00001

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
	CFBundleRef bundleRef = CFBundleGetBundleWithIdentifier(CFSTR("tetraface.ExportPMD"));
	if(bundleRef == NULL) return MString();
	
	CFURLRef resourceUrl = CFBundleCopyResourcesDirectoryURL(bundleRef);
	char path[PATH_MAX+100];
	Boolean ret = CFURLGetFileSystemRepresentation(resourceUrl, TRUE, (UInt8*)path, PATH_MAX+100);
	CFRelease(resourceUrl);
	if(!ret) return MString();
	
	return MString::fromUtf8String(path);
}
#endif

class ExportPMDPlugin : public MQExportPlugin
{
public:
	ExportPMDPlugin();
	~ExportPMDPlugin();

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
	struct BoneIKNameSetting {
		MString bone;
		MString ik;
		MString ikend;
	};
	struct BoneGroupSetting {
		MString jp;
		MString en;
	};

	BoneNameSetting m_RootBoneName;
	std::vector<BoneNameSetting> m_BoneNameSetting;
	std::vector<BoneIKNameSetting> m_BoneIKNameSetting;
	std::vector<BoneGroupSetting> m_BoneGroupSetting;
	bool LoadBoneSettingFile();
};


ExportPMDPlugin::ExportPMDPlugin()
{
}

ExportPMDPlugin::~ExportPMDPlugin()
{
}


void ExportPMDPlugin::GetPlugInID(DWORD *Product, DWORD *ID)
{
	*Product = 0xB2B2501D;
	*ID      = 0x0ED64D3E;
}

const char *ExportPMDPlugin::GetPlugInName(void)
{
	return "Export GPB for BoneManager Copyright(C) 2024, usagi";
}

const char *ExportPMDPlugin::EnumFileType(int index)
{
	if(index == 0){
		return "GPB BoneManager(*.gpb)";
	}
	return NULL;
}

const char *ExportPMDPlugin::EnumFileExt(int index)
{
	if(index == 0){
		return "gpb";
	}
	return NULL;
}


class PMDOptionDialog : public MQDialog
{
public:
	MQCheckBox *check_visible;
	MQComboBox *combo_bone;
	MQComboBox *combo_ikend;
	MQComboBox *combo_facial;
	MQEdit *edit_modelname;
	MQMemo *memo_comment;

	PMDOptionDialog(int id, int parent_frame_id, ExportPMDPlugin *plugin, MLanguage& language);

	BOOL ComboBoneChanged(MQWidgetBase *sender, MQDocument doc);
};

PMDOptionDialog::PMDOptionDialog(int id, int parent_frame_id, ExportPMDPlugin *plugin, MLanguage& language) : MQDialog(id)
{
	MQFrame parent(parent_frame_id);

	MQGroupBox *group = CreateGroupBox(&parent, language.Search("Option"));

	MQFrame *hframe;

	check_visible = CreateCheckBox(group, language.Search("VisibleOnly"));

	hframe = CreateHorizontalFrame(group);
	CreateLabel(hframe, language.Search("Bone"));
	combo_bone = CreateComboBox(hframe);
	combo_bone->AddItem(language.Search("Disable"));
	combo_bone->AddItem(language.Search("Enable"));
	combo_bone->SetHintSizeRateX(8);
	combo_bone->SetFillBeforeRate(1);

	hframe = CreateHorizontalFrame(group);
	CreateLabel(hframe, language.Search("IKEnd"));
	combo_ikend = CreateComboBox(hframe);
	combo_ikend->AddItem(language.Search("Disable"));
	combo_ikend->AddItem(language.Search("Enable"));
	combo_ikend->SetHintSizeRateX(8);
	combo_ikend->SetFillBeforeRate(1);

	// モーフの出力可否のGUI作成
	hframe = CreateHorizontalFrame(group);
	CreateLabel(hframe, language.Search("Facial"));
	combo_facial = CreateComboBox(hframe);
	combo_facial->AddItem(language.Search("Disable"));
	combo_facial->AddItem(language.Search("Enable"));
	combo_facial->SetHintSizeRateX(8);
	combo_facial->SetFillBeforeRate(1);

	hframe = CreateHorizontalFrame(group);
	CreateLabel(hframe, language.Search("ModelName"));
	edit_modelname = CreateEdit(hframe); 
	edit_modelname->SetMaxAnsiLength(20);
	edit_modelname->SetHorzLayout(LAYOUT_FILL);

	CreateLabel(group, language.Search("Comment"));
	memo_comment = CreateMemo(group);
	//memo_comment->SetMaxLength(256);
}

BOOL PMDOptionDialog::ComboBoneChanged(MQWidgetBase *sender, MQDocument doc)
{
	combo_ikend->SetEnabled(combo_bone->GetCurrentIndex() == 1);
	return FALSE;
}


struct CreateDialogOptionParam
{
	ExportPMDPlugin *plugin;
	PMDOptionDialog *dialog;
	MLanguage *lang;

	bool visible_only;
	bool bone_exists;
	bool facial_exists;
	bool output_bone;
	bool output_ik_end;
	bool output_facial;
	MAnsiString modelname;
	MAnsiString comment;

};

static void CreateDialogOption(bool init, MQFileDialogCallbackParam *param, void *ptr)
{
	CreateDialogOptionParam *option = (CreateDialogOptionParam*)ptr;

	if(init)
	{
		PMDOptionDialog *dialog = new PMDOptionDialog(param->dialog_id, param->parent_frame_id, option->plugin, *option->lang);
		option->dialog = dialog;

		dialog->check_visible->SetChecked(option->visible_only);
		dialog->combo_bone->SetEnabled(option->bone_exists);
		dialog->combo_bone->SetCurrentIndex(option->output_bone ? 1 : 0);
		dialog->combo_ikend->SetEnabled(option->bone_exists && option->output_bone);
		dialog->combo_ikend->SetCurrentIndex(option->output_ik_end ? 1 : 0);
		dialog->combo_facial->SetEnabled(option->facial_exists);
		dialog->combo_facial->SetCurrentIndex(option->output_facial ? 1 : 0);
		dialog->edit_modelname->SetText(MString::fromAnsiString(option->modelname).c_str());
	}
	else
	{
		option->visible_only = option->dialog->check_visible->GetChecked();
		option->output_bone = option->dialog->combo_bone->GetCurrentIndex() == 1;
		option->output_ik_end = option->dialog->combo_ikend->GetCurrentIndex() == 1;
		option->output_facial = option->dialog->combo_facial->GetCurrentIndex() == 1;
		option->modelname = getMultiBytesSubstring(MString(option->dialog->edit_modelname->GetText()).toAnsiString(), 20);
		option->comment = getMultiBytesSubstring(MString(option->dialog->memo_comment->GetText()).toAnsiString(), 256);
		delete option->dialog;
	}
}


#pragma pack(push,1)
struct PMDFileHeader {
	unsigned char magic[3];
	float version;
	unsigned char model_name[20];
	unsigned char comment[256];
};

struct PMDFileVertexList {
	float pos[3];
	float normal_vec[3];
	float uv[2];
	unsigned short bone_num[2];
	unsigned char bone_weight;
	unsigned char edge_flag;
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
	int pmd_ik_index;
	int pmd_ik_end_index;
	int pmd_ik_tip;

	std::vector<int> pmd_ik_chain;

	MString name_jp;
	MString name_en;
	int root_group;
	int group;
	int ik_group;

	UINT link_id;
	int link_rate;

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
		pmd_ik_index = -1;
		pmd_ik_end_index = -1;
		pmd_ik_tip = -1;
		root_group = -1;
		group = -1;
		ik_group = -1;
		
		link_id = 0;
		link_rate = 0;

		movable = false;
		group_id = 0;
		tip_id = 0;
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

struct PMDMorphInputParam
{
	MQObject	base;
	std::vector<std::pair<MQObject, MorphType> >	target;
};

struct PMDMorphParam
{
	char skin_name[20];		//　表情名
	DWORD vertNum;			// 表情用の頂点数
	MorphType type;

	std::vector<std::pair<DWORD, MQPoint> >	vertex;
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

BOOL ExportPMDPlugin::ExportFile(int index, const wchar_t *filename, MQDocument doc)
{
	std::wstring lang = GetSettingValue(MQSETTINGVALUE_LANGUAGE);
#ifdef _WIN32
	MString dir = MFileUtil::extractDirectory(s_DllPath);
#else
	MString dir = GetResourceDir();
#endif
	MString path = MFileUtil::combinePath(dir, L"ExportPMD.resource.xml");
	MLanguage language;
	language.Load(lang, path.c_str());

	// Load bone setting
	LoadBoneSettingFile();

	MQBoneManager bone_manager(this, doc);

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
				bone_param[i].link_id = param.link_bone_id;
				bone_param[i].link_rate = (int)param.rotate;
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

	// モーフプラグインから必要な情報を取得
	MQMorphManager morph_manager(this, doc);
	int morph_num = 0;
	int morph_target_size = 0;
	std::vector<PMDMorphInputParam>	morph_intput_list;
	std::vector<std::vector<MQObject> >	morph_target_list;
	std::vector<std::vector<int> >		morph_target_index_list;

	// モーフのベースオブジェクトを取得
	morph_num = morph_manager.GetBaseObjectNum();
	if(morph_num > 0)
	{
		std::vector<MQObject> morphBaseObj;
		morph_manager.EnumBaseObjects(morphBaseObj);

		morph_intput_list.resize(morph_num);
		morph_target_list.resize(morph_num);
		morph_target_index_list.resize(morph_num);

		for(int i=0; i<morph_num; ++i)
			morph_intput_list[i].base = morphBaseObj[i];

		// モーフターゲット情報を取得
		int targetIndex = 1;
		for(int i=0; i<morph_num; ++i)
		{
			auto& targets = morph_target_list.at(i);
			auto& targetIndexes = morph_target_index_list.at(i);

			PMDMorphInputParam& iParam = morph_intput_list.at(i);

			std::vector<MQObject> target_objs;
			int target_size = morph_manager.GetTargetObjects(iParam.base, target_objs);

			for(auto it = target_objs.begin(); it != target_objs.end(); ++it){
				MQObject target = *it;
				MQMorphManager::TargetType type;
				if(morph_manager.GetTargetType(iParam.base, target, type)){
					std::pair<MQObject, MorphType> mt(target, MORPH_OTHER);
					switch(type){
					case MQMorphManager::TARGET_BROW: mt.second = MORPH_BROW; break;
					case MQMorphManager::TARGET_EYE: mt.second = MORPH_EYE; break;
					case MQMorphManager::TARGET_LIP: mt.second = MORPH_LIP; break;
					case MQMorphManager::TARGET_OTHER: mt.second = MORPH_OTHER; break;
					}
					iParam.target.push_back(mt);
					targets.push_back(target);
					targetIndexes.push_back(targetIndex++);
				}
			}

			morph_target_size += target_size;
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
	option.facial_exists = (morph_num > 0);
	option.output_bone = true;
	option.output_ik_end = true;
	option.output_facial = true;
	option.modelname = getMultiBytesSubstring(MFileUtil::extractFileNameOnly(filename).toAnsiString(), 20);
	option.comment = MAnsiString();
	// Load a setting.
	MQSetting *setting = OpenSetting();
	if(setting != NULL){
		setting->Load("VisibleOnly", option.visible_only, option.visible_only);
		setting->Load("Bone", option.output_bone, option.output_bone);
		setting->Load("IKEnd", option.output_ik_end, option.output_ik_end);
		setting->Load("Facial", option.output_facial, option.output_facial);
	}
	MQFileDialogInfo dlginfo;
	memset(&dlginfo, 0, sizeof(dlginfo));
	dlginfo.dwSize = sizeof(dlginfo);
	dlginfo.scale = scaling;
	dlginfo.hidden_flag = MQFileDialogInfo::HIDDEN_AXIS | MQFileDialogInfo::HIDDEN_INVERT_FACE;
	dlginfo.axis_x = MQFILE_TYPE_RIGHT;
	dlginfo.axis_y = MQFILE_TYPE_UP;
	dlginfo.axis_z = MQFILE_TYPE_FRONT;
	dlginfo.softname = "";
	dlginfo.dialog_callback = CreateDialogOption;
	dlginfo.dialog_callback_ptr = &option;
	MQ_ShowFileDialog("GPB Export", &dlginfo);

	// Save a setting.
	scaling = dlginfo.scale;
	if(setting != NULL){
		setting->Save("VisibleOnly", option.visible_only);
		setting->Save("Bone", option.output_bone);
		setting->Save("IKEnd", option.output_ik_end);
		setting->Save("Facial", option.output_facial);
		CloseSetting(setting);
	}

	if(!option.output_bone)
	{
		bone_num = 0;
		bone_id.clear();
		bone_param.clear();
	}

	bool isOutputFacial = option.output_facial;
	if(!isOutputFacial)
	{
		morph_num = 0;
		morph_target_size = 0;
		morph_intput_list.clear();
	}

	
	int numObj = doc->GetObjectCount();
	int numMat = doc->GetMaterialCount();

	// 頂点をひとまとめにする（単一オブジェクトしか扱えないので）
	std::vector<MQExportObject*> expobjs(numObj, nullptr);
	std::vector<std::vector<int>> orgvert_vert(numObj);
	std::vector<int> vert_orgobj;
	std::vector<int> vert_expvert;
	std::vector<MQPoint> vert_normal;
	std::vector<MQCoordinate> vert_coord;
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

		// ターゲットオブジェクトは飛ばす
		if(isOutputFacial && containsTargetObject(morph_intput_list, org_obj))
			continue;

		int vert_num = eobj->GetVertexCount();
		orgvert_vert[oi].resize(vert_num, -1);
		for(int evi=0; evi<vert_num; evi++){
			orgvert_vert[oi][evi] = total_vert_num;

			vert_orgobj.push_back(oi);
			vert_expvert.push_back(evi);

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
	for(int i=bone_num-1; i>=0; i--){ // from end to root
		if(bone_param[i].ikchain > 0){
			int chain_num = bone_param[i].ikchain;
			int idx = i;
			for(int p=0; p<chain_num; p++){
				if(bone_param[idx].parent == 0)
					break;
				if(bone_id_index.end() == bone_id_index.find(bone_param[idx].parent))
					break;
				idx = bone_id_index[bone_param[idx].parent];
				bone_param[i].pmd_ik_chain.push_back(idx);

				// IKチェイン内に別のIKチェインを含めない
				if(p+1 < chain_num && bone_param[idx].ikchain > 0)
					bone_param[idx].ikchain = 0;
			}
		}
	}
	for(int i=0; i<bone_num; i++){
		if(bone_param[i].pmd_ik_chain.size() > 0){
			bone_param[i].pmd_ik_index = pmdbone_num++;
			if(option.output_ik_end)
				bone_param[i].pmd_ik_end_index = pmdbone_num++;
			ik_chain_end_list.push_back(i);
		}
	}
	for(int i=0; i<bone_num; i++){
		if(bone_param[i].pmd_ik_index >= 0){
			for(size_t j=0; j<bone_param[i].pmd_ik_chain.size(); j++){
				if(bone_param[bone_param[i].pmd_ik_chain[j]].pmd_ik_tip == -1)
					bone_param[bone_param[i].pmd_ik_chain[j]].pmd_ik_tip = bone_param[i].pmd_ik_index;
			}
		}
	}
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

		// IK name
		bone_param[i].ik_name_jp = bone_param[i].ik_name;
		bone_param[i].ik_name_en = bone_param[i].ik_name;
		for(auto it = m_BoneNameSetting.begin(); it != m_BoneNameSetting.end(); ++it){
			if((*it).jp == bone_param[i].ik_name || (*it).en == bone_param[i].ik_name){
				bone_param[i].ik_name_jp = (*it).jp;
				bone_param[i].ik_name_en = (*it).en;
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
	// モーフ用情報収集
	std::vector<PMDMorphParam>	morph_param_list;
	if(morph_target_size != 0)
		morph_param_list.resize(morph_target_size + 1);

	// ベースオブジェクト情報
	PMDMorphParam *morph_base_param = NULL;
	if(!morph_intput_list.empty()){
		morph_base_param = &morph_param_list.front();
		strcpy_s(morph_base_param->skin_name, "base");
		morph_base_param->type = MORPH_BASE;
		morph_base_param->vertNum = 0;
	}

	// ターゲットオブジェクト情報
	if(isOutputFacial && morph_num > 0)
	{
		size_t totalIdx = 1;
		for(auto it = morph_intput_list.begin(); it != morph_intput_list.end(); ++it){
			for(auto mit = (*it).target.begin(); mit != (*it).target.end(); ++mit){
				auto& target = *mit;
				PMDMorphParam& param = morph_param_list.at(totalIdx++);
				target.first->GetName(param.skin_name, 20);
				param.type = target.second;
				param.vertNum = 0;
			}
		}
	}

	// モーフの頂点情報
	std::vector<DWORD> morph_base_vertex_index_list;
	if(isOutputFacial && morph_num > 0)
	{
		size_t paramIdx, baseIdx;
		int baseVertSize, baseExpIdx, baseOrgIdx;
		MQObject base, target;
		MQPoint basePos, targetPos;
		bool isWriteBase;
		PMDMorphParam *param;
		for(auto ite = morph_intput_list.begin(); ite != morph_intput_list.end(); ++ite)
		{
			isWriteBase = false;

			paramIdx = std::distance(morph_intput_list.begin(), ite);

			base = ite->base;
			baseIdx = doc->GetObjectIndex(base);
			if(expobjs[baseIdx] == nullptr)
				continue;
			baseVertSize = expobjs[baseIdx]->GetVertexCount();

			for(int i=0; i<baseVertSize; ++i)
			{
				baseExpIdx = orgvert_vert.at(baseIdx).at(i);
				baseOrgIdx = expobjs[baseIdx]->GetOriginalVertex(i);
				basePos = base->GetVertex(baseOrgIdx);

				for(auto tIte = ite->target.begin(); tIte != ite->target.end(); ++tIte)
				{
					target = tIte->first;
					targetPos = target->GetVertex(baseOrgIdx);
					if(!MQPointFuzzyEqual(basePos, targetPos))
					{
						isWriteBase = true;

						auto& targetList = morph_target_list.at(paramIdx);
						auto& targetIndexList = morph_target_index_list.at(paramIdx);
						param = &morph_param_list.at(targetIndexList.at(std::distance(targetList.begin(), std::find(targetList.begin(), targetList.end(), target))));
						param->vertex.push_back(std::make_pair(baseExpIdx, targetPos - basePos));
						++param->vertNum;
					}
				}

				if(isWriteBase)
				{
					morph_base_param->vertex.push_back(std::make_pair(baseExpIdx, basePos));
					morph_base_vertex_index_list.push_back(baseExpIdx);
					++morph_base_param->vertNum;
				}
			}
		}
	}

	// Open a file.
	FILE *fh;
	errno_t err = _wfopen_s(&fh, filename, L"wb");
	//errno_t err = fopen_s(&fh, filename, "w");
	if(err != 0){
		for(size_t i=0; i<expobjs.size(); i++){
			delete expobjs[i];
		}
		return FALSE;
	}

	// Header
	float version = 1.0f;
	char model_name[20];
	char comment[256];
	memset(model_name, 0, 20);
	memcpy(model_name, option.modelname.c_str(), option.modelname.length());
	memset(comment, 0, 256);
	memcpy(comment, option.comment.c_str(), option.comment.length());

	fwrite("Pmd", 3, 1, fh);
	fwrite(&version, 4, 1, fh);
	fwrite(model_name, 20, 1, fh);
	fwrite(comment, 256, 1, fh);
	// Vertex list
	DWORD dw_vert_num = total_vert_num;
	fwrite(&dw_vert_num, 4, 1, fh);
	for(int i=0; i<total_vert_num; i++)
	{
		float pos[3];
		float nrm[3];
		float uv[2];
		WORD bone_index[2]; // ボーン番号1、番号2 // モデル変形(頂点移動)時に影響
		BYTE bone_weight; // ボーン1に与える影響度 // min:0 max:100 // ボーン2への影響度は、(100 - bone_weight)
		BYTE edge_flag; // 0:通常、1:エッジ無効 // エッジ(輪郭)が有効の場合

		MQObject obj = doc->GetObject(vert_orgobj[i]);
		MQExportObject *eobj = expobjs[vert_orgobj[i]];
		MQPoint v = obj->GetVertex(eobj->GetOriginalVertex(vert_expvert[i]));
		pos[0] = v.x * scaling;
		pos[1] = v.y * scaling;
		pos[2] = -v.z * scaling;
		fwrite(pos, 4, 3, fh);

		nrm[0] = vert_normal[i].x;
		nrm[1] = vert_normal[i].y;
		nrm[2] = -vert_normal[i].z;
		fwrite(nrm, 4, 3, fh);

		uv[0] = vert_coord[i].u;
		uv[1] = vert_coord[i].v;
		fwrite(uv, 4, 2, fh);

		UINT vert_bone_id[16];
		float weights[16];
		int weight_num = 0;
		if(bone_num > 0){
			UINT vert_id = obj->GetVertexUniqueID(eobj->GetOriginalVertex(vert_expvert[i]));
			int max_num = 16;
			weight_num = bone_manager.GetVertexWeightArray(obj, vert_id, max_num, vert_bone_id, weights);
		}

		if(weight_num >= 2){
			int max_bone1 = -1;
			float max_weight1 = 0.0f;
			for(int n=0; n<weight_num; n++){
				if(max_weight1 < weights[n]){
					max_weight1 = weights[n];
					max_bone1 = n;
				}
			}
			int max_bone2 = -1;
			float max_weight2 = 0.0f;
			for(int n=0; n<weight_num; n++){
				if(n == max_bone1) continue;
				if(max_weight2 < weights[n]){
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
			if(bone_index[0] != bone_index[1]){
				bone_weight = (BYTE)floor(max_weight1 / total_weights * 100.f + 0.5f);
			}else{
				bone_weight = 100;
			}
		}else if(weight_num == 1){
			int bi = bone_id_index[vert_bone_id[0]];
			bone_index[0] = bone_param[bi].pmd_index;
			bone_index[1] = bone_index[0];
			bone_weight = 100;
		}else{
			bone_index[0] = 0;
			bone_index[1] = 0;
			bone_weight = 100;
		}
		fwrite(bone_index, 2, 2, fh);
		fwrite(&bone_weight, 1, 1, fh);
		edge_flag = 0;
		fwrite(&edge_flag, 1, 1, fh);
	}

	// Face's vertices list
	DWORD face_vert_count = 0;
	std::vector<int> material_used(numMat+1, 0);
	for(int i=0; i<numObj; i++){
		MQObject obj = doc->GetObject(i);
		if(obj == NULL)
			continue;

		if(option.visible_only && obj->GetVisible() == 0)
			continue;

		// ターゲットオブジェクトは飛ばす
		if(isOutputFacial && containsTargetObject(morph_intput_list, obj))
			continue;

		int num_face = obj->GetFaceCount();
		for(int fi=0; fi<num_face; fi++){
			int n = obj->GetFacePointCount(fi);
			if(n >= 3){
				face_vert_count += (n-2) * 3;

				int mi = obj->GetFaceMaterial(fi);
				if(mi < 0 || mi >= numMat) mi = numMat;
				material_used[mi] += (n-2);
			}
		}
	}
	fwrite(&face_vert_count, 4, 1, fh);
	int output_face_vert_count = 0;
	for(int m=0; m<=numMat; m++)
	{
		if(material_used[m] == 0) continue;

		for(int i=0; i<numObj; i++){
			MQObject obj = doc->GetObject(i);
			if(obj == NULL)
				continue;

			if(option.visible_only && obj->GetVisible() == 0)
				continue;

			// ターゲットオブジェクトは飛ばす
			if(isOutputFacial && containsTargetObject(morph_intput_list, obj))
				continue;

			MQExportObject *eobj = expobjs[i];
			if(eobj == nullptr)
				continue;

			int num_face = obj->GetFaceCount();
			for(int fi=0; fi<num_face; fi++){
				int mi = obj->GetFaceMaterial(fi);
				if(mi < 0 || mi >= numMat) mi = numMat;

				int n = eobj->GetFacePointCount(fi);
				if(n >= 3 && mi == m){
					std::vector<int> vi(n);
					std::vector<MQPoint> p(n);

					eobj->GetFacePointArray(fi, vi.data());
					for(int j=0; j<n; j++){
						p[j] = obj->GetVertex(eobj->GetOriginalVertex(vi[j]));
					}
					std::vector<int> tri((n-2)*3);
					doc->Triangulate(p.data(), n, tri.data(), (n-2)*3);

					for(int j=0; j<n-2; j++){
						WORD tvi[3];
						tvi[0] = (WORD)orgvert_vert[i][vi[tri[j*3]]];
						tvi[1] = (WORD)orgvert_vert[i][vi[tri[j*3+1]]];
						tvi[2] = (WORD)orgvert_vert[i][vi[tri[j*3+2]]];
						fwrite(tvi, 2, 3, fh);
						output_face_vert_count += 3;
					}
				}
			}
		}
	}
	assert(face_vert_count == output_face_vert_count);

	// Matrial list
	DWORD used_mat_num = 0;
	for(int i=0; i<=numMat; i++){
		if(material_used[i] > 0) used_mat_num++;
	}
	fwrite(&used_mat_num, 4, 1, fh);
	for(int i=0; i<=numMat; i++){
		if(material_used[i] == 0) continue;

		MQColor col(1,1,1);
		float dif = 0.8f;
		float alpha = 1.0f;
		float spc_pow = 5.0f;
		MQColor spc_col(0,0,0);
		MQColor amb_col(0.6f,0.6f,0.6f);
		MAnsiString texture;
		int toon = 1;
		bool edge = false;

		if(i < numMat){
			MQMaterial mat = doc->GetMaterial(i);
			if(mat != NULL){
				col = mat->GetColor();
				dif = mat->GetDiffuse();
				alpha = mat->GetAlpha();
				spc_pow = mat->GetPower();
				spc_col = mat->GetSpecularColor();
				amb_col = mat->GetAmbientColor();
				wchar_t path[_MAX_PATH];
				mat->GetTextureNameW(path, _MAX_PATH);
				texture = MFileUtil::extractFilenameAndExtension(MString(path)).toAnsiString();

				int shader = mat->GetShader();
				if(shader == MQMATERIAL_SHADER_HLSL){
					MAnsiString shader_name = mat->GetShaderName();
					if(shader_name == "pmd"){
						toon = mat->GetShaderParameterIntValue("Toon", 0);
						edge = mat->GetShaderParameterBoolValue("Edge", 0);
					}
				}
			}
		}

		float diffuse_color[3]; // dr, dg, db // 減衰色
		diffuse_color[0] = col.r * dif;
		diffuse_color[1] = col.g * dif;
		diffuse_color[2] = col.b * dif;
		fwrite(diffuse_color, 4, 3, fh);
		fwrite(&alpha, 4, 1, fh);
		fwrite(&spc_pow, 4, 1, fh);

		float specular_color[3]; // sr, sg, sb // 光沢色
		specular_color[0] = sqrtf(spc_col.r);
		specular_color[1] = sqrtf(spc_col.g);
		specular_color[2] = sqrtf(spc_col.b);
		fwrite(&specular_color, 4, 3, fh);

		float ambient_color[3]; // mr, mg, mb // 環境色(ambient)
		ambient_color[0] = amb_col.r;
		ambient_color[1] = amb_col.g;
		ambient_color[2] = amb_col.b;
		fwrite(&ambient_color, 4, 3, fh);

		BYTE toon_index = (toon >= 1 && toon <= 10) ? (BYTE)(toon-1) : 0;
		fwrite(&toon_index, 1, 1, fh);

		BYTE edge_flag = edge ? 1 : 0;
		fwrite(&edge_flag, 1, 1, fh);

		DWORD face_vert_count = material_used[i] * 3;
		fwrite(&face_vert_count, 4, 1, fh);

		MAnsiString texture_str = getMultiBytesSubstring(texture, 20);
		char texture_file_name[20]; // The end null is unnecessary.
		memset(texture_file_name, 0, 20);
		memcpy(texture_file_name, texture_str.c_str(), texture_str.length());
		fwrite(texture_file_name, 20, 1, fh);
	}
	std::vector<int>bone_group;
	// Bone list
	if(bone_num == 0){
		WORD dw_bone_num = 1;
		fwrite(&dw_bone_num, 2, 1, fh);

		static const char bone_name[20] = "default";
		fwrite(bone_name, 20, 1, fh);

		WORD parent_bone_index = 0xFFFF;
		fwrite(&parent_bone_index, 2, 1, fh);

		WORD tail_pos_bone_index = 0;
		fwrite(&tail_pos_bone_index, 2, 1, fh);

		BYTE bone_type = 0;
		fwrite(&bone_type, 1, 1, fh);
		WORD ik_parent_bone_index = 0;
		fwrite(&ik_parent_bone_index, 2, 1, fh);

		float bone_head_pos[3] = {0,0,0};
		fwrite(bone_head_pos, 4, 3, fh);

		WORD ik_data_count = 0;
		fwrite(&ik_data_count, 2, 1, fh);
	}else{
		WORD dw_bone_num = (WORD)pmdbone_num;
		fwrite(&dw_bone_num, 2, 1, fh);
		int pmdbone_index = 0;
		for(int i=0; i<bone_num; i++){
			if(bone_param[i].pmd_index >= 0 && bone_param[i].pmd_index >= pmdbone_index){
				assert(bone_param[i].pmd_index == pmdbone_index);
				MAnsiString subname = getMultiBytesSubstring(bone_param[i].name_jp.toAnsiString(), 20);
				char bone_name[20];
				memset(bone_name, 0, 20);
				memcpy(bone_name, subname.c_str(), subname.length());
				fwrite(bone_name, 20, 1, fh);

				if(bone_param[i].link_id != 0 && bone_param[i].link_rate != 100)
					bone_group.push_back(-1);
				else if(bone_param[i].group_id != 0)
					bone_group.push_back(bone_param[i].group_id);
				else
					bone_group.push_back(-1);

				WORD parent_bone_index = 0xFFFF;
				if(bone_param[i].parent != 0){
					auto parent_it = bone_id_index.find(bone_param[i].parent);
					if(parent_it != bone_id_index.end()){
						parent_bone_index = (WORD)bone_param[(*parent_it).second].pmd_index;
					}
				}
				fwrite(&parent_bone_index, 2, 1, fh);

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
				if(bone_param[i].link_id != 0 && bone_param[i].link_rate != 100){
					tail_pos_bone_index = bone_id_index[bone_param[i].link_id]; // 回転連動ボーンの連動先
				}
				fwrite(&tail_pos_bone_index, 2, 1, fh);

				// bone_type
				//   0:回転 1:回転と移動 2:IK 3:不明 4:IK影響下 5:回転影響下 6:IK接続先 7:非表示 8:捩れ 9:回転連動
				BYTE bone_type = (bone_param[i].parent == 0) ? 1 : 0;
				if(bone_param[i].tip_id == 0 && bone_param[i].child_num != 0) bone_type = 7;
				else if(bone_param[i].pmd_ik_tip >= 0) bone_type = 4;
				else if(bone_param[i].pmd_ik_index >= 0) bone_type = 6;
				else if(bone_param[i].link_id != 0){
					if(bone_param[i].link_rate == 100) bone_type = 5;
					else bone_type = 9;
				}
				else if(bone_param[i].twist) bone_type = 8;
				else if(bone_param[i].child_num == 0) bone_type = 7;
				if(bone_type <= 1){
					bone_type = bone_param[i].movable ? 1 : 0;
				}
				fwrite(&bone_type, 1, 1, fh);

				// ik_parent_bone_index
				WORD ik_parent_bone_index = 0; // IKボーン番号(影響IKボーン。ない場合は0)
				if(bone_param[i].pmd_ik_index >= 0)
					ik_parent_bone_index = (WORD)bone_param[i].pmd_ik_index;
				else if(bone_param[i].pmd_ik_tip >= 0)
					ik_parent_bone_index = (WORD)bone_param[i].pmd_ik_tip;
				else if(bone_param[i].link_id != 0){
					if( bone_param[i].link_rate == 100){
						ik_parent_bone_index = bone_id_index[bone_param[i].link_id];
					}else{
						ik_parent_bone_index = bone_param[i].link_rate;
					}
				}
				fwrite(&ik_parent_bone_index, 2, 1, fh);

				float bone_head_pos[3];
				bone_head_pos[0] = bone_param[i].org_pos.x * scaling;
				bone_head_pos[1] = bone_param[i].org_pos.y * scaling;
				bone_head_pos[2] = -bone_param[i].org_pos.z * scaling;
				fwrite(&bone_head_pos, 4, 3, fh);

				pmdbone_index++;
			}
		}

		// IKの先端を別ノードとして登録する（IKリストとリンクさせる）
		for(int i=0; i<bone_num; i++){
			if(bone_param[i].pmd_ik_chain.empty()) continue;
			MString name = bone_param[i].ik_name_jp;
			MString ik_end_name = bone_param[i].ik_tip_name_jp;

			if(name.length() == 0 || ik_end_name.length() == 0){
				for(auto ikt=m_BoneIKNameSetting.begin(); ikt!=m_BoneIKNameSetting.end(); ++ikt){
					if((*ikt).bone == bone_param[i].name || (*ikt).bone == bone_param[i].name_en){
						MString n = (*ikt).ik;
						MString en = (*ikt).ikend;
						for(auto it = m_BoneNameSetting.begin(); it != m_BoneNameSetting.end(); ++it){
							if((*it).en == name){
								n = (*it).jp;
								break;
							}
						}
						for(auto it = m_BoneNameSetting.begin(); it != m_BoneNameSetting.end(); ++it){
							if((*it).en == ik_end_name){
								en = (*it).jp;
								break;
							}
						}
						if(name.length() == 0) name = n;
						if(ik_end_name.length() == 0) ik_end_name = en;
						break;
					}
				}
			}
			if(name.length() == 0){
				name = MString(L"IK-") + bone_param[i].name;
			}
			if(ik_end_name.length() == 0){
				ik_end_name = name + MString(L" end");
			}

			MAnsiString subname = getMultiBytesSubstring(name.toAnsiString(), 20);
			char bone_name[20];
			memset(bone_name, 0, 20);
			memcpy(bone_name, subname.c_str(), subname.length());
			fwrite(bone_name, 20, 1, fh);
			bone_group.push_back(-2);
			WORD parent_bone_index = 0xFFFF;
			//if(bone_param[bone_param[i].pmd_ik_chain.back()].pmd_ik_index >= 0){
			//	parent_bone_index = (WORD)bone_param[bone_param[i].pmd_ik_chain.back()].pmd_ik_index;
			//}
			if(bone_param[i].ikparent!=0){
				if(!bone_param[i].ikparent_isik){
					parent_bone_index = bone_id_index[bone_param[i].ikparent];
				}else{
					parent_bone_index = bone_param[bone_id_index[bone_param[i].ikparent]].pmd_ik_index;
				}
			}
			fwrite(&parent_bone_index, 2, 1, fh);

			WORD tail_pos_bone_index = 0;
			if(bone_param[i].pmd_ik_end_index >= 0){
				tail_pos_bone_index = (WORD)bone_param[i].pmd_ik_end_index;
			}else if(bone_param[i].pmd_ik_tip >= 0){
				tail_pos_bone_index = (WORD)bone_param[i].pmd_ik_tip;
			}
			fwrite(&tail_pos_bone_index, 2, 1, fh);

			BYTE bone_type = 2; // ボーンの種類 0:回転 1:回転と移動 2:IK 3:不明 4:IK影響下 5:回転影響下 6:IK接続先 7:非表示
			fwrite(&bone_type, 1, 1, fh);

			WORD ik_parent_bone_index = 0; // IKボーン番号(影響IKボーン。ない場合は0)
			fwrite(&ik_parent_bone_index, 2, 1, fh);

			float bone_head_pos[3];
			bone_head_pos[0] = bone_param[i].org_pos.x * scaling;
			bone_head_pos[1] = bone_param[i].org_pos.y * scaling;
			bone_head_pos[2] = -bone_param[i].org_pos.z * scaling;
			fwrite(&bone_head_pos, 4, 3, fh);
			
			assert(pmdbone_index == bone_param[i].pmd_ik_index);
			pmdbone_index++;

			if(option.output_ik_end){
				MAnsiString subname = getMultiBytesSubstring(ik_end_name.toAnsiString(), 20);
				char bone_name[20];
				memset(bone_name, 0, 20);
				memcpy(bone_name, subname.c_str(), subname.length());
				fwrite(bone_name, 20, 1, fh);
				bone_group.push_back(-1);
				
				WORD parent_bone_index;
				parent_bone_index = (WORD)bone_param[i].pmd_ik_index;
				fwrite(&parent_bone_index, 2, 1, fh);

				WORD tail_pos_bone_index = 0;
				fwrite(&tail_pos_bone_index, 2, 1, fh);

				BYTE bone_type = 7; // ボーンの種類 0:回転 1:回転と移動 2:IK 3:不明 4:IK影響下 5:回転影響下 6:IK接続先 7:非表示
				fwrite(&bone_type, 1, 1, fh);

				WORD ik_parent_bone_index = 0; // IKボーン番号(影響IKボーン。ない場合は0)
				fwrite(&ik_parent_bone_index, 2, 1, fh);

				MQPoint parent_dir(0,0,-1);
				if(bone_param[i].parent != 0 && bone_id_index.end() != bone_id_index.find(bone_param[i].parent)){
					parent_dir = bone_param[bone_id_index[bone_param[i].parent]].org_pos - bone_param[i].org_pos;
					parent_dir.normalize();
				}
				MQPoint vec1(0,-1,0), vec2(0,0,-1);
				MQPoint ik_end_dir;
				if(fabs(GetInnerProduct(parent_dir, vec1)) < fabs(GetInnerProduct(parent_dir, vec2))){
					ik_end_dir = vec1;
				}else{
					ik_end_dir = vec2;
				}

				if(!bone_param[i].children.empty()){
					MQPoint child_dir = bone_param[bone_param[i].children.front()].org_pos - bone_param[i].org_pos;
					child_dir.normalize();
					if(GetInnerProduct(child_dir, ik_end_dir) > 0){
						ik_end_dir = -ik_end_dir;
					}
				}
				MQPoint ik_end_pos;
				ik_end_pos = bone_param[i].org_pos + ik_end_dir;

				float bone_head_pos[3];
				bone_head_pos[0] = ik_end_pos.x * scaling;
				bone_head_pos[1] = ik_end_pos.y * scaling;
				bone_head_pos[2] = -ik_end_pos.z * scaling;
				fwrite(&bone_head_pos, 4, 3, fh);

				assert(pmdbone_index == bone_param[i].pmd_ik_end_index);

				pmdbone_index++;
			}
		}

		// IK list
		WORD ik_data_count = (WORD)ik_chain_end_list.size();
		fwrite(&ik_data_count, 2, 1, fh);
		for(size_t id=0; id<ik_chain_end_list.size(); id++){
			int idx = ik_chain_end_list[id];

			WORD ik_bone_index = (WORD)bone_param[idx].pmd_ik_index; // IKノード番号
			fwrite(&ik_bone_index, 2, 1, fh);

			WORD ik_target_bone_index = bone_param[idx].pmd_index; // IKターゲットボーン番号 // IKボーンが最初に接続するボーン
			fwrite(&ik_target_bone_index, 2, 1, fh);

			BYTE ik_chain_length = (BYTE)bone_param[idx].pmd_ik_chain.size(); // IKチェーンの長さ(子の数)
			fwrite(&ik_chain_length, 1, 1, fh);

			WORD iterations = 10; // 再帰演算回数 // IK値1
			fwrite(&iterations, 2, 1, fh);

			float control_weight = 0.5f; // 演算1回あたりの制限角度 // IK値2
			fwrite(&control_weight, 4, 1, fh);

			std::vector<WORD> ik_child_bone_index(bone_param[idx].pmd_ik_chain.size()); // IK影響下のボーン番号
			for(size_t j=0; j<ik_child_bone_index.size(); j++){
				ik_child_bone_index[j] = bone_param[bone_param[idx].pmd_ik_chain[j]].pmd_index;
			}
			fwrite(ik_child_bone_index.data(), 2, ik_child_bone_index.size(), fh);
		}
	}
	// モーフ情報書き込み
	// Facial
	WORD skin_count = static_cast<WORD>(morph_param_list.size());
	fwrite(&skin_count, 2, 1, fh);
	float skin_vert_pos[3];
	DWORD skin_vert_count = 0;
	DWORD skin_vert_index = 0;
	PMDMorphParam *mParam;
	for(int i=0; i<(int)skin_count; i++)
	{
		mParam = &morph_param_list.at(i);

		char facial_name[20];
		memset(facial_name, 0, 20);

		auto subname = getMultiBytesSubstring(mParam->skin_name, 20);
		memcpy(facial_name, subname.c_str(), subname.length());

		fwrite(facial_name, sizeof(char) * 20, 1, fh);
		fwrite(&mParam->vertNum, sizeof(DWORD), 1, fh);
		fwrite(&mParam->type, sizeof(BYTE), 1, fh);

		skin_vert_count = mParam->vertNum;
		for(DWORD j=0; j<skin_vert_count; ++j)
		{
			auto vertex = &mParam->vertex.at(j);

			if(i == 0)
				skin_vert_index = vertex->first;
			else
				skin_vert_index = static_cast<DWORD>(std::distance(morph_base_vertex_index_list.begin(), std::find(morph_base_vertex_index_list.begin(), morph_base_vertex_index_list.end(), vertex->first)));
			fwrite(&skin_vert_index, sizeof(DWORD), 1, fh);

			skin_vert_pos[0] = vertex->second.x * scaling;
			skin_vert_pos[1] = vertex->second.y * scaling;
			skin_vert_pos[2] = -vertex->second.z * scaling;

			fwrite(skin_vert_pos, sizeof(float), 3, fh);
		}
	}

	// 表情枠用表示リスト
	BYTE skin_disp_count = static_cast<BYTE>(skin_count);
	if(skin_disp_count != 0)
		skin_disp_count -= 1;
	fwrite(&skin_disp_count, 1, 1, fh);
	for(WORD i=1; i<=(WORD)skin_disp_count; i++)
		fwrite(&i, sizeof(WORD), 1, fh);

	// ボーン枠用枠名リスト
	std::map<int,int> group_table;
	BYTE bone_disp_name_count = (BYTE)groups.size();
	if(ik_chain_end_list.size()!=0)bone_disp_name_count++;
	fwrite(&bone_disp_name_count, 1, 1, fh);
	int i = 1;
	for(auto it = groups.begin(); it != groups.end(); it++){
		group_table.insert(std::pair<int,int>(it->first,i++));
		MAnsiString name = getMultiBytesSubstring(group_names[it->first].first.toAnsiString(), 48);
		char disp_name[50];
		memset(disp_name, 0, 50);
		memcpy(disp_name, name.c_str(), name.length());
		disp_name[name.length()] = 0x0A;
		fwrite(disp_name, 50, 1, fh); // 枠名(50Bytes/枠)
	}
	if(ik_chain_end_list.size()!=0){
		MString name(L"ＩＫ");
		MAnsiString str = name.toAnsiString();
		char disp_name[50];
		memset(disp_name, 0, 50);
		memcpy(disp_name, str.c_str(), str.length());
		disp_name[str.length()] = 0x0A;
		fwrite(disp_name, 50, 1, fh); // 枠名(50Bytes/枠)
	
	}
	
	// ボーン枠用表示リスト
	DWORD bone_disp_count = 0;
	for(int i = 0;i < (int)bone_group.size();i++){
		if(bone_group[i]==-1)continue;
		bone_disp_count++;
	}
	fwrite(&bone_disp_count, 4, 1, fh);
	for(int i = 0;i < (int)bone_group.size();i++){
		if(bone_group[i] == -1)continue;
		WORD bone_index = i;
		fwrite(&bone_index, 2, 1, fh);
		BYTE bone_disp_frame_index; // 表示枠番号 // センター:00 他の表示枠:01～ // センター枠にセンター(index 0)以外のボーンを指定しても表示されません。
		if(bone_group[i] == -2)
			bone_disp_frame_index = (BYTE)(groups.size()+1);
		else
			bone_disp_frame_index = (BYTE)(group_table[bone_group[i]]);
		fwrite(&bone_disp_frame_index, 1, 1, fh);
	}

	// 英名
	BYTE english_name_compatibility = 1;
	fwrite(&english_name_compatibility, 1, 1, fh);
	fwrite(model_name, 20, 1, fh);
	fwrite(comment, 256, 1, fh);
	if(bone_num == 0){
		char bone_name[20];
		memset(bone_name, 0, 20);
		strcpy_s(bone_name, "default");
		fwrite(bone_name, 20, 1, fh);
	}else{
		int pmdbone_index = 0;
		for(int i=0; i<bone_num; i++){
			if(bone_param[i].pmd_index >= 0 && bone_param[i].pmd_index >= pmdbone_index){
				assert(bone_param[i].pmd_index == pmdbone_index);
				char bone_name[20];
				MAnsiString subname = getMultiBytesSubstring(bone_param[i].name_en.toAnsiString(), 20);
				memset(bone_name, 0, 20);
				memcpy(bone_name, subname.c_str(), subname.length());

				fwrite(bone_name, 20, 1, fh);

				pmdbone_index++;
			}
		}

		for(int i=0; i<bone_num; i++){
			if(bone_param[i].pmd_ik_chain.empty()) continue;

			MAnsiString name = getMultiBytesSubstring(bone_param[i].ik_name_en.toAnsiString(),20);
			MAnsiString ik_end_name = getMultiBytesSubstring(bone_param[i].ik_tip_name_en.toAnsiString(),20);
			char bone_name[20];
			memset(bone_name, 0, 20);
			memcpy(bone_name, name.c_str(), name.length());
			fwrite(bone_name, 20, 1, fh);

			assert(pmdbone_index == bone_param[i].pmd_ik_index);
			pmdbone_index++;

			if(option.output_ik_end){
				char bone_name[20];
				memset(bone_name, 0, 20);
				memcpy(bone_name, ik_end_name.c_str(), ik_end_name.length());
				fwrite(bone_name, 20, 1, fh);

				assert(pmdbone_index == bone_param[i].pmd_ik_end_index);
				pmdbone_index++;
			}
		}
	}

	// skin_name
	for(int i=1; i<=morph_target_size; ++i)
	{
		char facial_name[20];
		memset(facial_name, 0, 20);

		auto subname = getMultiBytesSubstring(morph_param_list[i].skin_name, 20);
		memcpy(facial_name, subname.c_str(), subname.length());

		fwrite(facial_name, sizeof(char) * 20, 1, fh);
	}

	for(auto it=groups.begin(); it!= groups.end(); it++){
		MString wsubname = group_names[it->first].second;
		MAnsiString subname = getMultiBytesSubstring(wsubname.toAnsiString(), 48);
		char disp_name[50];
		memset(disp_name, 0, 50);
		memcpy(disp_name, subname.c_str(), subname.length());
		disp_name[subname.length()] = 0x0A;
		fwrite(disp_name, 50, 1, fh); // 枠名(50Bytes/枠)
	}
	//IK枠
	if(ik_chain_end_list.size()!=0){
		MString name = L"IK";
		MAnsiString subname = getMultiBytesSubstring(name.toAnsiString(), 48);
		char disp_name[50];
		memset(disp_name, 0, 50);
		memcpy(disp_name, subname.c_str(), subname.length());
		disp_name[subname.length()] = 0x0A;
		fwrite(disp_name, 50, 1, fh); // 枠名(50Bytes/枠)
	}
	for(int i=0; i<10; i++){
		char toon_file_name[100];
		memset(toon_file_name, 0, 100);
		MAnsiString n = MAnsiString::format("toon%02d.bmp", i+1);
		memcpy(toon_file_name, n.c_str(), n.length());
		fwrite(toon_file_name, 100, 1, fh);
	}

	DWORD rigid_body_count = 0;
	fwrite(&rigid_body_count, 4, 1, fh);

	DWORD joint_count = 0;
	fwrite(&joint_count, 4, 1, fh);

	for(size_t i=0; i<expobjs.size(); i++){
		delete expobjs[i];
	}

	if(fclose(fh) != 0){
		return FALSE;
	}
	return TRUE;
}

bool ExportPMDPlugin::LoadBoneSettingFile()
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

		const tinyxml2::XMLElement *ik_names = root->FirstChildElement("ik_names");
		if(ik_names != NULL){
			const tinyxml2::XMLElement *elem = ik_names->FirstChildElement("ik");
			while(elem != NULL){
				const char *bone = elem->Attribute("bone");
				const char *ik = elem->Attribute("ik");
				const char *end = elem->Attribute("end");

				BoneIKNameSetting setting;
				setting.bone = MString::fromUtf8String(bone);
				setting.ik = MString::fromUtf8String(ik);
				setting.ikend = MString::fromUtf8String(end);
				m_BoneIKNameSetting.push_back(setting);

				elem = elem->NextSiblingElement("ik");
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

//---------------------------------------------------------------------------
//  GetPluginClass
//    プラグインのベースクラスを返す
//---------------------------------------------------------------------------
MQBasePlugin *GetPluginClass()
{
	static ExportPMDPlugin plugin;
	return &plugin;
}

