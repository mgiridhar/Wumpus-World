// Agent.cc

#include <iostream>
#include <ctime>
#include "Agent.h"

using namespace std;

Agent::Agent ()
{
	srand((int) time(0)); // to generate different random numbers when run every time
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
	hasBumpedBefore = false;
}

Action Agent::Process (Percept& percept)
{
	Action action;

	//logic for going forward after getting bumped
	if(hasBumpedBefore) {
		action = GOFORWARD;
		updateLocation(false);
		hasBumpedBefore = false;
		return action;
	}

	//reflexes for model-based reflex agent
	if(percept.Glitter) {
		action = GRAB;
		currentState.agentHasGold = true;
	}
	else if(currentState.agentLocation.X == 1 && currentState.agentLocation.Y == 1 && currentState.agentHasGold) {
		action = CLIMB;
	}
	else if(percept.Stench && currentState.agentHasArrow) {
		action = SHOOT;
		currentState.agentHasArrow = false;
	}
	else if(percept.Bump) {
		updateLocation(true);
		action = TURNLEFT;
		updateOrientation(action);
		//logic for going forward...
		hasBumpedBefore = true;
	}
	else {
		// Choose an action randomly from TURNLEFT, TURNRIGHT, GOFORWARD and update orientation or location accordingly
		int choice = (rand() % 3) + 1;
		switch(choice) {
			case 1: action = TURNLEFT;
				updateOrientation(action);
				break;
			case 2: action = TURNRIGHT;
				updateOrientation(action);
				break;
			case 3: action = GOFORWARD;
				updateLocation(false);
				break;
			default:;
		}
	}
	/*
	char c;
	bool validAction = false;

	while (! validAction)
	{
		validAction = true;
		cout << "Action? ";
		cin >> c;
		if (c == 'f') {
			action = GOFORWARD;
		} else if (c == 'l') {
			action = TURNLEFT;
		} else if (c == 'r') {
			action = TURNRIGHT;
		} else if (c == 'g') {
			action = GRAB;
		} else if (c == 's') {
			action = SHOOT;
		} else if (c == 'c') {
			action = CLIMB;
		} else {
			cout << "Huh?" << endl;
			validAction = false;
		}
	}*/
	
	return action;
}

void Agent::updateOrientation (Action& action) {
	//{RIGHT, UP, LEFT, DOWN}
	if(action == TURNLEFT) {
		this->currentState.agentOrientation = (this->currentState.agentOrientation == DOWN) ? RIGHT : Orientation(this->currentState.agentOrientation + 1);
	}
	else if(action == TURNRIGHT) {
		this->currentState.agentOrientation = (this->currentState.agentOrientation == RIGHT) ? DOWN : Orientation(this->currentState.agentOrientation - 1);
	}
}

void Agent::updateLocation (bool bumped) {
	int step = (bumped) ? -1 : 1;
	switch(this->currentState.agentOrientation) {
		case UP:
			this->currentState.agentLocation.Y += step; 
			break;
		case RIGHT:
		       	this->currentState.agentLocation.X += step;
			break;
		case DOWN:
			this->currentState.agentLocation.Y -= step;
			break;
		case LEFT:
			this->currentState.agentLocation.X -= step;
			break;
		default:;
	}
}

void Agent::GameOver (int score)
{

}

