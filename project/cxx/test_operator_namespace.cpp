namespace foo
{
	struct baz { int x; };
	
	int operator % (baz x, int y) { return x.x + y; }
}

int main()
{
	foo::baz q = { 4 };
	return q % 4;
}
