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

struct CHANNEL {
	std::wstring targetId;
	std::wstring targetAttrib;

	unsigned int attribVal;

	std::vector<float> keytimes;
	std::vector<ONEVAL> values;
	std::vector<float> tangentsIn;
	std::vector<float> tangentsOut;
	std::vector<float> interpolation;

	CHANNEL() {
		attribVal = ROTMOV_VAL;
		interpolation.push_back(1.0f);
	}
};

struct ANIMATION {
	std::wstring id;
	std::vector<CHANNEL> channels;
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


