struct Foo
{
 public:
 	template <int J, class X>
 	X bar(X a, X b) { return a + b + J; }
};

int main()
{
	Foo f;
	return f.bar<3>(4, 5);
}
