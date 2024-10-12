#pragma once

#ifndef _MQBOUNDINGBOX_H_
#define _MQBOUNDINGBOX_H_

#include "MQPlugin.h"


struct MQBoundingBox2
{
	MQCoordinate minp, maxp;

	MQBoundingBox2();
	MQBoundingBox2(const MQCoordinate& p1, const MQCoordinate& p2);

	void 			init(void);
	bool 			isEnabled(void) const;
	const MQCoordinate& 	getMin(void) const		{ return minp; }
	const MQCoordinate& 	getMax(void) const		{ return maxp; }
	const MQCoordinate getCenter(void) const	{ return (minp + maxp) / 2; }
	const MQCoordinate getSize(void) 	const	{ return maxp - minp; }
	bool 			isInside(const MQCoordinate& p) const;
	bool			isOverlapped(const MQBoundingBox2& box) const;
	MQBoundingBox2	getOverlappedBox(const MQBoundingBox2& box) const;
	void 			expand(const MQCoordinate& p);
	void			combine(const MQBoundingBox2& box);
	void			inflate(float dx, float dy);
	void			inflate(const MQCoordinate& d);
};

struct MQBoundingBox
{
	MQPoint 		minp, maxp;

	MQBoundingBox();
	MQBoundingBox(const MQPoint& p1, const MQPoint& p2);

	void 			init(void);
	bool 			isEnabled(void) const;
	const MQPoint& 	getMin(void) const		{ return minp; }
	const MQPoint& 	getMax(void) const		{ return maxp; }
	const MQPoint 	getCenter(void) const	{ return (minp + maxp) / 2; }
	const MQPoint 	getSize(void) 	const	{ return maxp - minp; }
	bool 			isInside(const MQPoint& p) const;
	bool 			isInsideXY(const MQPoint& p) const;
	bool 			isInsideXZ(const MQPoint& p) const;
	bool 			isInsideYZ(const MQPoint& p) const;
	bool			isOverlapped(const MQBoundingBox& box) const;
	MQBoundingBox	getOverlappedBox(const MQBoundingBox& box) const;
	void 			expand(const MQPoint& p);
	void			combine(const MQBoundingBox& box);
	void			inflate(float dx, float dy, float dz);
};



#endif //_MQBOUNDINGBOX_H_
