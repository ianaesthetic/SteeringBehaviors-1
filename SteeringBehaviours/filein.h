#include "Util.h" 
#pragma warning(disable:4996)
class FileIn {
public: 
	void Init(std::string name) {
		freopen(name.data(), "r", stdin); 
	}

	/* common */ 

	bool IsSkip(const char &c) {
		bool ret; 
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '.') || (c == '#') || (c == '-') || (c >= '0' && c <= '9')) ret = false; 
		else ret = true; 
		return ret; 
	}

	float FloatIn() {
		bool adverse = false; 
		float ret = 0; 
		float t = 0.1f;
		bool flag = false; 
		cin = (char)getchar();
		while (IsSkip(cin)) cin = (char)getchar(); 
		while ((cin >= '0' && cin <= '9') || cin == '.'||cin == '-') {
			if (cin == '-') adverse = true;
			else if (cin == '.') flag = true; 
			else {
				if (!flag) ret = (float)(cin - '0') + ret * 10.0f;
				else {
					ret += (float)(cin - '0') * t; 
					t /= 10.0f; 
				}
			}
			cin = (char)getchar();
		}
		if (adverse) ret = -ret; 
		return ret; 
	}

	int IntIn() {
		bool adverse = false; 
		int ret = 0; 
		cin = (char)getchar(); 
		while (IsSkip(cin)) cin = (char)getchar();
		while ((cin >= '0' && cin <= '9') || cin == '-') {
			if (cin == '-') adverse = true; 
			else {
				ret = (int)(cin - '0') + ret * 10;
			}
			cin = (char)getchar();
		}
		return ret;
	}

	std::string StringIn() {
		std::string ret;
		ret.clear();
		cin = (char)getchar();
		while ((cin >= 'a' && cin <= 'z') || (cin >= 'A' && cin <= 'Z')) {
			ret.push_back(cin);
			cin = (char)getchar();
		}
		return ret;
	}
	
	char cin; 
};