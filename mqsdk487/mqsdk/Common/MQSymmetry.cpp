//---------------------------------------------------------------------------
//
//   MQHandleObject.cpp      Copyright(C) 1999-2024, tetraface Inc.
//
//		Symmetry judgement functions
//
//    　対称判定関数
//
//---------------------------------------------------------------------------

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
#include <float.h>
#include <cmath>
#include <algorithm>
#include "MQSymmetry.h"
#include "MQ3DLib.h"
#include "MQBoundingBox.h"


//---------------------------------------------------------------------------
//  class MQSymmetryLeaf
//---------------------------------------------------------------------------
struct MQSymmetryIndex {
	int index; //index of vertex
};

class MQSymmetryLeaf
{
public:
	std::vector<MQSymmetryIndex> tree;

	inline void Add(const MQSymmetryIndex& ntree) { tree.push_back(ntree); }
	void DeleteByTreeIndex(int index);
	void DeleteByVertexIndex(int index);
	int Search(const std::vector<MQPoint>& points, const MQPoint& p, float &dis, int &treeindex);
};

void MQSymmetryLeaf::DeleteByTreeIndex(int index)
{
	if(tree.size() > 0)
		tree[index] = tree[tree.size()-1];
	tree.resize(tree.size()-1);
}

void MQSymmetryLeaf::DeleteByVertexIndex(int index)
{
	for(size_t i=0; i<tree.size(); i++){
		if (tree[i].index == index){
			tree[i] = tree[tree.size()-1];
			tree.resize(tree.size()-1);
			break;
		}
	}
}

int MQSymmetryLeaf::Search(const std::vector<MQPoint>& points, const MQPoint& p, float &dis, int &treeindex)
{
	float dis2 = dis*dis;
	int minvi = -1;
	size_t count = tree.size();

	for(size_t i=0; i<count; i++)
	{
		int cvi = tree[i].index;
		if(cvi < 0) continue;
		MQPoint dv = points[cvi] - p;
		float cdis = dv.x*dv.x + dv.y*dv.y + dv.z*dv.z;
		if(cdis <= dis2) {
			treeindex = (int)i;
			minvi = cvi;
			dis2 = cdis;
		}
	}

	if(minvi >= 0)
		dis = sqrtf(dis2);
	return minvi;
}

#define OVSEGX 4
#define OVSEGY 8
#define OVSEGZ 8

//---------------------------------------------------------------------------
//  class MQSymmetryTree
//---------------------------------------------------------------------------
class MQSymmetryTree {
private:
	MQSymmetryLeaf left[OVSEGX*OVSEGY*OVSEGZ];
	MQSymmetryLeaf right[OVSEGX*OVSEGY*OVSEGZ];

	inline int getIndex(int ix, int iy, int iz) {
		return ix + iy*OVSEGX + iz*OVSEGX*OVSEGY;
	}
	inline MQSymmetryLeaf *GetLeft(int ix, int iy, int iz) {
		return &left[getIndex(ix,iy,iz)];
	}
	inline MQSymmetryLeaf *GetRight(int ix, int iy, int iz) {
		return &right[getIndex(ix,iy,iz)];
	}
	bool GetSymmetryBoundingBox(const std::vector<MQPoint>& points, MQBoundingBox& box, MQPoint& boxsize);

public:

	MQSymmetryTree() {
		for(int i=0; i<OVSEGX*OVSEGY*OVSEGZ; i++) {
			//left[i].tree.resize(16);
			//right[i].tree.resize(16);
		}
	}
	int CreateTable(const std::vector<MQPoint>& points, std::vector<MQSymmetryTable>& table, const std::vector<bool> *apply, float mindis, MQSymmetryTable::SymmetryAxis axis);
};


bool MQSymmetryTree::GetSymmetryBoundingBox(const std::vector<MQPoint>& points, MQBoundingBox& box, MQPoint& boxsize)
{
	float box_x;

	box.init();

	size_t num = points.size();
	for(size_t i=0; i<num; i++){
		if(points[i].x != FLT_MAX)
			box.expand(points[i]);
	}
	if(!box.isEnabled() || box.maxp.x<0 || box.minp.x>0)
		return false;
	box_x = std::max(box.maxp.x, -box.minp.x);
	box.minp.x = -box_x;
	box.maxp.x = box_x;
	boxsize.x = box_x * 2;
	boxsize.y = box.maxp.y - box.minp.y;
	boxsize.z = box.maxp.z - box.minp.z;

	return true;
}


//---------------------------------------------------------------------------
//  MQSymmetryTree::CreateTable
//---------------------------------------------------------------------------
int MQSymmetryTree::CreateTable(const std::vector<MQPoint>& points, std::vector<MQSymmetryTable>& table, const std::vector<bool> *apply, float mindis, MQSymmetryTable::SymmetryAxis axis)
// return: count of pair of symmetry vertices
{
	size_t vc = points.size();
	int pairc = 0;
	size_t i;

	table.resize(vc);
	for(i=0; i<vc; i++){
		table[i].index = -1;
	}

	const float zero_err = std::max(mindis * (float)1e-6, (float)1e-6);
	switch(axis){
	case MQSymmetryTable::AxisX:
		for(i=0; i<vc; i++){
			if(points[i].x == FLT_MAX){
				table[i].side = MQSymmetryTable::SideNone;
			}else{
				if(points[i].x > zero_err)
					table[i].side = MQSymmetryTable::SidePlus;
				else if(points[i].x <-zero_err)
					table[i].side = MQSymmetryTable::SideMinus;
				else
					table[i].side = MQSymmetryTable::SideCenter;
			}
		}
		break;
	case MQSymmetryTable::AxisY:
		for(i=0; i<vc; i++){
			if(points[i].x == FLT_MAX){
				table[i].side = MQSymmetryTable::SideNone;
			}else{
				if(points[i].y > zero_err)
					table[i].side = MQSymmetryTable::SidePlus;
				else if(points[i].y <-zero_err)
					table[i].side = MQSymmetryTable::SideMinus;
				else
					table[i].side = MQSymmetryTable::SideCenter;
			}
		}
		break;
	case MQSymmetryTable::AxisZ:
		for(i=0; i<vc; i++){
			if(points[i].x == FLT_MAX){
				table[i].side = MQSymmetryTable::SideNone;
			}else{
				if(points[i].z > zero_err)
					table[i].side = MQSymmetryTable::SidePlus;
				else if(points[i].z <-zero_err)
					table[i].side = MQSymmetryTable::SideMinus;
				else
					table[i].side = MQSymmetryTable::SideCenter;
			}
		}
		break;
	}

	// get boundind-box
	MQPoint boxsize;
	MQBoundingBox box;
	if( !GetSymmetryBoundingBox(points, box, boxsize) )
		return 0;

	MQPoint axisboxsize;
	switch(axis){
	case MQSymmetryTable::AxisX: axisboxsize = MQPoint(box.maxp.x, boxsize.y, boxsize.z); break;
	case MQSymmetryTable::AxisY: axisboxsize = MQPoint(boxsize.x, box.maxp.y, boxsize.z); break;
	case MQSymmetryTable::AxisZ: axisboxsize = MQPoint(boxsize.x, boxsize.y, box.maxp.z); break;
	}

	float cdis;
	int ix1,ix2,iy1,iy2,iz1,iz2;

	// register unselected vertices
	for(i=0; i<vc; i++)
	{
		if(points[i].x == FLT_MAX) continue;
		if(apply != nullptr && (*apply)[i]) continue;

		const MQPoint& v = points[i];
		MQSymmetryIndex ot;
		ot.index = (int)i;

		MQPoint segp;
		switch(axis){
		case MQSymmetryTable::AxisX: segp = MQPoint(fabsf(v.x), v.y-box.minp.y, v.z-box.minp.z); break;
		case MQSymmetryTable::AxisY: segp = MQPoint(v.x-box.minp.x, fabsf(v.y), v.z-box.minp.z); break;
		case MQSymmetryTable::AxisZ: segp = MQPoint(v.x-box.minp.x, v.y-box.minp.y, fabsf(v.z)); break;
		}
		switch(table[i].side){
		case MQSymmetryTable::SidePlus:
			ix1 = (axisboxsize.x > 0) ? std::max(0,std::min(OVSEGX-1,(int)(segp.x/axisboxsize.x*(float)OVSEGX))) : 0;
			iy1 = (axisboxsize.y > 0) ? std::max(0,std::min(OVSEGY-1,(int)(segp.y/axisboxsize.y*(float)OVSEGY))) : 0;
			iz1 = (axisboxsize.z > 0) ? std::max(0,std::min(OVSEGZ-1,(int)(segp.z/axisboxsize.z*(float)OVSEGZ))) : 0;
			GetLeft(ix1,iy1,iz1)->Add(ot);
			break;
		case MQSymmetryTable::SideMinus:
			ix1 = (axisboxsize.x > 0) ? std::max(0,std::min(OVSEGX-1,(int)(segp.x/axisboxsize.x*(float)OVSEGX))) : 0;
			iy1 = (axisboxsize.y > 0) ? std::max(0,std::min(OVSEGY-1,(int)(segp.y/axisboxsize.y*(float)OVSEGY))) : 0;
			iz1 = (axisboxsize.z > 0) ? std::max(0,std::min(OVSEGZ-1,(int)(segp.z/axisboxsize.z*(float)OVSEGZ))) : 0;
			GetRight(ix1,iy1,iz1)->Add(ot);
			break;
		}
	}

	for(i=0; i<vc; i++)
	{
		if(points[i].x == FLT_MAX) continue;

		if((apply != nullptr && !(*apply)[i]) || table[i].IsNoneOrCenter() || table[i].index!=-1)
			continue;

		const MQPoint& v = points[i];
		MQPoint segp;
		switch(axis){
		case MQSymmetryTable::AxisX: segp = MQPoint(fabs(v.x), v.y-box.minp.y, v.z-box.minp.z); break;
		case MQSymmetryTable::AxisY: segp = MQPoint(v.x-box.minp.x, fabs(v.y), v.z-box.minp.z); break;
		case MQSymmetryTable::AxisZ: segp = MQPoint(v.x-box.minp.x, v.y-box.minp.y, fabs(v.z)); break;
		}

		if(axisboxsize.x > 0){
			ix1 = std::max(0, (int)((segp.x-mindis)/axisboxsize.x*(float)OVSEGX));
			ix2 = std::min(OVSEGX-1, (int)((segp.x+mindis)/axisboxsize.x*(float)OVSEGX));
		}else{
			ix1 = ix2 = 0;
		}
		if(axisboxsize.y > 0){
			iy1 = std::max(0, (int)((segp.y-mindis)/axisboxsize.y*(float)OVSEGY));
			iy2 = std::min(OVSEGY-1, (int)((segp.y+mindis)/axisboxsize.y*(float)OVSEGY));
		}else{
			iy1 = iy2 = 0;
		}
		if(axisboxsize.z > 0){
			iz1 = std::max(0, (int)((segp.z-mindis)/axisboxsize.z*(float)OVSEGZ));
			iz2 = std::min(OVSEGZ-1, (int)((segp.z+mindis)/axisboxsize.z*(float)OVSEGZ));
		}else{
			iz1 = iz2 = 0;
		}

		cdis = mindis;
		int minvi = -1;
		int mintreeindex = -1;
		MQSymmetryLeaf *mintree = nullptr;

		switch(table[i].side) {
		case MQSymmetryTable::SidePlus:
			for(int z=iz1; z<=iz2; z++){
				for(int y=iy1; y<=iy2; y++){
					for(int x=ix1; x<=ix2; x++){
						int treeindex;
						int cvi = GetRight(x,y,z)->Search(points, MQSymmetryTable::Symmetry(v, axis), cdis, treeindex);
						if(cvi >= 0){
							minvi = cvi;
							mintreeindex = treeindex;
							mintree = GetRight(x,y,z);
						}
					}
				}
			}
			if(minvi >= 0){
				// 対が見つかった
				table[i].index = minvi;
				table[minvi].index = (int)i;
				mintree->DeleteByTreeIndex(mintreeindex);
				if(apply == nullptr){
					ix1 = (axisboxsize.x > 0) ? std::max(0,std::min(OVSEGX-1,(int)(segp.x/axisboxsize.x*(float)OVSEGX))) : 0;
					iy1 = (axisboxsize.y > 0) ? std::max(0,std::min(OVSEGY-1,(int)(segp.y/axisboxsize.y*(float)OVSEGY))) : 0;
					iz1 = (axisboxsize.z > 0) ? std::max(0,std::min(OVSEGZ-1,(int)(segp.z/axisboxsize.z*(float)OVSEGZ))) : 0;
					GetLeft(ix1,iy1,iz1)->DeleteByVertexIndex((int)i);
				}
				pairc++;
			}else{
				// 見つからないので候補に登録しておく
				if(apply != nullptr){
					MQSymmetryIndex ot;
					ot.index = (int)i;
					ix1 = (axisboxsize.x > 0) ? std::max(0,std::min(OVSEGX-1,(int)(segp.x/axisboxsize.x*(float)OVSEGX))) : 0;
					iy1 = (axisboxsize.y > 0) ? std::max(0,std::min(OVSEGY-1,(int)(segp.y/axisboxsize.y*(float)OVSEGY))) : 0;
					iz1 = (axisboxsize.z > 0) ? std::max(0,std::min(OVSEGZ-1,(int)(segp.z/axisboxsize.z*(float)OVSEGZ))) : 0;
					GetLeft(ix1,iy1,iz1)->Add(ot);
				}
			}
			break;

		case MQSymmetryTable::SideMinus:
			for(int z=iz1; z<=iz2; z++){
				for(int y=iy1; y<=iy2; y++){
					for(int x=ix1; x<=ix2; x++){
						int treeindex;
						int cvi = GetLeft(x,y,z)->Search(points, MQSymmetryTable::Symmetry(v, axis), cdis, treeindex);
						if(cvi >= 0){
							minvi = cvi;
							mintreeindex = treeindex;
							mintree = GetLeft(x,y,z);
						}
					}
				}
			}
			if(minvi >= 0){
				// 対が見つかった
				table[i].index = minvi;
				table[minvi].index = (int)i;
				mintree->DeleteByTreeIndex(mintreeindex);
				if(apply == nullptr){
					ix1 = (axisboxsize.x > 0) ? std::max(0,std::min(OVSEGX-1,(int)(segp.x/axisboxsize.x*(float)OVSEGX))) : 0;
					iy1 = (axisboxsize.y > 0) ? std::max(0,std::min(OVSEGY-1,(int)(segp.y/axisboxsize.y*(float)OVSEGY))) : 0;
					iz1 = (axisboxsize.z > 0) ? std::max(0,std::min(OVSEGZ-1,(int)(segp.z/axisboxsize.z*(float)OVSEGZ))) : 0;
					GetRight(ix1,iy1,iz1)->DeleteByVertexIndex((int)i);
				}
				pairc++;
			}else{
				// 見つからないので候補に登録しておく
				if(apply != nullptr){
					MQSymmetryIndex ot;
					ot.index = (int)i;
					ix1 = (axisboxsize.x > 0) ? std::max(0,std::min(OVSEGX-1,(int)(segp.x/axisboxsize.x*(float)OVSEGX))) : 0;
					iy1 = (axisboxsize.y > 0) ? std::max(0,std::min(OVSEGY-1,(int)(segp.y/axisboxsize.y*(float)OVSEGY))) : 0;
					iz1 = (axisboxsize.z > 0) ? std::max(0,std::min(OVSEGZ-1,(int)(segp.z/axisboxsize.z*(float)OVSEGZ))) : 0;
					GetRight(ix1,iy1,iz1)->Add(ot);
				}
			}
			break;
		}
	}

	return pairc;
}


//---------------------------------------------------------------------------
//  InitVertexTable
//---------------------------------------------------------------------------
int MQSymmetryTable::InitVertexTable(MQObject obj, std::vector<MQSymmetryTable>& table, const std::vector<bool> *apply, float mindis, MQSymmetryTable::SymmetryAxis axis)
// return: count of pair of symmetry vertices
{
	int num = obj->GetVertexCount();
	std::vector<MQPoint> points(num);
	for(int i=0; i<num; i++){
		if(obj->GetVertexRefCount(i) > 0)
			points[i] = obj->GetVertex(i);
		else
			points[i] = MQPoint(FLT_MAX,FLT_MAX,FLT_MAX);
	}

	MQSymmetryTree sym;
	int count = sym.CreateTable(points, table, apply, mindis, axis);
	return count;
}

int MQSymmetryTable::InitVertexTable(const std::vector<MQPoint>& points, std::vector<MQSymmetryTable>& table, const std::vector<bool> *apply, float mindis, MQSymmetryTable::SymmetryAxis axis)
// return: count of pair of symmetry vertices
{
	MQSymmetryTree sym;
	int count = sym.CreateTable(points, table, apply, mindis, axis);
	return count;
}


//---------------------------------------------------------------------------
//  InitSymmetryFaceTable
//---------------------------------------------------------------------------
// return: count of pair of symmetry faces
int MQSymmetryTable::InitFaceTable(MQObject obj, std::vector<MQSymmetryFaceTable>& face_table, const std::vector<bool> *face_apply, const std::vector<MQSymmetryTable>& vert_table, float mindis)
{
	std::vector<int> related_faces, face1_vert, face2_vert;
	int count = 0;
	int face_num = obj->GetFaceCount();

	face_table.resize(face_num);
	for(int i=0; i<face_num; i++){
		face_table[i].side = MQSymmetryTable::SideNone;
		face_table[i].index = -1;
	}

	for(int fi1=0; fi1<face_num; fi1++)
	{
		if(face_table[fi1].side != MQSymmetryTable::SideNone) 
			continue;
		int count1 = obj->GetFacePointCount(fi1);
		if(count1 == 0)
			continue;

		face1_vert.resize(count1);
		obj->GetFacePointArray(fi1, face1_vert.data());

		// すべての頂点が対称または中心のものでなければならない
		std::vector<int> symvert(count1);
		MQSymmetryTable::SymmetrySide side = MQSymmetryTable::SideNone;
		bool nosym = false;
		for(int vi1=0; vi1<count1 && !nosym; vi1++){
			MQSymmetryTable::SymmetrySide temp_side = vert_table[face1_vert[vi1]].side;
			switch(temp_side){
			case MQSymmetryTable::SideNone:
				nosym = true;
				break;
			case MQSymmetryTable::SidePlus:
			case MQSymmetryTable::SideMinus:
				symvert[vi1] = vert_table[face1_vert[vi1]].index;
				if (symvert[vi1] == -1){
					nosym = true;
					break;
				}
				if (side == MQSymmetryTable::SideNone)
					side = temp_side;
				break;
			case MQSymmetryTable::SideCenter:
				symvert[vi1] = face1_vert[vi1];
				break;
			}
		}
		if (nosym || side == MQSymmetryTable::SideNone)
			continue;

		// 対となる面を探す
		int related_num = obj->GetVertexRelatedFaces(symvert[0], nullptr);
		related_faces.resize(face_num);
		obj->GetVertexRelatedFaces(symvert[0], related_faces.data());
		for(int n=0; n<related_num; n++){
			int fi2 = related_faces[n];
			if(fi2 <= fi1) continue;
			if(face_table[fi2].side != MQSymmetryTable::SideNone) continue;

			int count2 = obj->GetFacePointCount(fi2);
			if(count1 != count2) continue;

			if(face_apply != nullptr){
				if(!(*face_apply)[fi1] && !(*face_apply)[fi2])
					continue;
			}

			face2_vert.resize(count2);
			obj->GetFacePointArray(fi2, face2_vert.data());

			for(int i=0; i<count1; i++){
				int j;
				for(j=0; j<count1; j++){
					if(face2_vert[(count1+i-j)%count1] != symvert[j])
						break;
				}
				if(j == count1){
					face_table[fi1].index = fi2;
					face_table[fi2].index = fi1;
					face_table[fi1].side = side;
					face_table[fi2].side = (side == MQSymmetryTable::SideMinus) ? MQSymmetryTable::SidePlus : MQSymmetryTable::SideMinus;
					face_table[fi1].apex0 = i;
					face_table[fi2].apex0 = i;
					count++;
					break;
				}
			}
		}
	}

	return count;
}


