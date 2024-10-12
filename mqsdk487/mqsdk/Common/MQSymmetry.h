//---------------------------------------------------------------------------
//
//   MQSymmetry.h      Copyright(C) 1999-2024, tetraface Inc.
//
//		Symmetry judgement functions
//
//    　対称判定関数
//
//---------------------------------------------------------------------------

#ifndef _MQSYMMETRY_H_
#define _MQSYMMETRY_H_

#include "MQBasePlugin.h"

struct MQSymmetryFaceTable;

struct MQSymmetryTable
{
	enum SymmetryAxis {
		AxisX = 0,
		AxisY = 1,
		AxisZ = 2,
	};
	enum SymmetrySide {
		SideNone   = 0,
		SideCenter = 1,
		SidePlus   = 2, // LEFT,TOP,FRONT
		SideMinus  = 3, // RIGHT,BOTTOM,BACK
	};

	SymmetrySide side;
	int index;  // index of pair of vertex

	bool IsNoneOrCenter() const { return side <= 1; }

	static MQPoint Symmetry(const MQPoint& p, SymmetryAxis axis);
	static int InitVertexTable(MQObject obj, std::vector<MQSymmetryTable>& table, const std::vector<bool> *apply, float mindis, MQSymmetryTable::SymmetryAxis axis);
	static int InitVertexTable(const std::vector<MQPoint>& points, std::vector<MQSymmetryTable>& table, const std::vector<bool> *apply, float mindis, MQSymmetryTable::SymmetryAxis axis);
	static int InitFaceTable(MQObject obj, std::vector<MQSymmetryFaceTable>& face_table, const std::vector<bool> *face_apply, const std::vector<MQSymmetryTable>& vert_table, float mindis);
};


struct MQSymmetryFaceTable
{
	MQSymmetryTable::SymmetrySide side;
	int index;  // index of pair of face
	int apex0;	// index of pair of apex[0]
};



inline MQPoint MQSymmetryTable::Symmetry(const MQPoint& p, SymmetryAxis axis){
	switch(axis){
	case MQSymmetryTable::AxisX: return MQPoint(-p.x, p.y, p.z);
	case MQSymmetryTable::AxisY: return MQPoint(p.x, -p.y, p.z);
	case MQSymmetryTable::AxisZ: return MQPoint(p.x, p.y, -p.z);
	}
	return p;
}


#endif //_MQSYMMETRY_H_
