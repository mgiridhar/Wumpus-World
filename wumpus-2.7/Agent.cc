// Agent.cc
//
// HW2

#include <iostream>
#include <list>
#include <vector>
#include "Agent.h"

using namespace std;

Agent::Agent ()
{
	currentState.worldSize = 2; // at least 2x2
	currentState.wumpusLocation = Location(0,0); // unknown
	currentState.goldLocation = Location(0,0); // unknown
}

Agent::~Agent ()
{

}

void Agent::Initialize ()
{
	currentState.agentLocation = Location(1,1);
	currentState.agentOrientation = RIGHT;
	currentState.agentHasArrow = true;
	currentState.agentHasGold = false;
	currentState.wumpusAlive = true;
	actionList.clear();
	searchEngine.AddSafeLocation(1,1); // (1,1) always safe
	// At start of game, wumpus is alive
	searchEngine.RemoveSafeLocation(currentState.wumpusLocation.X, currentState.wumpusLocation.Y);
	lastAction = CLIMB; // dummy action
	numActions = 0;

	//Pit and Wumpus Locations	
	vector<Location>::iterator itr;
	cout << "Pit Locations if inferred.." << endl;
	for (itr = currentState.pitLocations.begin(); itr != currentState.pitLocations.end(); itr++) {
		cout << "(" << itr->X << "," << itr->Y << ")" << endl;
	}
	cout << "Wumpus Location if inferred.." << endl;
	if (!(currentState.wumpusLocation == Location(0,0))) {
		cout << "(" << currentState.wumpusLocation.X << "," << currentState.wumpusLocation.Y << ")" << endl;
	}
}

Action Agent::Process (Percept& percept)
{
	Action action;
	list<Action> actionList2;
	bool foundPlan = false;

	lastPercept = percept;
	UpdateState(percept);
	
	if(percept.Stench) {
		stench_loc[currentState.agentLocation.X][currentState.agentLocation.Y] = true;
	}
	else {
		stench_loc[currentState.agentLocation.X][currentState.agentLocation.Y] = false;
	}
	if(percept.Breeze) {
		breeze_loc[currentState.agentLocation.X][currentState.agentLocation.Y] = true;
	}
	else {
		breeze_loc[currentState.agentLocation.X][currentState.agentLocation.Y] = false;
	}
	Infer(currentState.agentLocation.X, currentState.agentLocation.Y);

	if (actionList.empty()) {
		foundPlan = false;
		if ((! foundPlan) && percept.Glitter) {
			actionList.push_back(GRAB);
			foundPlan = true;
		}
		if ((! foundPlan) && currentState.agentHasGold && (currentState.agentLocation == Location(1,1))) {
			actionList.push_back(CLIMB);
			foundPlan = true;
		}
		if ((! foundPlan) && (! (currentState.goldLocation == Location(0,0))) && (! currentState.agentHasGold)) {
			// If know gold location, but don't have it, then find path to it
			actionList2 = searchEngine.FindPath(currentState.agentLocation, currentState.agentOrientation, currentState.goldLocation, UP);
			if (actionList2.size() > 0) {
				actionList.splice(actionList.end(), actionList2);
				foundPlan = true;
			}
		}
		if ((! foundPlan) && currentState.agentHasGold) {
			// If have gold, then find path to (1,1)
			actionList2 = searchEngine.FindPath(currentState.agentLocation, currentState.agentOrientation, Location(1,1), DOWN);
			if (actionList2.size() > 0) {
				actionList.splice(actionList.end(), actionList2);
				foundPlan = true;
			}
		}
		if ((! foundPlan) && percept.Stench && currentState.agentHasArrow) {
			actionList.push_back(SHOOT);
			foundPlan = true;
		}
		if ((! foundPlan) && percept.Bump) {
			actionList.push_back(TURNLEFT);
			actionList.push_back(GOFORWARD);
			foundPlan = true;
		}
		if (! foundPlan) {
			// Random move
			action = (Action) (rand() % 3);
			while((action == GOFORWARD) && FacingDeath()) {
				cout << "Going Forward is danger... Changing Action" << endl;
				action = (Action) (rand() % 3);
			}
			actionList.push_back(action);
			foundPlan = true;
		}
	}
	action = actionList.front();
	actionList.pop_front();
	// One final check that we aren't moving to our death
    	if ((action == GOFORWARD) && FacingDeath()) {
		cout << "Going Forward is danger... Changing Action" << endl;
    		action = TURNLEFT;
    	}
	lastAction = action;
	numActions++;
	//cin.get();
	return action;
}

void Agent::UpdateState (Percept& percept)
{
	// Check if wumpus killed
	if (percept.Scream)
	{
		currentState.wumpusAlive = false;
		// Since only kill wumpus point-blank, we know its location is in front of agent
		currentState.wumpusLocation = currentState.agentLocation;
		switch (currentState.agentOrientation)
		{
			case RIGHT: currentState.wumpusLocation.X++; break;
			case UP: currentState.wumpusLocation.Y++; break;
			case LEFT: currentState.wumpusLocation.X--; break;
			case DOWN: currentState.wumpusLocation.Y--; break;
		}
	}
	// Check if have gold
	if (lastAction == GRAB)
	{
		currentState.agentHasGold = true;
		currentState.goldLocation = currentState.agentLocation;
	}
	// Check if used arrow
	if (lastAction == SHOOT)
	{
		currentState.agentHasArrow = false;
	}
	// Update orientation
	if (lastAction == TURNLEFT)
	{
		currentState.agentOrientation = (Orientation) ((currentState.agentOrientation + 1) % 4);
	}
	if (lastAction == TURNRIGHT)
	{
		currentState.agentOrientation = (Orientation) ((currentState.agentOrientation + 3) % 4);
	}
	// Update location
	if ((lastAction == GOFORWARD) && (! percept.Bump))
	{
		switch (currentState.agentOrientation)
		{
			case RIGHT: currentState.agentLocation.X++; break;
			case UP: currentState.agentLocation.Y++; break;
			case LEFT: currentState.agentLocation.X--; break;
			case DOWN: currentState.agentLocation.Y--; break;
		}
	}
	// Update world size
	if (currentState.agentLocation.X > currentState.worldSize)
	{
		currentState.worldSize = currentState.agentLocation.X;
	}
	if (currentState.agentLocation.Y > currentState.worldSize)
	{
		currentState.worldSize = currentState.agentLocation.Y;
	}
	// Update safe locations in search engine
	int x = currentState.agentLocation.X;
	int y = currentState.agentLocation.Y;
	searchEngine.AddSafeLocation(x,y);
	if ((! percept.Breeze) && ((! percept.Stench) || (! currentState.wumpusAlive)))
	{
		if (x > 1) searchEngine.AddSafeLocation(x-1,y);
		if (y > 1) searchEngine.AddSafeLocation(x,y-1);
		// Add safe location to the right and up, if doesn't exceed our current estimate of world size
		if (x < currentState.worldSize) searchEngine.AddSafeLocation(x+1,y);
		if (y < currentState.worldSize) searchEngine.AddSafeLocation(x,y+1);
	}
}

bool Agent::FacingDeath()
{
	int x = currentState.agentLocation.X;
	int y = currentState.agentLocation.Y;
	Orientation orientation = currentState.agentOrientation;
	if (orientation == RIGHT) {
		x++;
	}
	if (orientation == UP) {
		y++;
	}
	if (orientation == LEFT) {
		x--;
	}
	if (orientation == DOWN) {
		y--;
	}
	vector<Location>::iterator itr;
	Location facingLoc = Location(x,y);
	for (itr = currentState.pitLocations.begin(); itr != currentState.pitLocations.end(); itr++) {
		if (*itr == facingLoc) {
			return true;
		}
	}
	if ((currentState.wumpusLocation == facingLoc) && currentState.wumpusAlive) {
		return true;
	}
    return false;
}

// 0 - false
// 1 - true
// 2 - unknown

int Agent::Stench(int x, int y) {
	if(stench_loc.find(x) != stench_loc.end() && stench_loc[x].find(y) != stench_loc[x].end()) {
		return stench_loc[x][y] ? 1 : 0;
	}
	return 2;
}

int Agent::Breeze(int x, int y) {
	if(breeze_loc.find(x) != breeze_loc.end() && breeze_loc[x].find(y) != breeze_loc[x].end()) {
		return breeze_loc[x][y] ? 1 : 0;
	}
	return 2;
}

void Agent::Infer(int x, int y) {
	if(currentState.wumpusLocation == Location(0,0))
	{
		//inferring wumpus location
		if(Stench(x,y) < 2 && Stench(x+1,y+1) < 2 && Stench(x,y+1) < 2 && Stench(x,y) && Stench(x+1,y+1) && !Stench(x,y+1)) {
			currentState.wumpusLocation = Location(x+1,y);
		}
		else if(Stench(x,y) < 2 && Stench(x+1,y+1) < 2 && Stench(x+1,y) < 2 && Stench(x,y) && Stench(x+1,y+1) && !Stench(x+1,y)) {
			currentState.wumpusLocation = Location(x,y+1);
		}
		else if(Stench(x,y) < 2 && Stench(x+1,y-1) < 2 && Stench(x,y-1) < 2 && Stench(x,y) && Stench(x+1,y-1) && !Stench(x,y-1)) {
			currentState.wumpusLocation = Location(x+1,y);
		}
		else if(Stench(x,y)<2 && Stench(x-1,y+1)<2 && Stench(x,y+1)<2 && Stench(x,y) && Stench(x-1,y+1) && !Stench(x,y+1)) {
			currentState.wumpusLocation = Location(x-1,y);
		}	
	}

	//inferring pit locations
	if(Breeze(x,y)<2 && Breeze(x-1,y)<2 && Breeze(x,y+1)<2 && Breeze(x+1,y)<2 && Breeze(x,y) && !Breeze(x-1,y) && !Breeze(x,y+1) && !Breeze(x+1,y)) {
	    	currentState.pitLocations.push_back(Location(x,y-1));
	}
	if(Breeze(x,y)<2 && Breeze(x-1,y)<2 && Breeze(x,y+1)<2 && Breeze(x,y-1)<2 && Breeze(x,y) && !Breeze(x-1,y) && !Breeze(x,y+1) && !Breeze(x,y-1)) {
		currentState.pitLocations.push_back(Location(x+1,y));
	}
	if(Breeze(x,y)<2 && Breeze(x-1,y)<2 && Breeze(x,y-1)<2 && Breeze(x+1,y)<2 && Breeze(x,y) && !Breeze(x-1,y) && !Breeze(x,y-1) && !Breeze(x+1,y)) {
		currentState.pitLocations.push_back(Location(x,y+1));
	}
	if(Breeze(x,y)<2 && Breeze(x,y-1)<2 && Breeze(x+1,y)<2 && Breeze(x,y+1)<2 && Breeze(x,y) && !Breeze(x,y-1) && !Breeze(x+1,y) && !Breeze(x,y+1)) {
		currentState.pitLocations.push_back(Location(x-1,y));
	}
}

void Agent::GameOver (int score)
{
	if ((score < 0) and (numActions < 1000)) {
		// Agent died by GOFORWARD into location with wumpus or pit, so make that location unsafe
		int x = currentState.agentLocation.X;
		int y = currentState.agentLocation.Y;
	    Orientation orientation = currentState.agentOrientation;
	    if (orientation == RIGHT) {
	    	x++;
	    }
	    if (orientation == UP) {
	    	y++;
	    }
	    if (orientation == LEFT) {
	    	x--;
	    }
	    if (orientation == DOWN) {
	    	y--;
	    }
	    if (lastPercept.Breeze && (! lastPercept.Stench)) {
	    	currentState.pitLocations.push_back(Location(x,y));
		//searchEngine.RemoveSafeLocation(x, y);
	    }
	    if (lastPercept.Stench && (! lastPercept.Breeze)) {
	    	currentState.wumpusLocation = Location(x,y);
	    }
	}
}



