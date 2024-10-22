
#include <vector>

struct ONEVAL {
	float x;
	float y;
	float z;
	/// <summary>
	/// 逆向き度単位
	/// </summary>
	float head;
	float pitch;
	float bank;

	ONEVAL() {
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		head = 0.0f;
		pitch = 0.0f;
		bank = 0.0f;
	}
};

struct ONEVALTARGET {
	/// <summary>
	/// ボーン名
	/// </summary>
	std::wstring target;
	ONEVAL val;
};

struct ONEKEYVAL {
	int msec;
	/// <summary>
	/// ボーン複数の1つ
	/// </summary>
	std::vector<ONEVALTARGET> vals;
};

struct BONESINFO {
	std::vector<ONEKEYVAL> keyvals;
};

