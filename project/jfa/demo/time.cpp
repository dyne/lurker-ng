#include <jfa.h>
#include <iostream>

struct IntTraits
{
	typedef int T;
	
	enum { len = 4 };
	
	static T deserialize(const unsigned char* buf)
	{
		return *((T*)buf);
	}
	
	static void serialize(unsigned char* buf, T i)
	{
		*((T*)buf) = i;
	}
};

using namespace JFA;
using namespace std;

extern int bar(int bar);

int main()
{
	typedef VectorDB<IntTraits> DB;
	typedef DB::Session         Session;
	
	Environment foo = Guess("/tmp");
	DB db(foo, "time");
	
	for (int i = 0; i < 1000; i++)
	{
		JFA_BEGIN_TRANSACTION(t, foo)
		
		Session s = t(db);
		for (int j = 0; j < 10000; j++)
		{
			s.push_back(5);
		}
		
		JFA_END_TRANSACTION(t)
	}
	
	int k = 0;
	
//	JFA_BEGIN_TRANSACTION(t, foo)
//	Session s = t(db);
//	Session::iterator e = s.end();
//	Session::const_iterator i;
//	int j = 0;
//	for (i = s.begin(); i != e; ++i)
//	{
//		j += *i;
//		bar(j);
//		// assert (*i == 4);
////		*i = 6;
//		k++;
//	}
//	JFA_END_TRANSACTION(t)
//	
//	cout << k << endl;

	return 0;
}
