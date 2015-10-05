#include "modelerglobals.h"
#include "animation.h"
#include <vector>
#include "Utils.h"
#include <string>
#include <algorithm>

using namespace std;

vector<AnimationDef*>* kumaAnimes = nullptr;

vector<string> animations = { "15,0,19,0;15,3,19,-3;15,6,19,-6;15,9,19,-9;15,12,19,-12;15,15,19,-15;15,18,19,-18;15,21,19,-21;15,24,19,-24;15,27,19,-27;15,30,19,-30;15,33,19,-33;15,36,19,-36;15,39,19,-39;15,42,19,-42;15,45,19,-45;15,45,19,-45;15,42,19,-42;15,39,19,-39;15,36,19,-36;15,33,19,-33;15,30,19,-30;15,27,19,-27;15,24,19,-24;15,21,19,-21;15,18,19,-18;15,15,19,-15;15,12,19,-12;15,9,19,-9;15,6,19,-6;15,3,19,-3;15,0,19,0;19,0,15,0;19,3,15,-3;19,6,15,-6;19,9,15,-9;19,12,15,-12;19,15,15,-15;19,18,15,-18;19,21,15,-21;19,24,15,-24;19,27,15,-27;19,30,15,-30;19,33,15,-33;19,36,15,-36;19,39,15,-39;19,42,15,-42;19,45,15,-45;19,45,15,-45;19,42,15,-42;19,39,15,-39;19,36,15,-36;19,33,15,-33;19,30,15,-30;19,27,15,-27;19,24,15,-24;19,21,15,-21;19,18,15,-18;19,15,15,-15;19,12,15,-12;19,9,15,-9;19,6,15,-6;19,3,15,-3;19,0,15,0" };

/*
Convert a string to an animation
ANIMATION -> FRAME | ANIMATION; FRAME
FRAME -> INT, DOUBLE | FRAME, FRAME

Spaces will be discarded.
*/
AnimationDef* stringToAnimation(string s)
{
	remove_if(s.begin(), s.end(), [](char c)->bool { return c == ' '; });

	AnimationDef* anime = new AnimationDef;
	AnimationFrame* oneFrame = nullptr;

	auto sv = Utils::split(s, ';', false);
	for (auto frameS : sv)
	{
		oneFrame = new AnimationFrame;

		auto valS = Utils::split(frameS, ',', false);
		for (int i = 1; i < valS.size(); i += 2)
		{
			oneFrame->push_back(make_pair(stoi(valS[i - 1]), stod(valS[i])));
		}
		
		anime->push_back(oneFrame);
	}

	return anime;
}

void kumaAnimationsSetup()
{
	if (kumaAnimes != nullptr)
	{
		return;
	}

	kumaAnimes = new vector<AnimationDef*>;

	AnimationDef* oneAnime = nullptr;
	for (auto animationS : animations)
	{
		oneAnime = stringToAnimation(animationS);
		kumaAnimes->push_back(oneAnime);
	}
}