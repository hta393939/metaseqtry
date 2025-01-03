//---------------------------------------------------------------------------
// このコードは ExportPMD.cpp をベースに実装されています。
//    　作成したDLLは"Plugins\Export"フォルダに入れる必要がある。
// https://github.com/gameplay3d/gameplay/blob/master/gameplay/src/Bundle.cpp
//---------------------------------------------------------------------------

#define MY_PRODUCT (0xB2B2501D)
#define MY_ID (0x0ED64D3E)
#define MY_PLUGINNAME "Export GPB Copyright(C) 2024, hta393939"
#define MY_FILETYPE "HSP GPB(*.gpb)"
#define MY_EXT "gpb"

#define IDENVER "0.12.1"

// 0 だと無効化
#define USESCALING (0)

// 1 だと拡張UI有効
#define USEEXTENDEDUI (0)

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
	FILEOUT_NO = 0,
	FILEOUT_FORCE = 1,
	FILEOUT_CONFIRM = 2,
	FILEOUT_OWFORBIDDEN = 3,
};

enum {
	FILEIN_NOTUSE = 0,
	FILEIN_USE = 1,
};


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
	/// <summary>
	/// Bone Manager から取得できる行列
	/// </summary>
	MQMatrix base_mtx;
	/// <summary>
	/// 親行列で割った行列
	/// </summary>
	MQMatrix rel_mtx;

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

		this->base_mtx.Identify();
		this->rel_mtx.Identify();
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
		const std::vector<MString>& boneNames,
		const std::vector<GPBBoneParam>& bones);

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
		float scaling,
		bool useScaleRot);

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

struct CreateDialogOptionParam
{
	bool visible_only;
	/// <summary>
	/// ボーンが1つ以上かどうか
	/// </summary>
	bool bone_exists;

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

	/// <summary>
	/// 0: 変えない, 1: 変える
	/// </summary>
	int material_conv;
	/// <summary>
	/// 1: スケールと回転も採用する
	/// </summary>
	int bone_scale_rot = 0;

	/// <summary>
	/// 0: 変えない, 1: 変える
	/// </summary>
	int bone_conv = 0;

	int additive_info = 0;
};


/// <summary>
/// GUIパーツを格納するクラス
/// MQMemo は複数行テキスト
/// </summary>
class GPBOptionDialog : public MQDialog
{
public:
	MQCheckBox *check_visible;
	MQComboBox *combo_bone;
	MQEdit *edit_textureprefix;

	MQComboBox* combo_mtlfile;
	MQComboBox* combo_hspfile;
	MQComboBox* combo_xmlanimfile;

	MQComboBox* combo_materialconv;

	MQComboBox* combo_bonescalerot;
#if (USEEXTENDEDUI!=0)
	MQComboBox* combo_boneconv;
#endif

	MQButton* btn_ok;

	BOOL OnClickOK(MQWidgetBase* sender, MQDocument doc);

	GPBOptionDialog(ExportGPBPlugin* plugin, MLanguage& language);

	/// <summary>
	/// フレームウィジェットにUIを追加する
	/// </summary>
	int addUIs(int parent_frame_id, MLanguage& language);

	int setValues(CreateDialogOptionParam *option);
	int getValues(CreateDialogOptionParam *option);

	int canceled = 0;

	// combo_bone を変更した際に呼び出す関数
	BOOL ComboBoneChanged(MQWidgetBase *sender, MQDocument doc);
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

