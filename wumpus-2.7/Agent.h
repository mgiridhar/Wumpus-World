// Agent.h

#ifndef AGENT_H
#define AGENT_H

#include "Action.h"
#include "Percept.h"
#include "WorldState.h"
//#include "Location.h"
//#include "Orientation.h"

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

	WorldState currentState;
	bool hasBumpedBefore;
};

#endif // AGENT_H
