
#include <vector>

struct ONEVAL {
	float x;
	float y;
	float z;
	/// <summary>
	/// �t�����x�P��
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
	/// �{�[����
	/// </summary>
	std::wstring target;
	ONEVAL val;
};

struct ONEKEYVAL {
	int msec;
	/// <summary>
	/// �{�[��������1��
	/// </summary>
	std::vector<ONEVALTARGET> vals;
};

struct BONESINFO {
	std::vector<ONEKEYVAL> keyvals;
};

