#ifndef _MODELER_LSYSTEM
#define _MODELER_LSYSTEM

#include <list>
#include <vector>
#include <map>
#include <string>

class LSystem
{
public:
	LSystem(const std::map<int, std::list<int> >&, const std::list<int>&, const std::map<int, std::pair<int, int> >&);
	std::list<int>* ops;
	void runIteration(int it);
	std::map<int, std::pair<int, int> > opMap;
private:
	std::map<int, std::list<int> > opIterMap;
	std::list<int>* initOps;
	int prevIter;
};

#endif // _MODELER_LSYSTEM