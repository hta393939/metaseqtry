//---------------------------------------------------------------------------
//
//   MQBoneManager.h      Copyright(C) 1999-2024, tetraface Inc.
//
//		An accessor for bone information
//
//    　ボーン情報のアクセス
//
//---------------------------------------------------------------------------

#ifndef _MQBONEMANAGER_H_
#define _MQBONEMANAGER_H_

#include "MQBasePlugin.h"

class MQBoneManager
{
public:
	MQBoneManager(MQBasePlugin *plugin, MQDocument doc);

	static DWORD GetProductID();
	static DWORD GetPluginID();

	bool Verified() const { return m_Verified; }

	// Must call first in a 'Import' plugin.
	void BeginImport();
	// Must call last in a 'Import' plugin.
	void EndImport();

	struct ADD_BONE_PARAM {
		//MQPoint root_pos;
		//MQPoint tip_pos;
		MQPoint pos;
		std::wstring name;
		UINT parent_id;
		int ik_chain;
		bool is_dummy;
		//bool end_point;

		ADD_BONE_PARAM(){
			pos = MQPoint(0,0,0);
			parent_id = 0;
			ik_chain = 0;
			is_dummy = false;
			//end_point = false;
		}
	};

	struct LINK_PARAM {
		UINT link_bone_id;
		float rotate;
		float move;
		float scale;

		LINK_PARAM(){
			link_bone_id = 0;
			rotate = move = scale = 100.f;
		}
	};

	UINT AddBone(const ADD_BONE_PARAM& param);
	//bool AddBrother(UINT bone_id, UINT brother_bone_id);
	bool DeleteBone(UINT bone_id);

	// Bone
	int GetBoneNum();
	int EnumBoneID(std::vector<UINT>& bone_id_array);
	int EnumSelectedBoneID(std::vector<UINT>& bone_id_array);
	void Update();

	bool GetName(UINT bone_id, std::wstring& name);
	//bool GetTipName(UINT bone_id, std::wstring& name);
	bool GetParent(UINT bone_id, UINT& parent_id);
	bool GetChildNum(UINT bone_id, int& child_num);
	bool GetChildren(UINT bone_id, std::vector<UINT>& children);
	//bool GetBrothers(UINT bone_id, std::vector<UINT>& brothers);
	//bool GetBaseRootPos(UINT bone_id, MQPoint& pos);
	//bool GetBaseTipPos(UINT bone_id, MQPoint& pos);
	bool GetBasePos(UINT bone_id, MQPoint& pos);
	bool GetBaseMatrix(UINT bone_id, MQMatrix& matrix);
	bool GetUpVector(UINT bone_id, MQMatrix& matrix);
	bool GetForwardAxis(UINT bone_id, int& axis);
	//bool GetDeformRootPos(UINT bone_id, MQPoint& pos);
	//bool GetDeformTipPos(UINT bone_id, MQPoint& pos);
	bool GetDeformPos(UINT bone_id, MQPoint& pos);
	bool GetDeformMatrix(UINT bone_id, MQMatrix& matrix);
	bool GetDeformScale(UINT bone_id, MQPoint& scale);
	bool GetDeformRotate(UINT bone_id, MQAngle& angle);
	bool GetDeformTranslate(UINT bone_id, MQPoint& translate);
	bool GetRotationMatrix(UINT bone_id, MQMatrix& matrix);
	bool GetAngleMin(UINT bone_id, MQAngle& angle);
	bool GetAngleMax(UINT bone_id, MQAngle& angle);
	bool GetVisible(UINT bone_id, bool& dummy);
	bool GetLock(UINT bone_id, bool& lock);
	bool GetDummy(UINT bone_id, bool& dummy);
	//bool GetEndPoint(UINT bone_id, bool& end_point);
	bool GetIKChain(UINT bone_id, int& chain);
	bool GetMovable(UINT bone_id, bool& movable);

	void SetName(UINT bone_id, const wchar_t *name);
	//void SetTipName(UINT bone_id, const wchar_t *tip_name);
	void SetParent(UINT bone_id, UINT parent_id);
	//void SetBaseRootPos(UINT bone_id, const MQPoint& pos);
	//void SetBaseTipPos(UINT bone_id, const MQPoint& pos);
	void SetBasePos(UINT bone_id, const MQPoint& pos);
	void SetUpVector(UINT bone_id, const MQMatrix matrix);
	void SetForwardAxis(UINT bone_id, int axis);
	void SetDeformScale(UINT bone_id, const MQPoint& scale);
	void SetDeformRotate(UINT bone_id, const MQAngle& angle);
	void SetDeformTranslate(UINT bone_id, const MQPoint& translate);
	void SetAngleMin(UINT bone_id, const MQAngle& angle);
	void SetAngleMax(UINT bone_id, const MQAngle& angle);
	void SetVisible(UINT bone_id, bool visible);
	void SetLock(UINT bone_id, bool lock);
	void SetDummy(UINT bone_id, bool dummy);
	//void SetEndPoint(UINT bone_id, bool end_point);
	void SetIKChain(UINT bone_id, int chain);
	void SetMovable(UINT bone_id, bool movable);
	
	// Skin object
	int GetSkinObjectNum();
	int EnumSkinObjectID(std::vector<UINT>& obj_id_array);
	bool AddSkinObject(MQObject obj);
	bool DeleteSkinObject(MQObject obj);
	int GetVertexWeightArray(MQObject obj, UINT vertex_id, int array_num, UINT *bone_ids, float *weights);
	bool SetVertexWeight(MQObject obj, UINT vertex_id, UINT bone_id, float weight);
	int GetWeightedVertexArray(UINT bone_id, MQObject obj, std::vector<UINT>& vertex_ids, std::vector<float>& weights);
	bool NormalizeVertexWeight(MQObject obj, UINT vertex_id);

	// for PMD
	//bool GetTipBone(UINT bone_id, UINT& tip_bone_id);
	bool GetIKName(UINT bone_id, std::wstring& ik_name, std::wstring& ik_tip_name);
	bool GetIKParent(UINT bone_id, UINT& ik_parent_bone_id, bool& isIK);
	bool GetLink(UINT bone_id, LINK_PARAM& param);
	bool GetGroupID(UINT bone_id, UINT& group_id);
	//void SetTipBone(UINT bone_id, UINT tip_bone_id);
	void SetIKName(UINT bone_id, const wchar_t *ik_name, const wchar_t *ik_tip_name);
	void SetIKParent(UINT bone_id, UINT ik_parent_bone_id, bool isIK);
	void SetLink(UINT bone_id, const LINK_PARAM& param);
	void SetGroupID(UINT bone_id, UINT group_id);

	enum class OPERATION_MODE {
		DEFAULT,
		PMD,
		VRM,   // VRM 1.0
		VRM0x, // VRM 0.x
	};
	OPERATION_MODE GetOperationMode();
	bool SetOperationMode(OPERATION_MODE mode);

	// for PMD group
	int EnumGroups(std::vector<UINT>& group_ids);
	UINT AddGroup(const wchar_t *name); // ANSI string
	bool GetGroupName(UINT group_id, std::wstring& name);

	// for VRM
	// meta
	struct VRM_META {
		std::wstring name;
		std::wstring version;
		std::wstring authors;
		std::wstring licenseUrl = L"https://vrm.dev/licenses/1.0/";
		std::wstring copyrightInformation;
		std::wstring contactInformation;
		std::wstring references;
		std::wstring thirdPartyLicenses;
		std::wstring thumbnailImage;
		int avatarPermission = 0; // onlyAuthor,onlySeparatelyLicensedPerson,everyone
		bool allowExcessivelyViolentUsage = false;
		bool allowExcessivelySexualUsage = false;
		bool allowPoliticalOrReligiousUsage = false;
		bool allowAntisocialOrHateUsage = false;
		int commercialUsage = 0; // personalNonProfit,personalProfit,corporation
		int creditNotation = 0; // [required],unnecessary
		bool allowRedistribution = false;
		int modification = 0; // [prohibited],allowModification,allowModificationRedistribution
		std::wstring otherLicenseUrl;

		static const char *avatarPermission_str[3];// = {"onlyAuthor","onlySeparatelyLicensedPerson","everyone"};
		static const char *commercialUsage_str[3];// = {"personalNonProfit","personalProfit","corporation"};
		static const char *creditNotation_str[2];// = {"required","unnecessary"};
		static const char *modification_str[3];// = {"prohibited","allowModification","allowModificationRedistribution"};

		static int indexAvatarPermission(const char *str){
			if(str == nullptr) return 0;
			for(int i=0; i<_countof(avatarPermission_str); i++){
				if(strcmp(avatarPermission_str[i], str) == 0)
					return i;
			}
			return 0;
		}
		static int indexCommercialUsage(const char *str){
			if(str == nullptr) return 0;
			for(int i=0; i<_countof(commercialUsage_str); i++){
				if(strcmp(commercialUsage_str[i], str) == 0)
					return i;
			}
			return 0;
		}
		static int indexCreditNotation(const char *str){
			if(str == nullptr) return 0;
			for(int i=0; i<_countof(creditNotation_str); i++){
				if(strcmp(creditNotation_str[i], str) == 0)
					return i;
			}
			return 0;
		}
		static int indexModification(const char *str){
			if(str == nullptr) return 0;
			for(int i=0; i<_countof(modification_str); i++){
				if(strcmp(modification_str[i], str) == 0)
					return i;
			}
			return 0;
		}
	};
	struct VRM0x_META {
		std::wstring title;
		std::wstring version;
		std::wstring author;
		std::wstring contactInformation;
		std::wstring reference;
		int allowedUserName = 0; // OnlyAuthor,ExplicitlyLicensedPerson,Everyone
		bool violentUsage = false;
		bool sexualUsage = false;
		bool commercialUsage = false;
		std::wstring otherPermissionUrl;
		int licenseName = 0;
		std::wstring otherLicenseUrl;
		std::wstring texture; // filename for thumbnail

		static const char *allowedUserName_str[3];// = {"OnlyAuthor","ExplicitlyLicensedPerson","Everyone"};
		static const char *licenseName_str[9];

		static int indexAllowedUserName(const char *str){
			if(str == nullptr) return 0;
			for(int i=0; i<_countof(allowedUserName_str); i++){
				if(strcmp(allowedUserName_str[i], str) == 0)
					return i;
			}
			return 0;
		}
		static int indexLicenseName(const char *str){
			if(str == nullptr) return 0;
			for(int i=0; i<_countof(licenseName_str); i++){
				if(strcmp(licenseName_str[i], str) == 0)
					return i;
			}
			return 0;
		}
	};
	bool VRMMeta_Get(VRM_META& meta);
	bool VRMMeta_Set(const VRM_META& meta);
	bool VRM0xMeta_Get(VRM0x_META& meta);
	bool VRM0xMeta_Set(const VRM0x_META& meta);
	bool VRM0x_GetFlipZ();
	void VRM0x_SetFlipZ(bool flag);
	// humanoid
	bool VRMHumanoid_Get(UINT bone_id, std::wstring& humanoid);
	void VRMHumanoid_Set(UINT bone_id, const wchar_t *humanoid);
	struct VRM_HUMANOID_TEMPLATE {
		std::wstring name;
		std::wstring parent;
		bool required = false;
		bool need_parent = false;
		UINT bone_id = 0;
	};
	bool VRMHumanoid_GetTemplate(std::vector<VRM_HUMANOID_TEMPLATE>& humanoid, OPERATION_MODE mode = OPERATION_MODE::DEFAULT, bool acquire_bone_id = true);
	// firstPerson
	struct VRM_MESH_ANNOTATION {
		enum class Type : int {
			kAuto,
			kBoth,
			kThirdPersonOnly,
			kFirstPersonOnly,
		} type;
	};
	UINT VRMFirstPerson_NewMeshAnnotation(UINT obj_id, const VRM_MESH_ANNOTATION& anno);
	bool VRMFirstPerson_DeleteMeshAnnotation(UINT annotation_id);
	bool VRMFirstPerson_Enumerate(std::vector<UINT>& annotation_ids);
	bool VRMFirstPerson_GetMeshAnnotation(UINT annotation_id, UINT& obj_id, VRM_MESH_ANNOTATION& anno);
	bool VRMFirstPerson_SetMeshAnnotation(UINT annotation_id, const VRM_MESH_ANNOTATION& anno);
	// lookAt
	struct VRM_LOOKAT_PARAM {
		enum class Type {
			kBone,
			kExpression,
		} type = Type::kBone;
		MQPoint offsetFromHeadBone = MQPoint(0.0f, 0.06f, 0.0f); // in meter
		struct RangeMap {
			float inputMaxValue = 90.f;
			float outputScale = 10.f;
		};
		RangeMap rangeMapHorizontalInner;
		RangeMap rangeMapHorizontalOuter;
		RangeMap rangeMapVerticalDown;
		RangeMap rangeMapVerticalUp;
	};
	bool VRMLookAt_Get(VRM_LOOKAT_PARAM& param);
	bool VRMLookAt_Set(const VRM_LOOKAT_PARAM& param);
	// springBone,collider
	struct VRM_COLLIDER_PARAM {
		enum Shape {
			kSphere,
			kCapsule,
		};
		int shape = kSphere;
		MQPoint offset = MQPoint(0,0,0); // in meter
		float radius = 0.0f; // in meter
		MQPoint tail = MQPoint(0,0,0); // for capsule only, in meter
	};
	struct VRM_SPRING_JOINT {
		float hitRadius = 0.0f; // in meter
		float stiffness = 1.0f;
		float gravityPower = 0.0f;
		MQPoint gravityDir = MQPoint(0,-1,0);
		float dragForce = 0.5f;
	};
	UINT VRMCollider_New(UINT bone_id);
	bool VRMCollider_Delete(UINT collider_id);
	bool VRMCollider_Enumerate(std::vector<UINT>& collider_ids);
	bool VRMCollider_GetParam(UINT collider_id, UINT& bone_id, VRM_COLLIDER_PARAM& param);
	bool VRMCollider_SetParam(UINT collider_id, const VRM_COLLIDER_PARAM& param);
	UINT VRMColliderGroup_New(const std::wstring& name, std::vector<UINT> collders = std::vector<UINT>());
	bool VRMColliderGroup_Delete(UINT group_id);
	bool VRMColliderGroup_Enumerate(std::vector<UINT>& group_ids);
	bool VRMColliderGroup_GetName(UINT group_id, std::wstring& name);
	bool VRMColliderGroup_SetName(UINT group_id, const std::wstring& name);
	bool VRMColliderGroup_GetColliders(UINT group_id, std::vector<UINT>& collider_ids);
	bool VRMColliderGroup_SetColliders(UINT group_id, std::vector<UINT> collider_ids);
	UINT VRMSpringBone_New(const std::wstring& name);
	bool VRMSpringBone_Delete(UINT spring_id);
	bool VRMSpringBone_Enumerate(std::vector<UINT>& spring_ids);
	bool VRMSpringBone_GetName(UINT spring_id, std::wstring& name);
	bool VRMSpringBone_SetName(UINT spring_id, const std::wstring& name);
	int VRMSpringBone_GetJointNum(UINT spring_id);
	bool VRMSpringBone_AddJoint(UINT spring_id, UINT bone_id, const VRM_SPRING_JOINT& joint);
	bool VRMSpringBone_DeleteJoint(UINT spring_id, int index);
	bool VRMSpringBone_GetJoint(UINT spring_id, int index, UINT& bone_id, VRM_SPRING_JOINT& joint);
	bool VRMSpringBone_SetJoint(UINT spring_id, int index, const VRM_SPRING_JOINT& joint);
	bool VRMSpringBone_GetColliderGroups(UINT spring_id, std::vector<UINT>& group_ids);
	bool VRMSpringBone_SetColliderGroups(UINT spring_id, std::vector<UINT> group_ids);
	bool VRMSpringBone_GetCenterBone(UINT spring_id, UINT& center_bone_id);
	bool VRMSpringBone_SetCenterBone(UINT spring_id, UINT center_bone_id);
	// nodeConstraint
	struct VRM_NODE_CONSTRAINT {
		UINT source_bone_id = 0;
		enum class Type {
			Roll,
			Aim,
			Rotation,
		} type = Type::Roll;
		enum class RollAxis {
			X,
			Y,
			Z,
		};
		enum class AimAxis {
			PositiveX,
			NegativeX,
			PositiveY,
			NegativeY,
			PositiveZ,
			NegativeZ,
		};
		int axis = 0; // RollAxis(kRoll) or AimAxis(kAim)
		float weight = 1.0f;
	};
	UINT VRMNodeConstraint_New(UINT bone_id);
	bool VRMNodeConstraint_Delete(UINT nc_id);
	bool VRMNodeConstraint_Enumerate(std::vector<UINT>& nc_ids);
	bool VRMNodeConstraint_Get(UINT nc_id, UINT& bone_id, VRM_NODE_CONSTRAINT& param);
	bool VRMNodeConstraint_Set(UINT nc_id, const VRM_NODE_CONSTRAINT& param);

	// Configurations
	int GetEffectLimitNum();

	/*enum LIST_MODE {
		LIST_MODE_BONE,
		LIST_MODE_NODE,
	};
	LIST_MODE GetListMode();
	void SetListMode(LIST_MODE mode);*/

	// Deform
	bool DeformObject(MQObject obj, MQObject target); 	// 'obj' must be a skin object, and 'target' must be a cloned object from 'obj'

private:
	MQBasePlugin *m_Plugin;
	MQDocument m_Doc;
	bool m_Verified;
};


#endif //_MQBONEMANAGER_H_
