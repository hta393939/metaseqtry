//#include "stdafx.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
#include <float.h>
#include <algorithm>
#include "MQBoundingBox.h"


MQBoundingBox2::MQBoundingBox2()
{
	init();
}

MQBoundingBox2::MQBoundingBox2(const MQCoordinate& p1, const MQCoordinate& p2)
{
	minp = p1;
	maxp = p2;
}

void MQBoundingBox2::init(void)
{
	minp.u = FLT_MAX;
	minp.v = FLT_MAX;
	maxp.u = -FLT_MAX;
	maxp.v = -FLT_MAX;
}

bool MQBoundingBox2::isEnabled(void) const
{
	return (minp.u<=maxp.u && minp.v<=maxp.v);
}

bool MQBoundingBox2::isInside(const MQCoordinate& p) const
{
	return (
		minp.u <= p.u && p.u <= maxp.u &&
		minp.v <= p.v && p.v <= maxp.v);
}

bool MQBoundingBox2::isOverlapped(const MQBoundingBox2& box) const
{
	if( box.minp.u < maxp.u && minp.u < box.maxp.u && 
		box.minp.v < maxp.v && minp.v < box.maxp.v )
		return true;
	return false;
}

MQBoundingBox2 MQBoundingBox2::getOverlappedBox(const MQBoundingBox2& box) const
{
	float x1 = std::max(minp.u, box.minp.u);
	float x2 = std::min(maxp.u, box.maxp.u);
	float y1 = std::max(minp.v, box.minp.v);
	float y2 = std::min(maxp.v, box.maxp.v);
	if (x1 <= x2 && y1 <= y2){
		return MQBoundingBox2(MQCoordinate(x1,y1), MQCoordinate(x2,y2));
	}else{
		return MQBoundingBox2();
	}
}

void MQBoundingBox2::expand(const MQCoordinate& p)
{
	if(minp.u > p.u) minp.u = p.u;
	if(minp.v > p.v) minp.v = p.v;
	if(maxp.u < p.u) maxp.u = p.u;
	if(maxp.v < p.v) maxp.v = p.v;
}

void MQBoundingBox2::combine(const MQBoundingBox2& box)
{
	if(box.isEnabled()){
		expand(box.getMin());
		expand(box.getMax());
	}
}

void MQBoundingBox2::inflate(float dx, float dy)
{
	if(!isEnabled()) return;

	minp.u -= dx;
	minp.v -= dy;
	maxp.u += dx;
	maxp.v += dy;
}

void MQBoundingBox2::inflate(const MQCoordinate& d)
{
	inflate(d.u, d.v);
}

MQBoundingBox::MQBoundingBox()
{
	init();
}

MQBoundingBox::MQBoundingBox(const MQPoint& p1, const MQPoint& p2)
{
	minp = p1;
	maxp = p2;
}

void MQBoundingBox::init(void)
{
	minp =  FLT_MAX;
	maxp = -FLT_MAX;
}

bool MQBoundingBox::isEnabled(void) const
{
	return (minp.x<=maxp.x && minp.y<=maxp.y && minp.z<=maxp.z);
}

bool MQBoundingBox::isInside(const MQPoint& p) const
{
	return (
		minp.x <= p.x && p.x <= maxp.x &&
		minp.y <= p.y && p.y <= maxp.y &&
		minp.z <= p.z && p.z <= maxp.z);
}

bool MQBoundingBox::isInsideXY(const MQPoint& p) const
{
	return (
		minp.x <= p.x && p.x <= maxp.x &&
		minp.y <= p.y && p.y <= maxp.y);
}

bool MQBoundingBox::isInsideXZ(const MQPoint& p) const
{
	return (
		minp.x <= p.x && p.x <= maxp.x &&
		minp.z <= p.z && p.z <= maxp.z);
}

bool MQBoundingBox::isInsideYZ(const MQPoint& p) const
{
	return (
		minp.y <= p.y && p.y <= maxp.y &&
		minp.z <= p.z && p.z <= maxp.z);
}

bool MQBoundingBox::isOverlapped(const MQBoundingBox& box) const
{
	if( box.minp.x < maxp.x && minp.x < box.maxp.x && 
		box.minp.y < maxp.y && minp.y < box.maxp.y && 
		box.minp.z < maxp.z && minp.z < box.maxp.z )
		return true;
	return false;
}

MQBoundingBox MQBoundingBox::getOverlappedBox(const MQBoundingBox& box) const
{
	float x1 = std::max(minp.x, box.minp.x);
	float x2 = std::min(maxp.x, box.maxp.x);
	float y1 = std::max(minp.y, box.minp.y);
	float y2 = std::min(maxp.y, box.maxp.y);
	float z1 = std::max(minp.z, box.minp.z);
	float z2 = std::min(maxp.z, box.maxp.z);
	if (x1 <= x2 && y1 <= y2 && z1 <= z2){
		return MQBoundingBox(MQPoint(x1,y1,z1), MQPoint(x2,y2,z2));
	}else{
		return MQBoundingBox();
	}
}

void MQBoundingBox::expand(const MQPoint& p)
{
	if(minp.x > p.x) minp.x = p.x;
	if(minp.y > p.y) minp.y = p.y;
	if(minp.z > p.z) minp.z = p.z;
	if(maxp.x < p.x) maxp.x = p.x;
	if(maxp.y < p.y) maxp.y = p.y;
	if(maxp.z < p.z) maxp.z = p.z;
}

void MQBoundingBox::combine(const MQBoundingBox& box)
{
	if(box.isEnabled()){
		expand(box.getMin());
		expand(box.getMax());
	}
}

void MQBoundingBox::inflate(float dx, float dy, float dz)
{
	if(!isEnabled()) return;

	minp.x -= dx;
	minp.y -= dy;
	minp.z -= dz;
	maxp.x += dx;
	maxp.y += dy;
	maxp.z += dz;
}

