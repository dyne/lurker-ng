// Does the compiler support parital spec?
template <int I, int J>
struct Add
{
	enum { res = I+J };
};

template <int K>
struct Add<K, 3>	// a "special" add function =)
{
	enum { res = K+4 };
};

template <int K>
struct Add<3, K>	// a "special" add function =)
{
	enum { res = K+5 };
};

// This is a way to generate a compile error if a value is non-true
// sizeof(test_result<true>) compiles whilst sizeof(test_result<false>) won't
template <bool B>
struct test_result;

struct test_result<true>
{ };

int main()
{
	return sizeof(test_result<bool(Add<6, 7>::res == 13)>) +
	       sizeof(test_result<bool(Add<2, 3>::res == 6)>) +
	       sizeof(test_result<bool(Add<3, 5>::res == 10)>);
	// note: Add<3,3> would be ambiguous and undefined
}
