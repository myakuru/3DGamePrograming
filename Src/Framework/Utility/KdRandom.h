//===========================================
//
// ランダム値生成関係
//
//===========================================
class KdRandom
{
public:
	KdRandom() {
		std::random_device rd;
		rnd = std::mt19937_64(rd());
	}

	static long double GetRandom() { return static_cast<long double>(GetMT()()) / static_cast<long double>(std::mt19937_64::max()); }

	static int GetInt(int _min, int _max) {
		std::uniform_int_distribution range(_min, _max);
		return range(GetMT());
	}

	static float GetFloat(float _min, float _max) {
		std::uniform_real_distribution range(_min, _max);
		return range(GetMT());
	}
private:
	std::mt19937_64 rnd;

	static KdRandom& Instance()
	{
		static KdRandom instance;
		return instance;
	}

	static std::mt19937_64& GetMT() { return KdRandom::Instance().rnd; }
};