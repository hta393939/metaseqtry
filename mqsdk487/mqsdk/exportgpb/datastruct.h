/**
 * @file datastruct.h 
 */

#include <string>
#include <vector>

// @see https://github.com/gameplay3d/gameplay/blob/master/gameplay/src/AnimationTarget.cpp

// @see https://github.com/gameplay3d/gameplay/blob/master/gameplay/src/Transform.h
enum {
	ROT_VAL = 8,
	ROTMOV_VAL = 16,
	SCALEROTMOV_VAL = 17,
};

#define ROT_STR L"ANIMATE_ROTATE"
#define ROTMOV_STR L"ANIMATE_ROTATE_TRANSLATE"
#define SCALEROTMOV_STR L"ANIMATE_SCALE_ROTATE_TRANSLATE"


struct ONEVAL {
	float sx;
	float sy;
	float sz;
	float qx;
	float qy;
	float qz;
	float qw;
	float tx;
	float ty;
	float tz;
	ONEVAL() {
		sx = 1.0f;
		sy = 1.0f;
		sz = 1.0f;
		qx = 0.0f;
		qy = 0.0f;
		qz = 0.0f;
		qw = 1.0f;
		tx = 0.0f;
		ty = 0.0f;
		tz = 0.0f;
	}
};

struct ANIMATIONCHANNEL {
	std::wstring targetId;
	std::wstring targetAttrib;

	/// <summary>
	/// 16, 17 など
	/// </summary>
	unsigned int attribVal;

	std::vector<float> keytimes;
	/// <summary>
	/// 一直線の方
	/// </summary>
	std::vector<float> values;
	/// <summary>
	/// 構造体に読み直した方
	/// </summary>
	std::vector<ONEVAL> vals;
	std::vector<float> tangentsIn;
	std::vector<float> tangentsOut;
	std::vector<float> interpolations;

	ANIMATIONCHANNEL() {
		attribVal = ROTMOV_VAL;
		interpolations.push_back(1.0f);
	}
};

struct ANIMATION {
	std::wstring id;
	std::vector<ANIMATIONCHANNEL> channels;
	ANIMATION() {
		id = L"animations";
	}
};

struct ANIMATIONS {
	std::wstring id;
	std::vector<ANIMATION> anims;
	ANIMATIONS() {
		id = L"__Animations__";
	}
};


