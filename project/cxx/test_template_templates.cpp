// Does the compiler support template-template parameters?
template <class T, template <class A> class B>
struct Foo
{
	enum { bar = B<T>::bar };
};

template <class X>
struct Bar
{
};

struct Bar<int>
{
 enum { bar = 42 };
};

// This is a way to generate a compile error if a value is non-true
// sizeof(test_result<true>) compiles whilst sizeof(test_result<false>) won't
template <bool B>
struct test_result;

struct test_result<true>
{ };

int main()
{
	return sizeof(test_result<bool(Foo<int, Bar>::bar == 42)>);
}
