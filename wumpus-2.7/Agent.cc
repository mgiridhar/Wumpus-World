// Agent.cc

#include <iostream>
#include <list>
#include "Agent.h"

using namespace std;

Agent::Agent ()
{
	worldState.worldSize = 1;
	searchEngine.AddSafeLocation(1,1);
}

Agent::~Agent ()
{

}

void Agent::Initialize ()
{
	if(previousAction == GOFORWARD) {
		Move();
		int x = worldState.agentLocation.X;
		int y = worldState.agentLocation.Y;
		if(x > worldState.worldSize || y > worldState.worldSize){
			worldState.worldSize++;
			addSafeLocations(worldState.worldSize);
			//for(int i=1; i<=worldState.worldSize; i++) {
			//	searchEngine.AddSafeLocation(i, worldState.worldSize);
			//	searchEngine.AddSafeLocation(worldState.worldSize, i);
			//}
		}
		if(previousPercept.Stench && worldState.wumpusLocation == Location(0,0)) {
			worldState.wumpusLocation = worldState.agentLocation;
			cout << "Wumpus at (" << worldState.wumpusLocation.X << "," << worldState.wumpusLocation.Y << ")" << endl;
		}
		else if(previousPercept.Breeze) {
			worldState.pitLocations.push_back(worldState.agentLocation);
			cout << "Pit at (" << worldState.agentLocation.X << "," << worldState.agentLocation.Y << ")" << endl;
		}
		searchEngine.RemoveSafeLocation(x, y);
	}

	worldState.agentLocation = Location(1,1);
	worldState.agentOrientation = RIGHT;
	worldState.agentHasArrow = true;
	worldState.agentHasGold = false;
	actionList.clear();
	previousAction = CLIMB; // dummy action to start

}

Action Agent::Process (Percept& percept)
{
	list<Action> actionList2;

	UpdateState(percept);
	if (actionList.empty()) {

		if(!worldState.agentHasGold && !(worldState.goldLocation == Location(0,0))) {			
			// Goto gold location and GRAB
			cout << "Gold at Location (" << worldState.goldLocation.X << "," << worldState.goldLocation.Y << ")" << endl;
			actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, worldState.goldLocation, RIGHT);
			actionList.splice(actionList.end(), actionList2);
			actionList.push_back(GRAB);
		}
		else if(worldState.agentHasGold && !(worldState.agentLocation == Location(1,1))) {
			// Goto (1,1) and CLIMB
			cout << "Agent has Gold. Heading back to (1,1)" << endl;
			actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, Location(1,1), LEFT);
			actionList.splice(actionList.end(), actionList2);
			actionList.push_back(CLIMB);
		}
		else if (percept.Glitter) { // Rule 4a
			worldState.goldLocation = worldState.agentLocation;
			cout << "Gold at Location (" << worldState.goldLocation.X << "," << worldState.goldLocation.Y << ")" << endl;
			actionList.push_back(GRAB);
		} else if (worldState.agentHasGold && (worldState.agentLocation == Location(1,1))) { // Rule 4b
			actionList.push_back(CLIMB);
		} else if (percept.Stench && worldState.agentHasArrow) { // Rule 4c
			actionList.push_back(SHOOT);
		} else if (percept.Bump) { // Rule 4d
			actionList.push_back(TURNLEFT);
			actionList.push_back(GOFORWARD);
		} else { // Rule 4e
			int randomActionIndex = rand() % 3; // 0=GOFORWARD, 1=TURNLEFT, 2=TURNRIGHT
			Action randomAction = (Action) randomActionIndex;
			actionList.push_back(randomAction);
		}
	}
	Action action = actionList.front();
	actionList.pop_front();
	previousAction = action;
	previousPercept = percept;
	//cin.get();
	return action;
}

void Agent::GameOver (int score)
{

}

void Agent::UpdateState (Percept& percept) {
	int orientationInt = (int) worldState.agentOrientation;
	switch (previousAction) {
	case GOFORWARD:
		if (! percept.Bump) {
			Move();
			if(worldState.agentLocation.X > worldState.worldSize || worldState.agentLocation.Y > worldState.worldSize){
				worldState.worldSize++;
				addSafeLocations(worldState.worldSize);
			}
		}
		break;
	case TURNLEFT:
		worldState.agentOrientation = (Orientation) ((orientationInt + 1) % 4);
		break;
	case TURNRIGHT:
		orientationInt--;
		if (orientationInt < 0) orientationInt = 3;
		worldState.agentOrientation = (Orientation) orientationInt;
		break;
	case GRAB:
		worldState.agentHasGold = true; // Only GRAB when there's gold
		break;
	case SHOOT:
		worldState.agentHasArrow = false;
		if(percept.Scream) {
			worldState.wumpusLocation = getNextLocation(worldState.agentLocation, worldState.agentOrientation);
			cout << "Wumpus at (" << worldState.wumpusLocation.X << "," << worldState.wumpusLocation.Y << ")" << endl;
			if(worldState.wumpusLocation.X > worldState.worldSize || worldState.wumpusLocation.Y > worldState.worldSize){
				worldState.worldSize++;
				addSafeLocations(worldState.worldSize);
			}
			searchEngine.RemoveSafeLocation(worldState.wumpusLocation.X, worldState.wumpusLocation.Y);
		}
		break;
	case CLIMB:
		break;
	}
}
	
void Agent::Move() {
	switch (worldState.agentOrientation) {
	case RIGHT:
		worldState.agentLocation.X++;
		break;
	case UP:
		worldState.agentLocation.Y++;
		break;
	case LEFT:
		worldState.agentLocation.X--;
		break;
	case DOWN:
		worldState.agentLocation.Y--;
		break;
	}
}

void Agent::addSafeLocations(int worldSize) {
	for(int i=1; i<=worldSize; i++) {
		searchEngine.AddSafeLocation(i, worldSize);
		searchEngine.AddSafeLocation(worldSize, i);
	}
}

Location Agent::getNextLocation(Location loc, Orientation orient) {
	switch (orient) {
	case RIGHT:
		loc.X++;
		break;
	case UP:
		loc.Y++;
		break;
	case LEFT:
		loc.X--;
		break;
	case DOWN:
		loc.Y--;
		break;
	}
	return loc;
}
