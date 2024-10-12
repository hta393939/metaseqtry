//---------------------------------------------------------------------------
//
//   MQMorphManager.cpp      Copyright(C) 1999-2024, tetraface Inc.
//
//		An accessor for morph information
//
//    　モーフ情報のアクセス
//
//---------------------------------------------------------------------------

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "MQMorphManager.h"

static const DWORD morph_plugin_product = 0x56A31D20;
static const DWORD morph_plugin_id = 0xC452C6DB;


MQMorphManager::MQMorphManager(MQBasePlugin *plugin, MQDocument doc)
	: m_Plugin(plugin), m_Doc(doc)
{
	int version = 0x4700;

	void *array[3];
	array[0] = (void*)"version";
	array[1] = &version;
	array[2] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "QueryAPIVersion", array);
	m_Verified = (ret == 1);
}

DWORD MQMorphManager::GetProductID()
{
	return morph_plugin_product;
}

DWORD MQMorphManager::GetPluginID()
{
	return morph_plugin_id;
}

void MQMorphManager::BeginImport()
{
	m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "BeginImport", nullptr);
}

void MQMorphManager::EndImport()
{
	m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "EndImport", nullptr);
}

int MQMorphManager::GetBaseObjectNum()
{
	if(!m_Verified) return 0;

	int morph_num = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "GetBaseObject", nullptr);
	return morph_num;
}

int MQMorphManager::EnumBaseObjects(std::vector<MQObject>& bases)
{
	int morph_num = GetBaseObjectNum();
	if(morph_num == 0){
		bases.clear();
		return 0;
	}
	bases.resize(morph_num);
	return m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "GetBaseObject", bases.data());
}

int MQMorphManager::GetTargetObjects(MQObject base, std::vector<MQObject>& targets)
{
	if(!m_Verified){
		targets.clear();
		return 0;
	}

	void *array[5];
	array[0] = (void*)"base";
	array[1] = base;
	array[2] = (void*)"target";
	array[3] = nullptr;
	array[4] = nullptr;

	int size = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "GetTargetObject", array);
	
	if(size > 0){
		targets.resize(size);
		array[3] = targets.data();
		m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "GetTargetObject", array);
	}else{
		targets.clear();
	}
	return size;
}

bool MQMorphManager::GetTargetFlags(MQObject base, TargetFlags& flags)
{
	if(!m_Verified) return false;

	void *array[11];
	array[0] = (void*)"base";
	array[1] = base;
	array[2] = (void*)"position";
	array[3] = (void*)&flags.position;
	array[4] = (void*)"normal";
	array[5] = (void*)&flags.normal;
	array[6] = (void*)"texcoord";
	array[7] = (void*)&flags.texcoord;
	array[8] = (void*)"vcolor";
	array[9] = (void*)&flags.vcolor;
	array[10] = nullptr;

	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "GetTargetFlags", array);
	return (ret != 0);
}

bool MQMorphManager::GetTargetType(MQObject base, MQObject target, TargetType& type)
{
	if(!m_Verified) return false;

	void *array[7];
	array[0] = (void*)"base";
	array[1] = base;
	array[2] = (void*)"target";
	array[3] = target;
	array[4] = (void*)"type";
	array[5] = nullptr;
	array[6] = nullptr;

	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "GetTargetType", array);
	if(ret && array[5] != nullptr){
		if(strcmp((char*)array[5], "brow") == 0)
			type = TARGET_BROW;
		else if(strcmp((char*)array[5], "eye") == 0)
			type = TARGET_EYE;
		else if(strcmp((char*)array[5], "lip") == 0)
			type = TARGET_LIP;
		else if(strcmp((char*)array[5], "other") == 0)
			type = TARGET_OTHER;
		else
			type = TARGET_DEFAULT;
		return true;
	}
	return false;
}

bool MQMorphManager::BindTargetObject(MQObject base, MQObject target)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"base";
	array[1] = base;
	array[2] = (void*)"target";
	array[3] = target;
	array[4] = nullptr;

	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "BindTargetObject", array);
	return (ret != 0);
}

bool MQMorphManager::SetTargetFlags(MQObject base, const TargetFlags& flags)
{
	if(!m_Verified) return false;

	void *array[11];
	array[0] = (void*)"base";
	array[1] = base;
	array[2] = (void*)"position";
	array[3] = (void*)&flags.position;
	array[4] = (void*)"normal";
	array[5] = (void*)&flags.normal;
	array[6] = (void*)"texcoord";
	array[7] = (void*)&flags.texcoord;
	array[8] = (void*)"vcolor";
	array[9] = (void*)&flags.vcolor;
	array[10] = nullptr;

	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "SetTargetFlags", array);
	return (ret != 0);
}

bool MQMorphManager::SetTargetType(MQObject base, MQObject target, TargetType type)
{
	if(!m_Verified) return false;

	void *array[7];
	array[0] = (void*)"base";
	array[1] = base;
	array[2] = (void*)"target";
	array[3] = target;
	array[4] = (void*)"type";
	array[5] = &type;
	array[6] = nullptr;

	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "SetTargetType", array);
	return (ret != 0);
}

bool MQMorphManager::ApplyDeformation(MQObject base, MQObject proxy)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"base";
	array[1] = base;
	array[2] = (void*)"proxy";
	array[3] = proxy;
	array[4] = nullptr;
	return m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "ApplyDeformation", array) != 0;
}

MQMorphManager::OPERATION_MODE MQMorphManager::GetOperationMode()
{
	if(!m_Verified) return OPERATION_MODE::DEFAULT;

	const char *mode = nullptr;
	void *array[3];
	array[0] = (void*)"mode";
	array[1] = (void*)&mode;
	array[2] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "GetOperationMode", array);
	if(ret == 0 || mode == nullptr)
		return OPERATION_MODE::DEFAULT;
	if(strcmp(mode, "default") == 0) return OPERATION_MODE::DEFAULT;
	if(strcmp(mode, "pmd") == 0) return OPERATION_MODE::PMD;
	if(strcmp(mode, "vrm") == 0) return OPERATION_MODE::VRM;
	if(strcmp(mode, "vrm0x") == 0) return OPERATION_MODE::VRM0x;
	return OPERATION_MODE::DEFAULT;
}

bool MQMorphManager::SetOperationMode(OPERATION_MODE mode)
{
	if(!m_Verified) return false;

	void *array[3];
	array[0] = (void*)"mode";
	switch(mode){
	case OPERATION_MODE::DEFAULT: array[1] = (void*)"default"; break;
	case OPERATION_MODE::PMD: array[1] = (void*)"pmd"; break;
	case OPERATION_MODE::VRM: array[1] = (void*)"vrm"; break;
	case OPERATION_MODE::VRM0x: array[1] = (void*)"vrm0x"; break;
	default: return false;
	}
	array[2] = nullptr;

	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "SetOperationMode", array);
	return (ret != 0);
}

bool MQMorphManager::SetPMDMode(bool flag)
{
	if(!m_Verified) return false;

	void *array[3];
	array[0] = (void*)"mode";
	array[1] = flag ? (void*)"pmd" : (void*)"default";
	array[2] = nullptr;

	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "SetOperationMode", array);
	return (ret != 0);
}

UINT MQMorphManager::Preset_Add(const PRESET& preset)
{
	if(!m_Verified) return false;

	int num = (int)preset.morphTargetBinds.size();
	void *array[15];
	array[0] = (void*)"name";
	array[1] = (void*)preset.name.c_str();
	array[2] = (void*)"targets";
	array[3] = (void*)preset.morphTargetBinds.data();
	array[4] = (void*)"targetsNum";
	array[5] = (void*)&num;
	array[6] = nullptr;

	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "Preset_Add", array);
	return (UINT)ret;
}

bool MQMorphManager::Preset_Delete(UINT id)
{
	if(!m_Verified) return false;

	void *array[3];
	array[0] = (void*)"id";
	array[1] = (void*)&id;
	array[2] = nullptr;

	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "Preset_Delete", array);
	return (ret != 0);
}

bool MQMorphManager::Preset_Enumerate(std::vector<UINT>& ids)
{
	int num = 0;
	void *array[3];
	array[0] = (void*)"num";
	array[1] = (void*)&num;
	array[2] = nullptr;

	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "Preset_Enumerate", array);
	if(ret == 0)
		return false;
	ids.resize(num);

	array[0] = (void*)"ids";
	array[1] = (void*)ids.data();
	ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "Preset_Enumerate", array);
	return (ret != 0);
}

bool MQMorphManager::Preset_Get(UINT id, PRESET& preset)
{
	if(!m_Verified) return false;

	int num = 0;
	void *array[7];
	array[0] = (void*)"id";
	array[1] = (void*)&id;
	array[2] = (void*)"targetsNum";
	array[3] = (void*)&num;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "Preset_Get", array);
	if(ret == 0)
		return false;
	preset.morphTargetBinds.resize(num);

	const wchar_t *name = nullptr;
	array[2] = (void*)"name";
	array[3] = (void*)&name;
	array[4] = (void*)"targets";
	array[5] = (void*)preset.morphTargetBinds.data();
	array[6] = nullptr;
	ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "Preset_Get", array);
	if(ret == 0)
		return false;

	if(name)
		preset.name = std::wstring(name);
	else
		preset.name.clear();
	return (ret != 0);
}

bool MQMorphManager::Preset_Set(UINT id, const PRESET& preset)
{
	if(!m_Verified) return false;

	int num = (int)preset.morphTargetBinds.size();
	void *array[9];
	array[0] = (void*)"id";
	array[1] = (void*)&id;
	array[2] = (void*)"name";
	array[3] = (void*)preset.name.c_str();
	array[4] = (void*)"targets";
	array[5] = (void*)preset.morphTargetBinds.data();
	array[6] = (void*)"targetsNum";
	array[7] = (void*)&num;
	array[8] = nullptr;

	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "Preset_Set", array);
	return (ret != 0);
}

bool MQMorphManager::Preset_GetVRMExpression(UINT id, VRM_EXPRESSION& expression)
{
	if(!m_Verified) return false;

	const char *overrideBlink = nullptr, *overrideLookAt = nullptr, *overrideMouth = nullptr;
	void *array[17];
	array[0] = (void*)"id";
	array[1] = (void*)&id;
	array[2] = (void*)"isBinary";
	array[3] = (void*)&expression.isBinary;
	array[4] = (void*)"overrideBlink";
	array[5] = (void*)&overrideBlink;
	array[6] = (void*)"overrideLookAt";
	array[7] = (void*)&overrideLookAt;
	array[8] = (void*)"overrideMouth";
	array[9] = (void*)&overrideMouth;
	array[10] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "Preset_GetVRMExpression", array);
	if(ret == 0)
		return false;

	if(overrideBlink != nullptr){
		if(strcmp(overrideBlink, "none") == 0)
			expression.overrideBlink = VRM_EXPRESSION::OverrideType::none;
		else if(strcmp(overrideBlink, "block") == 0)
			expression.overrideBlink = VRM_EXPRESSION::OverrideType::block;
		else if(strcmp(overrideBlink, "blend") == 0)
			expression.overrideBlink = VRM_EXPRESSION::OverrideType::blend;
	}
	if(overrideLookAt != nullptr){
		if(strcmp(overrideLookAt, "none") == 0)
			expression.overrideLookAt = VRM_EXPRESSION::OverrideType::none;
		else if(strcmp(overrideLookAt, "block") == 0)
			expression.overrideLookAt = VRM_EXPRESSION::OverrideType::block;
		else if(strcmp(overrideLookAt, "blend") == 0)
			expression.overrideLookAt = VRM_EXPRESSION::OverrideType::blend;
	}
	if(overrideMouth != nullptr){
		if(strcmp(overrideMouth, "none") == 0)
			expression.overrideMouth = VRM_EXPRESSION::OverrideType::none;
		else if(strcmp(overrideMouth, "block") == 0)
			expression.overrideMouth = VRM_EXPRESSION::OverrideType::block;
		else if(strcmp(overrideMouth, "blend") == 0)
			expression.overrideMouth = VRM_EXPRESSION::OverrideType::blend;
	}
	return (ret != 0);
}

bool MQMorphManager::Preset_SetVRMExpression(UINT id, const VRM_EXPRESSION& expression)
{
	if(!m_Verified) return false;

	void *array[11];
	array[0] = (void*)"id";
	array[1] = (void*)&id;
	array[2] = (void*)"isBinary";
	array[3] = (void*)&expression.isBinary;
	array[4] = (void*)"overrideBlink";
	switch(expression.overrideBlink){
	default: array[5] = (void*)"none"; break;
	case VRM_EXPRESSION::OverrideType::block: array[5] = (void*)"block"; break;
	case VRM_EXPRESSION::OverrideType::blend: array[5] = (void*)"blend"; break;
	}
	array[6] = (void*)"overrideLookAt";
	switch(expression.overrideLookAt){
	default: array[7] = (void*)"none"; break;
	case VRM_EXPRESSION::OverrideType::block: array[7] = (void*)"block"; break;
	case VRM_EXPRESSION::OverrideType::blend: array[7] = (void*)"blend"; break;
	}
	array[8] = (void*)"overrideMouth";
	switch(expression.overrideMouth){
	default: array[9] = (void*)"none"; break;
	case VRM_EXPRESSION::OverrideType::block: array[9] = (void*)"block"; break;
	case VRM_EXPRESSION::OverrideType::blend: array[9] = (void*)"blend"; break;
	}
	array[10] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "Preset_SetVRMExpression", array);
	return (ret != 0);
}

bool MQMorphManager::Preset_GetVRMExpressionMaterialColorBinds(UINT id, std::vector<VRM_EXPRESSION::MaterialColorBind>& binds)
{
	if(!m_Verified) return false;

	int num = 0;
	void *array[11];
	array[0] = (void*)"id";
	array[1] = (void*)&id;
	array[2] = (void*)"num";
	array[3] = (void*)&num;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "Preset_GetVRMExpressionMaterialColorBinds", array);
	if(ret == 0)
		return false;

	binds.resize(num);
	for(int i=0; i<num; i++){
		const char *type = nullptr;
		array[0] = (void*)"id";
		array[1] = (void*)&id;
		array[2] = (void*)"index";
		array[3] = (void*)&i;
		array[4] = (void*)"material";
		array[5] = (void*)&binds[i].material_id;
		array[6] = (void*)"type";
		array[7] = (void*)&type;
		array[8] = (void*)"value";
		array[9] = (void*)binds[i].targetValue;
		array[10] = nullptr;
		m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "Preset_GetVRMExpressionMaterialColorBinds", array);

		if(type != nullptr)
			binds[i].type = type;
		else
			binds[i].type.clear();
	}
	return true;
}

bool MQMorphManager::Preset_SetVRMExpressionMaterialColorBinds(UINT id, const std::vector<VRM_EXPRESSION::MaterialColorBind>& binds)
{
	if(!m_Verified) return false;

	int num = (int)binds.size();
	void *array[11];
	array[0] = (void*)"id";
	array[1] = (void*)&id;
	array[2] = (void*)"num";
	array[3] = (void*)&num;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "Preset_SetVRMExpressionMaterialColorBinds", array);
	if(ret == 0)
		return false;

	for(int i=0; i<num; i++){
		array[0] = (void*)"id";
		array[1] = (void*)&id;
		array[2] = (void*)"index";
		array[3] = (void*)&i;
		array[4] = (void*)"material";
		array[5] = (void*)&binds[i].material_id;
		array[6] = (void*)"type";
		array[7] = (void*)binds[i].type.c_str();
		array[8] = (void*)"value";
		array[9] = (void*)binds[i].targetValue;
		array[10] = nullptr;
		m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "Preset_SetVRMExpressionMaterialColorBinds", array);
	}
	return true;
}

bool MQMorphManager::Preset_GetVRMExpressionTextureTransformBinds(UINT id, std::vector<VRM_EXPRESSION::TextureTransformBind>& binds)
{
	if(!m_Verified) return false;

	int num = 0;
	void *array[11];
	array[0] = (void*)"id";
	array[1] = (void*)&id;
	array[2] = (void*)"num";
	array[3] = (void*)&num;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "Preset_GetVRMExpressionTextureTransformBinds", array);
	if(ret == 0)
		return false;

	binds.resize(num);
	for(int i=0; i<num; i++){
		array[0] = (void*)"id";
		array[1] = (void*)&id;
		array[2] = (void*)"index";
		array[3] = (void*)&i;
		array[4] = (void*)"material";
		array[5] = (void*)&binds[i].material_id;
		array[6] = (void*)"scale";
		array[7] = (void*)&binds[i].scale;
		array[8] = (void*)"offset";
		array[9] = (void*)&binds[i].offset;
		array[10] = nullptr;
		m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "Preset_GetVRMExpressionTextureTransformBinds", array);
	}
	return true;
}

bool MQMorphManager::Preset_SetVRMExpressionTextureTransformBinds(UINT id, const std::vector<VRM_EXPRESSION::TextureTransformBind>& binds)
{
	if(!m_Verified) return false;

	int num = (int)binds.size();
	void *array[11];
	array[0] = (void*)"id";
	array[1] = (void*)&id;
	array[2] = (void*)"num";
	array[3] = (void*)&num;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "Preset_SetVRMExpressionTextureTransformBinds", array);
	if(ret == 0)
		return false;

	for(int i=0; i<num; i++){
		array[0] = (void*)"id";
		array[1] = (void*)&id;
		array[2] = (void*)"index";
		array[3] = (void*)&i;
		array[4] = (void*)"material";
		array[5] = (void*)&binds[i].material_id;
		array[6] = (void*)"scale";
		array[7] = (void*)&binds[i].scale;
		array[8] = (void*)"offset";
		array[9] = (void*)&binds[i].offset;
		array[10] = nullptr;
		m_Plugin->SendUserMessage(m_Doc, morph_plugin_product, morph_plugin_id, "Preset_SetVRMExpressionTextureTransformBinds", array);
	}
	return true;
}

