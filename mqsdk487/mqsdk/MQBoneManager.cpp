//---------------------------------------------------------------------------
//
//   MQBoneManager.cpp      Copyright(C) 1999-2024, tetraface Inc.
//
//		An accessor for bone information
//
//    　ボーン情報のアクセス
//
//---------------------------------------------------------------------------

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "MQBoneManager.h"

static const DWORD bone_plugin_product = 0x56A31D20;
static const DWORD bone_plugin_id = 0x71F282AB;


MQBoneManager::MQBoneManager(MQBasePlugin *plugin, MQDocument doc)
	: m_Plugin(plugin), m_Doc(doc)
{
	int version = 0x4700;

	void *array[3];
	array[0] = (void*)"version";
	array[1] = &version;
	array[2] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "QueryAPIVersion", array);
	m_Verified = (ret == 1);
}

DWORD MQBoneManager::GetProductID()
{
	return bone_plugin_product;
}

DWORD MQBoneManager::GetPluginID()
{
	return bone_plugin_id;
}

void MQBoneManager::BeginImport()
{
	if(!m_Verified) return;

	void *array = NULL;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "Init", array);
}

void MQBoneManager::EndImport()
{
	if(!m_Verified) return;

	void *array = NULL;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "NoDelEndDoc", array);
}

UINT MQBoneManager::AddBone(const ADD_BONE_PARAM& param)
{
	if(!m_Verified) return 0;

	const wchar_t *name = param.name.c_str();

	void *array[11];
	array[0] = (void*)"pos";
	array[1] = (void*)&param.pos;
	array[2] = (void*)"name";
	array[3] = (void*)&name;
	array[4] = (void*)"parent";
	array[5] = (void*)&param.parent_id;
	array[6] = (void*)"ikchain";
	array[7] = (void*)&param.ik_chain;
	array[8] = (void*)"dummy";
	array[9] = (void*)&param.is_dummy;
	array[10] = nullptr;
	UINT bone_id = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "AddBone", array);
	return bone_id;
}

bool MQBoneManager::DeleteBone(UINT bone_id)
{
	if(!m_Verified) return false;

	void *array[3];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "DeleteBone", array);
	return (ret != 0);
}

int MQBoneManager::GetBoneNum()
{
	if(!m_Verified) return 0;

	int bone_num = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "QueryBoneNum", nullptr);
	return bone_num;
}

int MQBoneManager::EnumBoneID(std::vector<UINT>& bone_id_array)
{
	if(!m_Verified) return 0;

	int num = GetBoneNum();
	bone_id_array.resize(num);
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "EnumBoneID", bone_id_array.data());
	return num;
}

int MQBoneManager::EnumSelectedBoneID(std::vector<UINT>& bone_id_array)
{
	if(!m_Verified) return 0;

	int num = GetBoneNum();
	bone_id_array.resize(num);
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "EnumSelectedBoneID", bone_id_array.data());
	return num;
}

void MQBoneManager::Update()
{
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "Update", nullptr);
}

bool MQBoneManager::GetName(UINT bone_id, std::wstring& name)
{
	if(!m_Verified) return false;

	name.clear();
	const wchar_t *p = nullptr;
	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"name";
	array[3] = &p;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	if(!ret)
		return false;
	if(p != nullptr)
		name = std::wstring(p);
	return true;
}

bool MQBoneManager::GetParent(UINT bone_id, UINT& parent_id)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"parent";
	array[3] = &parent_id;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetChildNum(UINT bone_id, int& child_num)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"child_num";
	array[3] = &child_num;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetChildren(UINT bone_id, std::vector<UINT>& children)
{
	if(!m_Verified) return false;

	children.clear();

	int child_num = 0;
	if(!GetChildNum(bone_id, child_num))
		return false;

	children.resize(child_num);

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"children";
	array[3] = children.data();
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	if(!ret)
		return false;
	return true;
}

bool MQBoneManager::GetBasePos(UINT bone_id, MQPoint& pos)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"org_pos";
	array[3] = &pos;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetBaseMatrix(UINT bone_id, MQMatrix& matrix)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"base_matrix";
	array[3] = matrix.t;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetUpVector(UINT bone_id, MQMatrix& matrix)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"upvector_matrix";
	array[3] = matrix.t;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetForwardAxis(UINT bone_id, int& axis)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"forward_axis";
	array[3] = &axis;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetDeformPos(UINT bone_id, MQPoint& pos)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"def_pos";
	array[3] = &pos;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetDeformMatrix(UINT bone_id, MQMatrix& matrix)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"matrix";
	array[3] = matrix.t;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetDeformScale(UINT bone_id, MQPoint& scale)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"scale";
	array[3] = &scale;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetDeformRotate(UINT bone_id, MQAngle& angle)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"rotate";
	array[3] = &angle;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetDeformTranslate(UINT bone_id, MQPoint& translate)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"translate";
	array[3] = &translate;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetRotationMatrix(UINT bone_id, MQMatrix& matrix)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"rotate_matrix";
	array[3] = matrix.t;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetAngleMin(UINT bone_id, MQAngle& angle)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"angle_min";
	array[3] = &angle;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetAngleMax(UINT bone_id, MQAngle& angle)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"angle_max";
	array[3] = &angle;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetIKChain(UINT bone_id, int& chain)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"ikchain";
	array[3] = &chain;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetVisible(UINT bone_id, bool& visible)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"visible";
	array[3] = &visible;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetLock(UINT bone_id, bool& lock)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"lock";
	array[3] = &lock;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetDummy(UINT bone_id, bool& dummy)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"dummy";
	array[3] = &dummy;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetMovable(UINT bone_id, bool& movable)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"movable";
	array[3] = &movable;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

void MQBoneManager::SetName(UINT bone_id, const wchar_t *name)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"name";
	array[3] = (void*)&name;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager:: SetParent(UINT bone_id, UINT parent_id)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"parent";
	array[3] = &parent_id;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetBasePos(UINT bone_id, const MQPoint& pos)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"org_pos";
	array[3] = (void*)&pos;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetUpVector(UINT bone_id, const MQMatrix matrix)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"upvector_matrix";
	array[3] = (void*)matrix.t;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetForwardAxis(UINT bone_id, int axis)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"forward_axis";
	array[3] = &axis;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetDeformScale(UINT bone_id, const MQPoint& scale)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"scale";
	array[3] = (void*)&scale;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetDeformRotate(UINT bone_id, const MQAngle& angle)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"rotate";
	array[3] = (void*)&angle;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetDeformTranslate(UINT bone_id, const MQPoint& translate)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"translate";
	array[3] = (void*)&translate;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetAngleMin(UINT bone_id, const MQAngle& angle)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"angle_min";
	array[3] = (void*)&angle;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetAngleMax(UINT bone_id, const MQAngle& angle)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"angle_max";
	array[3] = (void*)&angle;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetIKChain(UINT bone_id, int chain)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"ikchain";
	array[3] = (void*)&chain;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetVisible(UINT bone_id, bool visible)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"visible";
	array[3] = (void*)&visible;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetLock(UINT bone_id, bool lock)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"lock";
	array[3] = (void*)&lock;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetDummy(UINT bone_id, bool dummy)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"dummy";
	array[3] = (void*)&dummy;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}
/*
void MQBoneManager::SetTipBone(UINT bone_id, UINT tip_bone_id)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"tip_bone";
	array[3] = &tip_bone_id;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}
*/
void MQBoneManager::SetMovable(UINT bone_id, bool movable)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"movable";
	array[3] = &movable;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}
/*
bool MQBoneManager::GetTipBone(UINT bone_id, UINT& tip_bone_id)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"tip_bone";
	array[3] = &tip_bone_id;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}
*/
bool MQBoneManager::GetIKName(UINT bone_id, std::wstring& ik_name, std::wstring& ik_tip_name)
{
	if(!m_Verified) return false;

	ik_name.clear();
	ik_tip_name.clear();
	const wchar_t *name = nullptr;
	const wchar_t *tip_name = nullptr;
	void *array[7];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"name";
	array[3] = &name;
	array[4] = (void*)"tip_name";
	array[5] = &tip_name;
	array[6] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetIKName", array);
	if(!ret)
		return false;
	if(name != nullptr)
		ik_name = std::wstring(name);
	if(tip_name != nullptr)
		ik_tip_name = std::wstring(tip_name);
	return true;
}

bool MQBoneManager::GetIKParent(UINT bone_id, UINT& ik_parent_bone_id, bool& isIK)
{
	if(!m_Verified) return false;

	void *array[7];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"parent";
	array[3] = &ik_parent_bone_id;
	array[4] = (void*)"isIK";
	array[5] = &isIK;
	array[6] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetIKParent", array);
	return (ret != 0);
}

bool MQBoneManager::GetLink(UINT bone_id, LINK_PARAM& param)
{
	if(!m_Verified) return false;

	void *array[11];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"link";
	array[3] = &param.link_bone_id;
	array[4] = (void*)"rot";
	array[5] = &param.rotate;
	array[6] = (void*)"move";
	array[7] = &param.move;
	array[8] = (void*)"scale";
	array[9] = &param.scale;
	array[10] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetLink", array);
	return (ret != 0);
}

bool MQBoneManager::GetGroupID(UINT bone_id, UINT& group_id)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"group";
	array[3] = &group_id;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetGroupID", array);
	return (ret != 0);
}

void MQBoneManager::SetIKName(UINT bone_id, const wchar_t *ik_name, const wchar_t *ik_tip_name)
{
	if(!m_Verified) return;

	void *array[7];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"name";
	array[3] = &ik_name;
	array[4] = (void*)"tip_name";
	array[5] = &ik_tip_name;
	array[6] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetIKName", array);
}

void MQBoneManager::SetIKParent(UINT bone_id, UINT ik_parent_bone_id, bool isIK)
{
	if(!m_Verified) return;

	void *array[7];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"parent";
	array[3] = &ik_parent_bone_id;
	array[4] = (void*)"isIK";
	array[5] = &isIK;
	array[6] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetIKParent", array);
}

void MQBoneManager::SetLink(UINT bone_id, const LINK_PARAM& param)
{
	if(!m_Verified) return;

	void *array[11];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"link";
	array[3] = (void*)&param.link_bone_id;
	array[4] = (void*)"rot";
	array[5] = (void*)&param.rotate;
	array[6] = (void*)"move";
	array[7] = (void*)&param.move;
	array[8] = (void*)"scale";
	array[9] = (void*)&param.scale;
	array[10] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetLink", array);
}

MQBoneManager::OPERATION_MODE MQBoneManager::GetOperationMode()
{
	if(!m_Verified) return OPERATION_MODE::DEFAULT;

	const char *mode = nullptr;
	void *array[3];
	array[0] = (void*)"mode";
	array[1] = (void*)&mode;
	array[2] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetOperationMode", array);
	if(ret == 0 || mode == nullptr)
		return OPERATION_MODE::DEFAULT;
	if(strcmp(mode, "default") == 0) return OPERATION_MODE::DEFAULT;
	if(strcmp(mode, "pmd") == 0) return OPERATION_MODE::PMD;
	if(strcmp(mode, "vrm") == 0) return OPERATION_MODE::VRM;
	if(strcmp(mode, "vrm0x") == 0) return OPERATION_MODE::VRM0x;
	return OPERATION_MODE::DEFAULT;
}

bool MQBoneManager::SetOperationMode(OPERATION_MODE mode)
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

	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetOperationMode", array);
	return (ret != 0);
}

int MQBoneManager::EnumGroups(std::vector<UINT>& group_ids)
{
	if(!m_Verified) return 0;

	group_ids.clear();
	int group_num = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "QueryGroupNum", nullptr);
	if(group_num > 0){
		group_ids.resize(group_num);
		m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "EnumGroupID", group_ids.data());
	}
	return group_num;
}

UINT MQBoneManager::AddGroup(const wchar_t *name)
{
	if(!m_Verified) return 0;

	void *array[3];
	array[0] = (void*)"name";
	array[1] = (void*)name;
	array[2] = nullptr;
	UINT group_id = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "AddGroup", array);
	return group_id;
}

bool MQBoneManager::GetGroupName(UINT group_id, std::wstring& name)
{
	if(!m_Verified) return false;

	name.clear();

	const wchar_t *p = nullptr;
	void *array[5];
	array[0] = (void*)"group";
	array[1] = &group_id;
	array[2] = (void*)"name";
	array[3] = (void*)&p;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetGroupName", array);
	if(!ret)
		return false;
	if(p != nullptr)
		name = std::wstring(p);
	return true;
}

void MQBoneManager::SetGroupID(UINT bone_id, UINT group_id)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"group";
	array[3] = &group_id;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetGroup", array);
}

const char *MQBoneManager::VRM_META::avatarPermission_str[3] = {"onlyAuthor","onlySeparatelyLicensedPerson","everyone"};
const char *MQBoneManager::VRM_META::commercialUsage_str[3] = {"personalNonProfit","personalProfit","corporation"};
const char *MQBoneManager::VRM_META::creditNotation_str[2] = {"required","unnecessary"};
const char *MQBoneManager::VRM_META::modification_str[3] = {"prohibited","allowModification","allowModificationRedistribution"};
const char *MQBoneManager::VRM0x_META::allowedUserName_str[3] = {"OnlyAuthor","ExplicitlyLicensedPerson","Everyone"};
const char *MQBoneManager::VRM0x_META::licenseName_str[9] = {"Redistribution_Prohibited","CC0","CC_BY","CC_BY_NC","CC_BY_SA","CC_BY_NC_SA","CC_BY_ND","CC_BY_NC_ND","Other"};

bool MQBoneManager::VRMMeta_Get(VRM_META& meta)
{
	if(!m_Verified) return false;

	const wchar_t *name = nullptr;
	const wchar_t *version = nullptr;
	const wchar_t *authors = nullptr;
	const wchar_t *licenseUrl = nullptr;
	const wchar_t *copyrightInformation = nullptr;
	const wchar_t *contactInformation = nullptr;
	const wchar_t *references = nullptr;
	const wchar_t *thirdPartyLicenses = nullptr;
	const wchar_t *thumbnailImage = nullptr;
	const wchar_t *otherLicenseUrl = nullptr;
	void *array[39];
	array[0] = (void*)"name";
	array[1] = (void*)&name;
	array[2] = (void*)"version";
	array[3] = (void*)&version;
	array[4] = (void*)"authors";
	array[5] = (void*)&authors;
	array[6] = (void*)"licenseUrl";
	array[7] = (void*)&licenseUrl;
	array[8] = (void*)"copyrightInformation";
	array[9] = (void*)&copyrightInformation;
	array[10] = (void*)"contactInformation";
	array[11] = (void*)&contactInformation;
	array[12] = (void*)"references";
	array[13] = (void*)&references;
	array[14] = (void*)"avatarPermission";
	array[15] = (void*)&meta.avatarPermission;
	array[16] = (void*)"allowExcessivelyViolentUsage";
	array[17] = (void*)&meta.allowExcessivelyViolentUsage;
	array[18] = (void*)"allowExcessivelySexualUsage";
	array[19] = (void*)&meta.allowExcessivelySexualUsage;
	array[20] = (void*)"allowPoliticalOrReligiousUsage";
	array[21] = (void*)&meta.allowPoliticalOrReligiousUsage;
	array[22] = (void*)"allowAntisocialOrHateUsage";
	array[23] = (void*)&meta.allowAntisocialOrHateUsage;
	array[24] = (void*)"commercialUsage";
	array[25] = (void*)&meta.commercialUsage;
	array[26] = (void*)"creditNotation";
	array[27] = (void*)&meta.creditNotation;
	array[28] = (void*)"allowRedistribution";
	array[29] = (void*)&meta.allowRedistribution;
	array[30] = (void*)"modification";
	array[31] = (void*)&meta.modification;
	array[32] = (void*)"otherLicenseUrl";
	array[33] = (void*)&otherLicenseUrl;
	array[34] = (void*)"thirdPartyLicenses";
	array[35] = (void*)&thirdPartyLicenses;
	array[36] = (void*)"thumbnailImage";
	array[37] = (void*)&thumbnailImage;
	array[38] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMMeta_Get", array);
	if(!ret)
		return false;

	auto set_text = [](std::wstring& s, const wchar_t *v){
		if(v && v[0]!=L'\0') s = std::wstring(v);
		else s.clear();
	};
	set_text(meta.name, name);
	set_text(meta.version, version);
	set_text(meta.authors, authors);
	set_text(meta.licenseUrl, licenseUrl);
	set_text(meta.copyrightInformation, copyrightInformation);
	set_text(meta.contactInformation, contactInformation);
	set_text(meta.references, references);
	set_text(meta.thirdPartyLicenses, thirdPartyLicenses);
	set_text(meta.otherLicenseUrl, otherLicenseUrl);
	set_text(meta.thumbnailImage, thumbnailImage);
	return true;
}

bool MQBoneManager::VRMMeta_Set(const VRM_META& meta)
{
	if(!m_Verified) return false;

	void *array[39];
	array[0] = (void*)"name";
	array[1] = (void*)meta.name.c_str();
	array[2] = (void*)"version";
	array[3] = (void*)meta.version.c_str();
	array[4] = (void*)"authors";
	array[5] = (void*)meta.authors.c_str();
	array[6] = (void*)"licenseUrl";
	array[7] = (void*)meta.licenseUrl.c_str();
	array[8] = (void*)"copyrightInformation";
	array[9] = (void*)meta.copyrightInformation.c_str();
	array[10] = (void*)"contactInformation";
	array[11] = (void*)meta.contactInformation.c_str();
	array[12] = (void*)"references";
	array[13] = (void*)meta.references.c_str();
	array[14] = (void*)"avatarPermission";
	array[15] = (void*)&meta.avatarPermission;
	array[16] = (void*)"allowExcessivelyViolentUsage";
	array[17] = (void*)&meta.allowExcessivelyViolentUsage;
	array[18] = (void*)"allowExcessivelySexualUsage";
	array[19] = (void*)&meta.allowExcessivelySexualUsage;
	array[20] = (void*)"allowPoliticalOrReligiousUsage";
	array[21] = (void*)&meta.allowPoliticalOrReligiousUsage;
	array[22] = (void*)"allowAntisocialOrHateUsage";
	array[23] = (void*)&meta.allowAntisocialOrHateUsage;
	array[24] = (void*)"commercialUsage";
	array[25] = (void*)&meta.commercialUsage;
	array[26] = (void*)"creditNotation";
	array[27] = (void*)&meta.creditNotation;
	array[28] = (void*)"allowRedistribution";
	array[29] = (void*)&meta.allowRedistribution;
	array[30] = (void*)"modification";
	array[31] = (void*)&meta.modification;
	array[32] = (void*)"otherLicenseUrl";
	array[33] = (void*)meta.otherLicenseUrl.c_str();
	array[34] = (void*)"thirdPartyLicenses";
	array[35] = (void*)meta.thirdPartyLicenses.c_str();
	array[36] = (void*)"thumbnailImage";
	array[37] = (void*)meta.thumbnailImage.c_str();
	array[38] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMMeta_Set", array);
	if(!ret)
		return false;
	return true;
}

bool MQBoneManager::VRM0xMeta_Get(VRM0x_META& meta)
{
	if(!m_Verified) return false;

	const wchar_t *title = nullptr;
	const wchar_t *version = nullptr;
	const wchar_t *author = nullptr;
	const wchar_t *contactInformation = nullptr;
	const wchar_t *reference = nullptr;
	const wchar_t *otherPermissionUrl = nullptr;
	const wchar_t *otherLicenseUrl = nullptr;
	const wchar_t *texture = nullptr;
	void *array[27];
	array[0] = (void*)"title";
	array[1] = (void*)&title;
	array[2] = (void*)"version";
	array[3] = (void*)&version;
	array[4] = (void*)"author";
	array[5] = (void*)&author;
	array[6] = (void*)"contactInformation";
	array[7] = (void*)&contactInformation;
	array[8] = (void*)"reference";
	array[9] = (void*)&reference;
	array[10] = (void*)"allowedUserName";
	array[11] = (void*)&meta.allowedUserName;
	array[12] = (void*)"violentUsage";
	array[13] = (void*)&meta.violentUsage;
	array[14] = (void*)"sexualUsage";
	array[15] = (void*)&meta.sexualUsage;
	array[16] = (void*)"commercialUsage";
	array[17] = (void*)&meta.commercialUsage;
	array[18] = (void*)"otherPermissionUrl";
	array[19] = (void*)&otherPermissionUrl;
	array[20] = (void*)"licenseName";
	array[21] = (void*)&meta.licenseName;
	array[22] = (void*)"otherLicenseUrl";
	array[23] = (void*)&otherLicenseUrl;
	array[24] = (void*)"texture";
	array[25] = (void*)&texture;
	array[26] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRM0xMeta_Get", array);
	if(!ret)
		return false;

	auto set_text = [](std::wstring& s, const wchar_t *v){
		if(v && v[0]!=L'\0') s = std::wstring(v);
		else s.clear();
	};
	set_text(meta.title, title);
	set_text(meta.version, version);
	set_text(meta.author, author);
	set_text(meta.contactInformation, contactInformation);
	set_text(meta.reference, reference);
	set_text(meta.otherPermissionUrl, otherPermissionUrl);
	set_text(meta.otherLicenseUrl, otherLicenseUrl);
	set_text(meta.texture, texture);
	return true;
}

bool MQBoneManager::VRM0xMeta_Set(const VRM0x_META& meta)
{
	if(!m_Verified) return false;

	void *array[27];
	array[0] = (void*)"title";
	array[1] = (void*)meta.title.c_str();
	array[2] = (void*)"version";
	array[3] = (void*)meta.version.c_str();
	array[4] = (void*)"author";
	array[5] = (void*)meta.author.c_str();
	array[6] = (void*)"contactInformation";
	array[7] = (void*)meta.contactInformation.c_str();
	array[8] = (void*)"reference";
	array[9] = (void*)meta.reference.c_str();
	array[10] = (void*)"allowedUserName";
	array[11] = (void*)&meta.allowedUserName;
	array[12] = (void*)"violentUsage";
	array[13] = (void*)&meta.violentUsage;
	array[14] = (void*)"sexualUsage";
	array[15] = (void*)&meta.sexualUsage;
	array[16] = (void*)"commercialUsage";
	array[17] = (void*)&meta.commercialUsage;
	array[18] = (void*)"otherPermissionUrl";
	array[19] = (void*)meta.otherPermissionUrl.c_str();
	array[20] = (void*)"licenseName";
	array[21] = (void*)&meta.licenseName;
	array[22] = (void*)"otherLicenseUrl";
	array[23] = (void*)meta.otherLicenseUrl.c_str();
	array[24] = (void*)"texture";
	array[25] = (void*)meta.texture.c_str();
	array[26] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRM0xMeta_Set", array);
	if(!ret)
		return false;
	return true;
}

bool MQBoneManager::VRM0x_GetFlipZ()
{
	if(!m_Verified) return false;

	bool flag = false;
	void *array[3];
	array[0] = (void*)"flip";
	array[1] = (void*)&flag;
	array[2] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRM0x_GetFlipZ", array);
	return flag;
}

void MQBoneManager::VRM0x_SetFlipZ(bool flag)
{
	if(!m_Verified) return;

	void *array[3];
	array[0] = (void*)"flip";
	array[1] = (void*)&flag;
	array[2] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRM0x_SetFlipZ", array);
}

bool MQBoneManager::VRMHumanoid_Get(UINT bone_id, std::wstring& humanoid)
{
	if(!m_Verified) return false;

	humanoid.clear();
	const wchar_t *p = nullptr;
	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"vrm_humanoid";
	array[3] = &p;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	if(!ret)
		return false;
	if(p != nullptr)
		humanoid = std::wstring(p);
	return true;
}

void MQBoneManager::VRMHumanoid_Set(UINT bone_id, const wchar_t *humanoid)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"vrm_humanoid";
	array[3] = (void*)&humanoid;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

bool MQBoneManager::VRMHumanoid_GetTemplate(std::vector<VRM_HUMANOID_TEMPLATE>& humanoid, OPERATION_MODE mode, bool acquire_bone_id)
{
	if(!m_Verified) return false;

	humanoid.clear();
	int num = 0;
	void *array[13];
	array[0] = (void*)"mode";
	switch(mode){
	case OPERATION_MODE::VRM: array[1] = (void*)"vrm"; break;
	case OPERATION_MODE::VRM0x: array[1] = (void*)"vrm0x"; break;
	default: array[1] = (void*)"default"; break;
	}
	array[2] = (void*)"num";
	array[3] = &num;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMHumanoidTemplate", array);
	if(!ret)
		return false;
	for(int i=0; i<num; i++){
		const wchar_t *name = nullptr, *parent = nullptr;
		bool required = false, needparent = false;
		array[2] = (void*)"index";
		array[3] = &i;
		array[4] = (void*)"name";
		array[5] = &name;
		array[6] = (void*)"parent";
		array[7] = &parent;
		array[8] = (void*)"required";
		array[9] = &required;
		array[10] = (void*)"needparent";
		array[11] = &needparent;
		array[12] = nullptr;
		int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMHumanoidTemplate", array);
		if(!ret)
			return false;
		humanoid.emplace_back();
		if(name)
			humanoid.back().name = std::wstring(name);
		if(parent)
			humanoid.back().parent = std::wstring(parent);
		humanoid.back().required = required;
		humanoid.back().need_parent = needparent;
	}

	if(acquire_bone_id && num > 0){
		std::vector<UINT> bone_ids(num);
		array[0] = (void*)"bone_ids";
		array[1] = bone_ids.data();
		array[2] = nullptr;
		int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMHumanoidTemplate", array);
		if(!ret)
			return false;

		for(int i=0; i<num; i++)
			humanoid[i].bone_id = bone_ids[i];
	}
	return true;
}

bool MQBoneManager::VRMLookAt_Get(VRM_LOOKAT_PARAM& param)
{
	if(!m_Verified) return false;

	int typeval = -1;
	void *array[13];
	array[0] = (void*)"type";
	array[1] = (void*)&typeval;
	array[2] = (void*)"offset";
	array[3] = &param.offsetFromHeadBone;
	array[4] = (void*)"hin";
	array[5] = (void*)&param.rangeMapHorizontalInner;
	array[6] = (void*)"hout";
	array[7] = (void*)&param.rangeMapHorizontalOuter;
	array[8] = (void*)"vdown";
	array[9] = (void*)&param.rangeMapVerticalDown;
	array[10] = (void*)"vup";
	array[11] = (void*)&param.rangeMapVerticalUp;
	array[12] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMLookAt_Get", array);
	if(ret == 0)
		return false;
	switch(typeval){
	case 0: param.type = VRM_LOOKAT_PARAM::Type::kBone; break;
	case 1: param.type = VRM_LOOKAT_PARAM::Type::kExpression; break;
	}
	return (ret != 0);
}

bool MQBoneManager::VRMLookAt_Set(const VRM_LOOKAT_PARAM& param)
{
	if(!m_Verified) return false;

	int typeval = -1;
	if(param.type == VRM_LOOKAT_PARAM::Type::kBone) typeval = 0;
	else if(param.type == VRM_LOOKAT_PARAM::Type::kExpression) typeval = 1;

	void *array[13];
	array[0] = (void*)"type";
	array[1] = (void*)&typeval;
	array[2] = (void*)"offset";
	array[3] = (void*)&param.offsetFromHeadBone;
	array[4] = (void*)"hin";
	array[5] = (void*)&param.rangeMapHorizontalInner;
	array[6] = (void*)"hout";
	array[7] = (void*)&param.rangeMapHorizontalOuter;
	array[8] = (void*)"vdown";
	array[9] = (void*)&param.rangeMapVerticalDown;
	array[10] = (void*)"vup";
	array[11] = (void*)&param.rangeMapVerticalUp;
	array[12] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMLookAt_Set", array);
	return (ret != 0);
}

UINT MQBoneManager::VRMFirstPerson_NewMeshAnnotation(UINT obj_id, const VRM_MESH_ANNOTATION& anno)
{
	void *array[5];
	array[0] = (void*)"obj";
	array[1] = (void*)&obj_id;
	array[2] = (void*)"type";
	array[3] = (void*)&anno.type;
	array[4] = nullptr;
	UINT id = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMFirstPerson_NewMeshAnnotation", array);
	return id;
}

bool MQBoneManager::VRMFirstPerson_DeleteMeshAnnotation(UINT annotation_id)
{
	void *array[3];
	array[0] = (void*)"id";
	array[1] = (void*)&annotation_id;
	array[2] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMFirstPerson_DeleteMeshAnnotation", array);
	return (ret != 0);
}

bool MQBoneManager::VRMFirstPerson_Enumerate(std::vector<UINT>& annotation_ids)
{
	annotation_ids.clear();
	int num = 0;
	void *array[3];
	array[0] = (void*)"num";
	array[1] = (void*)&num;
	array[2] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMFirstPerson_Enumerate", array);
	if(ret){
		annotation_ids.resize(num);
		if(num > 0){
			array[0] = (void*)"ids";
			array[1] = (void*)annotation_ids.data();
			ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMFirstPerson_Enumerate", array);
		}
	}
	return ret != 0;
}

bool MQBoneManager::VRMFirstPerson_GetMeshAnnotation(UINT annotation_id, UINT& obj_id, VRM_MESH_ANNOTATION& anno)
{
	void *array[7];
	array[0] = (void*)"id";
	array[1] = (void*)&annotation_id;
	array[2] = (void*)"obj";
	array[3] = (void*)&obj_id;
	array[4] = (void*)"type";
	array[5] = (void*)&anno.type;
	array[6] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMFirstPerson_GetMeshAnnotation", array);
	return (ret != 0);
}

bool MQBoneManager::VRMFirstPerson_SetMeshAnnotation(UINT annotation_id, const VRM_MESH_ANNOTATION& anno)
{
	void *array[5];
	array[0] = (void*)"id";
	array[1] = (void*)&annotation_id;
	array[2] = (void*)"type";
	array[3] = (void*)&anno.type;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMFirstPerson_SetMeshAnnotation", array);
	return (ret != 0);
}

UINT MQBoneManager::VRMCollider_New(UINT bone_id)
{
	void *array[3];
	array[0] = (void*)"bone";
	array[1] = (void*)&bone_id;
	array[2] = nullptr;
	UINT id = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMCollider_New", array);
	return id;
}

bool MQBoneManager::VRMCollider_Delete(UINT collider_id)
{
	void *array[3];
	array[0] = (void*)"collider";
	array[1] = (void*)&collider_id;
	array[2] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMCollider_Delete", array);
	return ret != 0;
}

bool MQBoneManager::VRMCollider_Enumerate(std::vector<UINT>& collider_ids)
{
	collider_ids.clear();
	int num = 0;
	void *array[3];
	array[0] = (void*)"num";
	array[1] = (void*)&num;
	array[2] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMCollider_Enumerate", array);
	if(ret){
		collider_ids.resize(num);
		if(num > 0){
			array[0] = (void*)"colliders";
			array[1] = (void*)collider_ids.data();
			ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMCollider_Enumerate", array);
		}
	}
	return ret != 0;
}

bool MQBoneManager::VRMCollider_GetParam(UINT collider_id, UINT& bone_id, VRM_COLLIDER_PARAM& param)
{
	void *array[13];
	array[0] = (void*)"collider";
	array[1] = (void*)&collider_id;
	array[2] = (void*)"shape";
	array[3] = (void*)&param.shape;
	array[4] = (void*)"offset";
	array[5] = (void*)&param.offset;
	array[6] = (void*)"radius";
	array[7] = (void*)&param.radius;
	array[8] = (void*)"tail";
	array[9] = (void*)&param.tail;
	array[10] = (void*)"bone";
	array[11] = (void*)&bone_id;
	array[12] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMCollider_GetParam", array);
	return ret != 0;
}

bool MQBoneManager::VRMCollider_SetParam(UINT collider_id, const VRM_COLLIDER_PARAM& param)
{
	void *array[11];
	array[0] = (void*)"collider";
	array[1] = (void*)&collider_id;
	array[2] = (void*)"shape";
	array[3] = (void*)&param.shape;
	array[4] = (void*)"offset";
	array[5] = (void*)&param.offset;
	array[6] = (void*)"radius";
	array[7] = (void*)&param.radius;
	array[8] = (void*)"tail";
	array[9] = (void*)&param.tail;
	array[10] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMCollider_SetParam", array);
	return ret != 0;
}

UINT MQBoneManager::VRMColliderGroup_New(const std::wstring& name, std::vector<UINT> collders)
{
	void *array[5];
	array[0] = (void*)"name";
	array[1] = (void*)name.c_str();
	if(collders.empty())
		array[2] = nullptr;
	else{
		collders.push_back(0);
		array[2] = (void*)"colliders";
		array[3] = (void*)collders.data();
		array[4] = nullptr;
	}
	UINT id = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMColliderGroup_New", array);
	return id;
}

bool MQBoneManager::VRMColliderGroup_Delete(UINT group_id)
{
	void *array[3];
	array[0] = (void*)"group";
	array[1] = (void*)&group_id;
	array[2] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMColliderGroup_Delete", array);
	return ret != 0;
}

bool MQBoneManager::VRMColliderGroup_Enumerate(std::vector<UINT>& group_ids)
{
	group_ids.clear();
	int num = 0;
	void *array[3];
	array[0] = (void*)"num";
	array[1] = (void*)&num;
	array[2] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMColliderGroup_Enumerate", array);
	if(ret){
		group_ids.resize(num);
		if(num > 0){
			array[0] = (void*)"groups";
			array[1] = (void*)group_ids.data();
			ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMColliderGroup_Enumerate", array);
		}
	}
	return ret != 0;
}

bool MQBoneManager::VRMColliderGroup_GetName(UINT group_id, std::wstring& name)
{
	name.clear();
	const wchar_t *p = nullptr;
	void *array[5];
	array[0] = (void*)"group";
	array[1] = (void*)&group_id;
	array[2] = (void*)"name";
	array[3] = &p;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMColliderGroup_GetName", array);
	if(!ret)
		return false;
	if(p != nullptr)
		name = std::wstring(p);
	return true;
}

bool MQBoneManager::VRMColliderGroup_SetName(UINT group_id, const std::wstring& name)
{
	void *array[5];
	array[0] = (void*)"group";
	array[1] = (void*)&group_id;
	array[2] = (void*)"name";
	array[3] = (void*)name.c_str();
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMColliderGroup_SetName", array);
	return ret != 0;
}

bool MQBoneManager::VRMColliderGroup_GetColliders(UINT group_id, std::vector<UINT>& collider_ids)
{
	int num = 0;
	void *array[5];
	array[0] = (void*)"group";
	array[1] = (void*)&group_id;
	array[2] = (void*)"num";
	array[3] = (void*)&num;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMColliderGroup_GetColliders", array);
	if(ret){
		collider_ids.resize(num);
		if(num > 0){
			array[2] = (void*)"colliders";
			array[3] = (void*)collider_ids.data();
			ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMColliderGroup_GetColliders", array);
		}
	}
	return ret != 0;
}

bool MQBoneManager::VRMColliderGroup_SetColliders(UINT group_id, std::vector<UINT> collider_ids)
{
	if(!collider_ids.empty())
		collider_ids.push_back(0);
	void *array[5];
	array[0] = (void*)"group";
	array[1] = (void*)&group_id;
	array[2] = (void*)"colliders";
	array[3] = (void*)(!collider_ids.empty() ? collider_ids.data() : nullptr);
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMColliderGroup_SetColliders", array);
	return ret != 0;
}

UINT MQBoneManager::VRMSpringBone_New(const std::wstring& name)
{
	void *array[3];
	array[0] = (void*)"name";
	array[1] = (void*)name.c_str();
	array[2] = nullptr;
	UINT id = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMSpringBone_New", array);
	return id;
}

bool MQBoneManager::VRMSpringBone_Delete(UINT spring_id)
{
	void *array[3];
	array[0] = (void*)"spring";
	array[1] = (void*)&spring_id;
	array[2] = nullptr;
	UINT id = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMSpringBone_Delete", array);
	return id;
}

bool MQBoneManager::VRMSpringBone_Enumerate(std::vector<UINT>& spring_ids)
{
	spring_ids.clear();
	int num = 0;
	void *array[3];
	array[0] = (void*)"num";
	array[1] = (void*)&num;
	array[2] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMSpringBone_Enumerate", array);
	if(ret){
		spring_ids.resize(num);
		if(num > 0){
			array[0] = (void*)"springs";
			array[1] = (void*)spring_ids.data();
			ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMSpringBone_Enumerate", array);
		}
	}
	return ret != 0;
}

bool MQBoneManager::VRMSpringBone_GetName(UINT spring_id, std::wstring& name)
{
	name.clear();
	const wchar_t *p = nullptr;
	void *array[5];
	array[0] = (void*)"spring";
	array[1] = (void*)&spring_id;
	array[2] = (void*)"name";
	array[3] = &p;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMSpringBone_GetName", array);
	if(!ret)
		return false;
	if(p != nullptr)
		name = std::wstring(p);
	return true;
}

bool MQBoneManager::VRMSpringBone_SetName(UINT spring_id, const std::wstring& name)
{
	void *array[5];
	array[0] = (void*)"spring";
	array[1] = (void*)&spring_id;
	array[2] = (void*)"name";
	array[3] = (void*)name.c_str();
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMSpringBone_SetName", array);
	return ret != 0;
}

int MQBoneManager::VRMSpringBone_GetJointNum(UINT spring_id)
{
	int num = 0;
	void *array[5];
	array[0] = (void*)"spring";
	array[1] = (void*)&spring_id;
	array[2] = (void*)"num";
	array[3] = (void*)&num;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMSpringBone_GetJointNum", array);
	return num;
}

bool MQBoneManager::VRMSpringBone_AddJoint(UINT spring_id, UINT bone_id, const VRM_SPRING_JOINT& joint)
{
	void *array[15];
	array[0] = (void*)"spring";
	array[1] = (void*)&spring_id;
	array[2] = (void*)"bone";
	array[3] = (void*)&bone_id;
	array[4] = (void*)"hitRadius";
	array[5] = (void*)&joint.hitRadius;
	array[6] = (void*)"stiffness";
	array[7] = (void*)&joint.stiffness;
	array[8] = (void*)"gravityPower";
	array[9] = (void*)&joint.gravityPower;
	array[10] = (void*)"gravityDir";
	array[11] = (void*)&joint.gravityDir;
	array[12] = (void*)"dragForce";
	array[13] = (void*)&joint.dragForce;
	array[14] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMSpringBone_AddJoint", array);
	return ret != 0;
}

bool MQBoneManager::VRMSpringBone_DeleteJoint(UINT spring_id, int index)
{
	void *array[5];
	array[0] = (void*)"spring";
	array[1] = (void*)&spring_id;
	array[2] = (void*)"index";
	array[3] = (void*)&index;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMSpringBone_DeleteJoint", array);
	return ret != 0;
}

bool MQBoneManager::VRMSpringBone_GetJoint(UINT spring_id, int index, UINT& bone_id, VRM_SPRING_JOINT& joint)
{
	void *array[17];
	array[0] = (void*)"spring";
	array[1] = (void*)&spring_id;
	array[2] = (void*)"index";
	array[3] = (void*)&index;
	array[4] = (void*)"hitRadius";
	array[5] = (void*)&joint.hitRadius;
	array[6] = (void*)"stiffness";
	array[7] = (void*)&joint.stiffness;
	array[8] = (void*)"gravityPower";
	array[9] = (void*)&joint.gravityPower;
	array[10] = (void*)"gravityDir";
	array[11] = (void*)&joint.gravityDir;
	array[12] = (void*)"dragForce";
	array[13] = (void*)&joint.dragForce;
	array[14] = (void*)"bone";
	array[15] = (void*)&bone_id;
	array[16] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMSpringBone_GetJoint", array);
	return ret != 0;
}

bool MQBoneManager::VRMSpringBone_SetJoint(UINT spring_id, int index, const VRM_SPRING_JOINT& joint)
{
	void *array[15];
	array[0] = (void*)"spring";
	array[1] = (void*)&spring_id;
	array[2] = (void*)"index";
	array[3] = (void*)&index;
	array[4] = (void*)"hitRadius";
	array[5] = (void*)&joint.hitRadius;
	array[6] = (void*)"stiffness";
	array[7] = (void*)&joint.stiffness;
	array[8] = (void*)"gravityPower";
	array[9] = (void*)&joint.gravityPower;
	array[10] = (void*)"gravityDir";
	array[11] = (void*)&joint.gravityDir;
	array[12] = (void*)"dragForce";
	array[13] = (void*)&joint.dragForce;
	array[14] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMSpringBone_SetJoint", array);
	return ret != 0;
}

bool MQBoneManager::VRMSpringBone_GetColliderGroups(UINT spring_id, std::vector<UINT>& group_ids)
{
	int num = 0;
	void *array[5];
	array[0] = (void*)"spring";
	array[1] = (void*)&spring_id;
	array[2] = (void*)"num";
	array[3] = (void*)&num;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMSpringBone_GetColliderGroups", array);
	if(ret){
		group_ids.resize(num);
		if(num > 0){
			array[2] = (void*)"groups";
			array[3] = (void*)group_ids.data();
			ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMSpringBone_GetColliderGroups", array);
		}
	}
	return ret != 0;
}

bool MQBoneManager::VRMSpringBone_SetColliderGroups(UINT spring_id, std::vector<UINT> group_ids)
{
	if(!group_ids.empty())
		group_ids.push_back(0);
	void *array[5];
	array[0] = (void*)"spring";
	array[1] = (void*)&spring_id;
	array[2] = (void*)"groups";
	array[3] = (void*)(!group_ids.empty() ? group_ids.data() : nullptr);
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMSpringBone_SetColliderGroups", array);
	return ret != 0;
}

bool MQBoneManager::VRMSpringBone_GetCenterBone(UINT spring_id, UINT& center_bone_id)
{
	void *array[5];
	array[0] = (void*)"spring";
	array[1] = (void*)&spring_id;
	array[2] = (void*)"center";
	array[3] = (void*)&center_bone_id;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMSpringBone_GetCenterBone", array);
	return ret != 0;
}

bool MQBoneManager::VRMSpringBone_SetCenterBone(UINT spring_id, UINT center_bone_id)
{
	void *array[5];
	array[0] = (void*)"spring";
	array[1] = (void*)&spring_id;
	array[2] = (void*)"center";
	array[3] = (void*)&center_bone_id;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMSpringBone_SetCenterBone", array);
	return ret != 0;
}

UINT MQBoneManager::VRMNodeConstraint_New(UINT bone_id)
{
	void *array[3];
	array[0] = (void*)"bone";
	array[1] = (void*)&bone_id;
	array[2] = nullptr;
	UINT id = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMNodeConstraint_New", array);
	return id;
}

bool MQBoneManager::VRMNodeConstraint_Delete(UINT nc_id)
{
	void *array[3];
	array[0] = (void*)"id";
	array[1] = (void*)&nc_id;
	array[2] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMNodeConstraint_Delete", array);
	return (ret != 0);
}

bool MQBoneManager::VRMNodeConstraint_Enumerate(std::vector<UINT>& nc_ids)
{
	nc_ids.clear();
	int num = 0;
	void *array[3];
	array[0] = (void*)"num";
	array[1] = (void*)&num;
	array[2] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMNodeConstraint_Enumerate", array);
	if(ret){
		nc_ids.resize(num);
		if(num > 0){
			array[0] = (void*)"ids";
			array[1] = (void*)nc_ids.data();
			ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMNodeConstraint_Enumerate", array);
		}
	}
	return ret != 0;
}

bool MQBoneManager::VRMNodeConstraint_Get(UINT nc_id, UINT& bone_id, VRM_NODE_CONSTRAINT& param)
{
	const char *type = nullptr;
	void *array[13];
	array[0] = (void*)"id";
	array[1] = (void*)&nc_id;
	array[2] = (void*)"bone";
	array[3] = (void*)&bone_id;
	array[4] = (void*)"source";
	array[5] = (void*)&param.source_bone_id;
	array[6] = (void*)"type";
	array[7] = (void*)&type;
	array[8] = (void*)"axis";
	array[9] = (void*)&param.axis;
	array[10] = (void*)"weight";
	array[11] = (void*)&param.weight;
	array[12] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMNodeConstraint_Get", array);
	if(ret == 0)
		return false;
	if(type != nullptr){
		if(strcmp(type, "roll") == 0)
			param.type = VRM_NODE_CONSTRAINT::Type::Roll;
		else if(strcmp(type, "aim") == 0)
			param.type = VRM_NODE_CONSTRAINT::Type::Aim;
		else if(strcmp(type, "rotation") == 0)
			param.type = VRM_NODE_CONSTRAINT::Type::Rotation;
	}
	return true;
}

bool MQBoneManager::VRMNodeConstraint_Set(UINT nc_id, const VRM_NODE_CONSTRAINT& param)
{
	void *array[11];
	array[0] = (void*)"id";
	array[1] = (void*)&nc_id;
	array[2] = (void*)"source";
	array[3] = (void*)&param.source_bone_id;
	array[4] = (void*)"type";
	switch(param.type){
	case VRM_NODE_CONSTRAINT::Type::Roll: array[5] = (void*)"roll"; break;
	case VRM_NODE_CONSTRAINT::Type::Aim: array[5] = (void*)"aim"; break;
	case VRM_NODE_CONSTRAINT::Type::Rotation: array[5] = (void*)"rotation"; break;
	default: return false;
	}
	array[6] = (void*)"axis";
	array[7] = (void*)&param.axis;
	array[8] = (void*)"weight";
	array[9] = (void*)&param.weight;
	array[10] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "VRMNodeConstraint_Set", array);
	return (ret != 0);
}

int MQBoneManager::GetSkinObjectNum()
{
	if(!m_Verified) return 0;

	int bone_object_num = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "QueryObjectNum", nullptr);
	return bone_object_num;
}

int MQBoneManager::EnumSkinObjectID(std::vector<UINT>& obj_id_array)
{
	if(!m_Verified) return 0;

	int num = GetSkinObjectNum();
	obj_id_array.resize(num);
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "EnumObjectID", obj_id_array.data());
	return num;
}

bool MQBoneManager::AddSkinObject(MQObject obj)
{
	if(!m_Verified) return false;

	void *array[3];
	array[0] = (void*)"object";
	array[1] = &obj;
	array[2] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "AddObject", array);
	return (ret != 0);
}

bool MQBoneManager::DeleteSkinObject(MQObject obj)
{
	if(!m_Verified) return false;

	void *array[3];
	array[0] = (void*)"object";
	array[1] = &obj;
	array[2] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "DeleteObject", array);
	return (ret != 0);
}

int MQBoneManager::GetVertexWeightArray(MQObject obj, UINT vertex_id, int array_num, UINT *bone_ids, float *weights)
{
	if(!m_Verified) return false;

	if(obj == nullptr)
		return false;
	UINT obj_id = obj->GetUniqueID();
	void *array[5];
	array[0] = &obj_id;
	array[1] = &vertex_id;
	array[2] = &array_num;
	array[3] = bone_ids;
	array[4] = weights;
	int weight_num = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetVertexWeight", array);
	return weight_num;
}

bool MQBoneManager::SetVertexWeight(MQObject obj, UINT vertex_id, UINT bone_id, float weight)
{
	if(!m_Verified) return false;

	if(obj == nullptr)
		return false;
	UINT obj_id = obj->GetUniqueID();
	void *array[9];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"object";
	array[3] = &obj_id;
	array[4] = (void*)"vertex";
	array[5] = &vertex_id;
	array[6] = (void*)"weight";
	array[7] = &weight;
	array[8] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetWeight", array);
	return (ret != 0);
}

int MQBoneManager::GetWeightedVertexArray(UINT bone_id, MQObject obj, std::vector<UINT>& vertex_ids, std::vector<float>& weights)
{
	if(!m_Verified) return false;

	if(obj == nullptr)
		return false;
	UINT obj_id = obj->GetUniqueID();
	void *array[4];
	array[0] = &obj_id;
	array[1] = &bone_id;
	array[2] = nullptr;
	array[3] = nullptr;
	int vert_num = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBoneWeight", array);
	if(vert_num > 0){				
		vertex_ids.resize(vert_num);
		weights.resize(vert_num);
		array[2] = vertex_ids.data();
		array[3] = weights.data();
		m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBoneWeight", array);
	}else{
		vertex_ids.clear();
		weights.clear();
	}
	return vert_num;
}

bool MQBoneManager::NormalizeVertexWeight(MQObject obj, UINT vertex_id)
{
	if(!m_Verified) return false;

	if(obj == nullptr)
		return false;
	UINT obj_id = obj->GetUniqueID();
	void *array[5];
	array[0] = (void*)"object";
	array[1] = &obj_id;
	array[2] = (void*)"vertex";
	array[3] = &vertex_id;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "NormalizeWeight", array);
	return (ret != 0);
}

int MQBoneManager::GetEffectLimitNum()
{
	if(!m_Verified) return 0;

	void *array = nullptr;
	int effectlimit = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetEffectLimitNum", array);
	return effectlimit;
}

bool MQBoneManager::DeformObject(MQObject obj, MQObject target)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"object";
	array[1] = obj;
	array[2] = (void*)"target";
	array[3] = target;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "DeformObject", array);
	return (ret != 0);
}
