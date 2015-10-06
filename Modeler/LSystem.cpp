#include "LSystem.h"

#include <list>
#include <vector>
#include <map>

using namespace std;

LSystem::LSystem(const map<int, list<int> >& opIter, const list<int>& opinit, const map<int, std::pair<int, int> >& opM)
{
	opIterMap = opIter;
	ops = new list<int>(opinit);
	initOps = new list<int>(opinit);
	opMap = opM;
	prevIter = 0;
}

void LSystem::runIteration(int it)
{
	if (prevIter == it || it < 0)
	{
		return;
	}
	prevIter = it;

	if (ops)
		delete ops;
	ops = new list<int>(*initOps);

	for (int i = 0; i < it; ++i)
	{
		auto oldOps = ops;
		ops = new list<int>;
		for (auto op : (*oldOps))
		{
			if (opIterMap.find(op) != opIterMap.end())
			{
				ops->insert(ops->end(), opIterMap[op].begin(), opIterMap[op].end());
			}
			else
			{
				ops->insert(ops->end(), op);
			}
		}
		delete oldOps;
	}
}