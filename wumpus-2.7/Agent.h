// Agent.h
//
// HW2

#ifndef AGENT_H
#define AGENT_H

#include "Action.h"
#include "Percept.h"

#include "Location.h"
#include "Orientation.h"
#include "Search.h"
#include <list>
#include <map>
#include <set>

#include "WorldState.h"

class Agent
{
public:
	Agent ();
	~Agent ();
	void Initialize ();
	Action Process (Percept& percept);
	void GameOver (int score);

	void UpdateState (Percept& percept);
	bool FacingDeath();
	int Stench(int x, int y);
	int Breeze(int x, int y);
	void Infer(int x, int y);

	WorldState currentState;
	Action lastAction;
	Percept lastPercept;
	list<Action> actionList;
	int numActions;
	map<int, map<int, bool> > stench_loc;
	map<int, map<int, bool> > breeze_loc;

	SearchEngine searchEngine;
};

#endif // AGENT_H
