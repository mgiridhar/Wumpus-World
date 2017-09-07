// Agent.h

#ifndef AGENT_H
#define AGENT_H

#include "Action.h"
#include "Percept.h"
#include "Location.h"
#include "Orientation.h"

class Agent
{
public:
	Agent ();
	~Agent ();
	void Initialize ();
	Action Process (Percept& percept);
	void updateOrientation(Action& action);
	void updateLocation(bool bumped);
	void GameOver (int score);

	Location position;
	bool has_gold, has_arrow, has_bumped_before;
	Orientation orientation;
};

#endif // AGENT_H
