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
#include <utility>

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

	void addToFrontier(int x, int y);
	double getPitProb(int x, int y);
	void updatePitProbs(int x, int y);
	//void calcPosProb(vector< pair<int,int> > frontBreezeList, int br, set< pair<int,int> > frontierSet, set< pair<int,int> >::iterator fr_it, vector< vector<bool> > pitPresent, double currProb,  double &sum);
	void calcProbs(vector< pair<int,int> > frontierList, int fl, vector< pair<int,int> > frontBreezeList, double currProb, double &sum, vector< vector<bool> > pitPresent);
	bool consistentSetting(vector< pair<int,int> > frontBreezeList, vector< vector<bool> > pitPresent);

	WorldState currentState;
	Action lastAction;
	Percept lastPercept;
	list<Action> actionList;
	int numActions;
	map<int, map<int, bool> > stench_loc;
	map<int, map<int, bool> > breeze_loc;

	set< pair<int, int> > knownSet, breezeSet, frontierSet;
	vector< vector<int> > breezes;
	vector< vector<double> > pits;

	SearchEngine searchEngine;
};

#endif // AGENT_H
