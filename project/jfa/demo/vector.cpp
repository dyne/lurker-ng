#include <iostream>
#include <jfa.h>

struct IntTraits
{
	typedef int T;
	
	enum { len = sizeof(int) };
	
	static T deserialize(const unsigned char* buf)
	{
		return *((T*)buf);
	}
	
	static void serialize(unsigned char* buf, T i)
	{
		*((T*)buf) = i;
	}
};

int main()
{
	typedef JFA::VectorDB<IntTraits> DB;
	typedef DB::Session              Session;
	
	JFA::Environment env = JFA::Guess("/tmp/");
	DB db(env, "intvect");
	
	JFA_BEGIN_TRANSACTION(t, env)
	Session s = t(db);
	
	int i;
	for (i = 0; i < 1000; ++i) s.push_back(i);
	
	Session::const_iterator e = s.end();
	for (Session::const_iterator i = s.begin(); i != e; ++i)
	{
		std::cout << *i << " ";
	}
	std::cout << std::endl;
	
	try
	{
		JFA_BEGIN_TRANSACTION(t2, env)
		t2(db).size();
		JFA_END_TRANSACTION(t2)
	}
	catch (const JFA::trans_exception& t)
	{
		std::cout << t.desc() << std::endl;
	}
	
	JFA_END_TRANSACTION(t)
	
	return 0;
}
