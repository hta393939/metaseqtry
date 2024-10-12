//---------------------------------------------------------------------------
//
//   MQMorphManager.h      Copyright(C) 1999-2024, tetraface Inc.
//
//		An accessor for morph information
//
//    　モーフ情報のアクセス
//
//---------------------------------------------------------------------------

#ifndef _MQMORPHMANAGER_H_
#define _MQMORPHMANAGER_H_

#include "MQBasePlugin.h"

class MQMorphManager
{
public:
	MQMorphManager(MQBasePlugin *plugin, MQDocument doc);

	static DWORD GetProductID();
	static DWORD GetPluginID();

	// Must call first in a 'Import' plugin.
	void BeginImport();
	// Must call last in a 'Import' plugin.
	void EndImport();

	struct TargetFlags {
		bool position;
		bool normal;
		bool texcoord;
		bool vcolor;

		TargetFlags() : position(true), normal(false), texcoord(false), vcolor(false) { }
	};

	// target type for PMD
	enum TargetType {
		TARGET_DEFAULT = 0,
		TARGET_BROW,
		TARGET_EYE,
		TARGET_LIP,
		TARGET_OTHER,
	};

	int GetBaseObjectNum();
	int EnumBaseObjects(std::vector<MQObject>& base_objs);
	int GetTargetObjects(MQObject base, std::vector<MQObject>& target_objs);
	bool GetTargetFlags(MQObject base, TargetFlags& flags);
	bool GetTargetType(MQObject base, MQObject target, TargetType& type);

	bool BindTargetObject(MQObject base, MQObject target);
	bool SetTargetFlags(MQObject base, const TargetFlags& flags);
	bool SetTargetType(MQObject base, MQObject target, TargetType type);

	bool ApplyDeformation(MQObject base, MQObject proxy);

	enum class OPERATION_MODE {
		DEFAULT,
		PMD,
		VRM,   // VRM 1.0
		VRM0x, // VRM 0.x
	};
	OPERATION_MODE GetOperationMode();
	bool SetOperationMode(OPERATION_MODE mode);

	[[deprecated("Use SetOperationMode() instead.")]]
	bool SetPMDMode(bool flag);

	// Preset
	struct PRESET {
		std::wstring name;
		struct TARGET_BIND {
			UINT base_obj_id = 0;
			UINT target_obj_id = 0;
			float weight = 1.0f;
		};
		std::vector<TARGET_BIND> morphTargetBinds;
	};
	UINT Preset_Add(const PRESET& expression);
	bool Preset_Delete(UINT id);
	bool Preset_Enumerate(std::vector<UINT>& ids);
	bool Preset_Get(UINT id, PRESET& expression);
	bool Preset_Set(UINT id, const PRESET& expression);

	struct VRM_EXPRESSION {
		enum class OverrideType {
			none,
			block,
			blend,
		};
		struct MaterialColorBind {
			UINT material_id;
			std::string type; // color, emissionColor, shadeColor, matcapColor, rimColor, outlineColor
			float targetValue[4];
		};
		struct TextureTransformBind {
			UINT material_id;
			MQCoordinate scale;
			MQCoordinate offset;
		};

		bool isBinary = false;
		OverrideType overrideBlink = OverrideType::none;
		OverrideType overrideLookAt = OverrideType::none;
		OverrideType overrideMouth = OverrideType::none;
	};
	bool Preset_GetVRMExpression(UINT id, VRM_EXPRESSION& expression);
	bool Preset_SetVRMExpression(UINT id, const VRM_EXPRESSION& expression);
	bool Preset_GetVRMExpressionMaterialColorBinds(UINT id, std::vector<VRM_EXPRESSION::MaterialColorBind>& binds);
	bool Preset_SetVRMExpressionMaterialColorBinds(UINT id, const std::vector<VRM_EXPRESSION::MaterialColorBind>& binds);
	bool Preset_GetVRMExpressionTextureTransformBinds(UINT id, std::vector<VRM_EXPRESSION::TextureTransformBind>& binds);
	bool Preset_SetVRMExpressionTextureTransformBinds(UINT id, const std::vector<VRM_EXPRESSION::TextureTransformBind>& binds);

private:
	MQBasePlugin *m_Plugin;
	MQDocument m_Doc;
	bool m_Verified;
};


#endif //_MQMORPHMANAGER_H_
