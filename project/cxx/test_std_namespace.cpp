// Are the stdc++ system headers available under the right file names and
// under the right namespace?

#include <map>
#include <string>
#include <set>

int main()
{
	typedef std::map<std::string, std::set<std::string> > Crazy;
	Crazy foo;
	return 0;
}
