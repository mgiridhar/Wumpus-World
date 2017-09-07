// Agent.cc

#include <iostream>
#include <ctime>
#include "Agent.h"

using namespace std;

Agent::Agent ()
{
	srand((int) time(0)); // to generate different random numbers when run every time
	//WorldState currentState;
}

Agent::~Agent ()
{

}

void Agent::Initialize ()
{
	position = Location(1, 1);
	has_gold = false;
	has_arrow = true;
	orientation = RIGHT;
	has_bumped_before = false;
}

Action Agent::Process (Percept& percept)
{
	Action action;

	//logic for going forward after getting bumped
	if(has_bumped_before) {
		action = GOFORWARD;
		updateLocation(false);
		has_bumped_before = false;
		return action;
	}

	//reflexes for model-based reflex agent
	if(percept.Glitter) {
		action = GRAB;
		has_gold = true;
	}
	else if(position.X == 1 && position.Y == 1 && has_gold) {
		action = CLIMB;
	}
	else if(percept.Stench && has_arrow) {
		action = SHOOT;
		has_arrow = false;
	}
	else if(percept.Bump) {
		updateLocation(true);
		action = TURNLEFT;
		updateOrientation(action);
		//logic for going forward...
		has_bumped_before = true;
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
		this->orientation = (this->orientation == DOWN) ? RIGHT : Orientation(this->orientation + 1);
	}
	else if(action == TURNRIGHT) {
		this->orientation = (this->orientation == RIGHT) ? DOWN : Orientation(this->orientation - 1);
	}
}

void Agent::updateLocation (bool bumped) {
	int step = (bumped) ? -1 : 1;
	switch(this->orientation) {
		case UP:
			this->position.Y += step; 
			break;
		case RIGHT:
		       	this->position.X += step;
			break;
		case DOWN:
			this->position.Y -= step;
			break;
		case LEFT:
			this->position.X -= step;
			break;
		default:;
	}
}

void Agent::GameOver (int score)
{

}

