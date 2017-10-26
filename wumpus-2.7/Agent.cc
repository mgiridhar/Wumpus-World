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
	breezes = vector< vector<int> >(5, vector<int>(5, -1));
	pits = vector< vector<double> >(5, vector<double>(5, 0.2));
	pits[1][1] = 0.0;
	knownSet.insert(make_pair(1,1));
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
	
	/*
	 * Logical Inference *
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
	*/

	if(percept.Breeze) {
		if(breezeSet.find(make_pair(currentState.agentLocation.X, currentState.agentLocation.Y)) == breezeSet.end()) {
			breezeSet.insert(make_pair(currentState.agentLocation.X, currentState.agentLocation.Y));
			breezes[currentState.agentLocation.X][currentState.agentLocation.Y] = 1;
			addToFrontier(currentState.agentLocation.X, currentState.agentLocation.Y);
		}	
	}
	else {
		if(breezeSet.find(make_pair(currentState.agentLocation.X, currentState.agentLocation.Y)) == breezeSet.end()) {
			breezeSet.insert(make_pair(currentState.agentLocation.X, currentState.agentLocation.Y));
			breezes[currentState.agentLocation.X][currentState.agentLocation.Y] = 0;
			updatePitProbs(currentState.agentLocation.X, currentState.agentLocation.Y);
		}
	}
	
	if(knownSet.find(make_pair(currentState.agentLocation.X, currentState.agentLocation.Y)) == knownSet.end()) {
		knownSet.insert(make_pair(currentState.agentLocation.X, currentState.agentLocation.Y));
		pits[currentState.agentLocation.X][currentState.agentLocation.Y] = 0.0;
		if(frontierSet.find(make_pair(currentState.agentLocation.X, currentState.agentLocation.Y)) != frontierSet.end())
			frontierSet.erase(make_pair(currentState.agentLocation.X, currentState.agentLocation.Y));
	}

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
	cin.get();
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

	//check probability of pit in this location
	double prob = getPitProb(x,y);
	cout << "Probability of pit ahead: " << prob << endl;
	if(prob >= 0.5)
		return true;

    return false;
}

void Agent::addToFrontier(int x, int y) {
	if(y-1 > 0 && knownSet.find(make_pair(x, y-1)) == knownSet.end() && frontierSet.find(make_pair(x,y-1)) == frontierSet.end()) {
		frontierSet.insert(make_pair(x, y-1));
	}
	if(x-1 > 0 && knownSet.find(make_pair(x-1, y)) == knownSet.end() && frontierSet.find(make_pair(x-1,y)) == frontierSet.end()) {
		frontierSet.insert(make_pair(x-1, y));
	}
	if(y+1 < 5 && knownSet.find(make_pair(x, y+1)) == knownSet.end() && frontierSet.find(make_pair(x,y+1)) == frontierSet.end()) {
		frontierSet.insert(make_pair(x, y+1));
	}
	if(x+1 < 5 && knownSet.find(make_pair(x+1, y)) == knownSet.end() && frontierSet.find(make_pair(x+1,y)) == frontierSet.end()) {
		frontierSet.insert(make_pair(x+1, y));
	}
}

void Agent::updatePitProbs(int x, int y) {
	if(y-1 > 0) {
	       	pits[x][y-1] = 0.0;	
		if(knownSet.find(make_pair(x, y-1)) == knownSet.end())
		       knownSet.insert(make_pair(x, y-1));
		if(frontierSet.find(make_pair(x,y-1)) != frontierSet.end())
			frontierSet.erase(make_pair(x, y-1));
	}
	if(x-1 > 0) { 
	       	pits[x-1][y] = 0.0;	
		if(knownSet.find(make_pair(x-1, y)) == knownSet.end())
		       knownSet.insert(make_pair(x-1, y));
		if(frontierSet.find(make_pair(x-1,y)) != frontierSet.end())
			frontierSet.erase(make_pair(x-1, y));
	}
	if(y+1 < 5) { 
	       	pits[x][y+1] = 0.0;	
		if(knownSet.find(make_pair(x, y+1)) == knownSet.end())
		       knownSet.insert(make_pair(x, y+1));
		if(frontierSet.find(make_pair(x,y+1)) != frontierSet.end())
			frontierSet.erase(make_pair(x, y+1));
	}
	if(x+1 < 5) { 
	       	pits[x+1][y] = 0.0;
		if(knownSet.find(make_pair(x+1, y)) == knownSet.end())
		       knownSet.insert(make_pair(x+1, y));
		if(frontierSet.find(make_pair(x+1,y)) != frontierSet.end())
			frontierSet.erase(make_pair(x+1, y));
	}
}

double Agent::getPitProb(int x, int y) {
	if(x <= 0 || x >= 5 || y <= 0 || y >= 5 )
		return 0.0;
	if(knownSet.find(make_pair(x,y)) != knownSet.end()) 
		return pits[x][y];
	if(frontierSet.find(make_pair(x,y)) != frontierSet.end()) {
		double yes = 0.2, no = 0.8;
		set< pair<int,int> >::iterator it;
		set< pair<int,int> > tempBreezeSet;
		//pair<int, int> startBreeze;
		for(it = frontierSet.begin(); it != frontierSet.end(); it++) {
			if(breezeSet.find(make_pair(it->first, it->second-1)) != breezeSet.end() && breezes[it->first][it->second-1] == 1) {
				tempBreezeSet.insert(make_pair(it->first, it->second-1));
				//if(it->first == x && it->second == y)
				//	startBreeze = make_pair(it->first, it->second-1);
			}
			if(breezeSet.find(make_pair(it->first-1, it->second)) != breezeSet.end() && breezes[it->first-1][it->second] == 1) {
				tempBreezeSet.insert(make_pair(it->first-1, it->second));
				//if(it->first == x && it->second == y)
				//	startBreeze = make_pair(it->first-1, it->second);
			}
			if(breezeSet.find(make_pair(it->first, it->second+1)) != breezeSet.end() && breezes[it->first][it->second+1] == 1) {
				tempBreezeSet.insert(make_pair(it->first, it->second+1));
				//if(it->first == x && it->second == y)
				//	startBreeze = make_pair(it->first, it->second+1);
			}
			if(breezeSet.find(make_pair(it->first+1, it->second)) != breezeSet.end() && breezes[it->first+1][it->second] == 1) {
				tempBreezeSet.insert(make_pair(it->first+1, it->second));
				//if(it->first == x && it->second == y)
				//	startBreeze = make_pair(it->first+1, it->second);
			}
		}
		frontierSet.erase(make_pair(x,y));
		vector< pair<int, int> > frontBreezeList(tempBreezeSet.begin(), tempBreezeSet.end()), frontierList(frontierSet.begin(), frontierSet.end());
		/*frontBreezeList.push_back(startBreeze);
		for(set< pair<int, int> >::iterator b = tempBreezeSet.begin(); b != tempBreezeSet.end(); b++) {
			frontBreezeList.push_back(*b);
		}*/
		
		/*cout << "Calculating probabilities.. " << endl;
		cout << "Breezes " << endl;
		//frontiers and breezes
		for(int i=0; i<frontBreezeList.size(); i++) {
			cout << frontBreezeList[i].first << " " << frontBreezeList[i].second << endl;
		}
		cout << "Frontiers " << endl;
		for(it = frontierSet.begin(); it != frontierSet.end(); it++) {
			cout << it->first << " " << it->second << endl;
		}
		*/

		vector< vector<bool> > pitPresent(6, vector<bool>(6, false));
		pitPresent[x][y] = true;
		double sum = 0.0;
		//calcPosProb(frontBreezeList, 0, frontierSet, frontierSet.begin(), pitPresent, 1, sum);
		calcProbs(frontierList, 0, frontBreezeList, 1, sum, pitPresent);
		yes = yes * sum;
		//cout << "Yes: " << yes << endl;

		pitPresent = vector<vector<bool> >(6, vector<bool>(6, false));
		pitPresent[x][y] = false;
		sum = 0.0;
		//calcPosProb(frontBreezeList, 0, frontierSet, frontierSet.begin(), pitPresent, 1, sum);
		calcProbs(frontierList, 0, frontBreezeList, 1, sum, pitPresent);
		no = no * sum;
		//cout << "No: " << no << endl;

		frontierSet.insert(make_pair(x,y));

		return yes / (yes + no);
	}
	return 0.2;
}

void Agent::calcProbs(vector< pair<int,int> > frontierList, int fl, vector< pair<int,int> > frontBreezeList, double currProb, double &sum, vector< vector<bool> > pitPresent) {
	if(fl >= frontierList.size()) {
		if(consistentSetting(frontBreezeList, pitPresent)) {
			sum += currProb;
		}
		return;
	}
	
	pitPresent[frontierList[fl].first][frontierList[fl].second] = true;
	calcProbs(frontierList, fl+1, frontBreezeList, currProb * 0.2, sum, pitPresent);

	pitPresent[frontierList[fl].first][frontierList[fl].second] = false;
	calcProbs(frontierList, fl+1, frontBreezeList, currProb * 0.8, sum, pitPresent);
}

bool Agent::consistentSetting(vector< pair<int,int> > frontBreezeList, vector< vector<bool> > pitPresent) {
	for(int i=0; i<frontBreezeList.size(); i++) {
		int x = frontBreezeList[i].first;
		int y = frontBreezeList[i].second;
		if(!(pitPresent[x][y-1] || pitPresent[x-1][y] || pitPresent[x][y+1] || pitPresent[x+1][y]))
			return false;
	}
	return true;
}

/*
void Agent::calcPosProb(vector< pair<int,int> > frontBreezeList, int br, set< pair<int,int> > frontierSet, set< pair<int,int> >::iterator fr_it, vector< vector<bool> > pitPresent, double currProb,  double &sum) {
	if(br >= frontBreezeList.size()) {
		sum += currProb;
		return;
	}
	if(fr_it == frontierSet.end()) {
		return;
	}
	if(pitPresent[fr_it->first][fr_it->second]) {
		//is near?
		pair<int, int> temp = frontBreezeList[br];
		if((temp.first == fr_it->first && temp.second-1 == fr_it->second) || (temp.first-1 == fr_it->first && temp.second == fr_it->second) || (temp.first == fr_it->first && temp.second+1 == fr_it->second) || (temp.first+1 == fr_it->first && temp.second == fr_it->second))
			calcPosProb(frontBreezeList, br+1, frontierSet, next(fr_it), pitPresent, currProb, sum);
		else
			calcPosProb(frontBreezeList, br, frontierSet, next(fr_it), pitPresent, currProb, sum);

		return;
	}

	if(frontBreezeList[br].first == fr_it->first && frontBreezeList[br].second-1 == fr_it->second) {
		calcPosProb(frontBreezeList, br, frontierSet, next(fr_it), pitPresent, currProb * 0.8, sum);
		pitPresent[fr_it->first][fr_it->second] = true;
		calcPosProb(frontBreezeList, br+1, frontierSet, next(fr_it), pitPresent, currProb * 0.2, sum);
		pitPresent[fr_it->first][fr_it->second] = false;
	}
	if(frontBreezeList[br].first-1 == fr_it->first && frontBreezeList[br].second == fr_it->second) {
		calcPosProb(frontBreezeList, br, frontierSet, next(fr_it), pitPresent, currProb * 0.8, sum);
		pitPresent[fr_it->first][fr_it->second] = true;
		calcPosProb(frontBreezeList, br+1, frontierSet, next(fr_it), pitPresent, currProb * 0.2, sum);
		pitPresent[fr_it->first][fr_it->second] = false;
	}
	if(frontBreezeList[br].first == fr_it->first && frontBreezeList[br].second+1 == fr_it->second) {
		calcPosProb(frontBreezeList, br, frontierSet, next(fr_it), pitPresent, currProb * 0.8, sum);
		pitPresent[fr_it->first][fr_it->second] = true;
		calcPosProb(frontBreezeList, br+1, frontierSet, next(fr_it), pitPresent, currProb * 0.2, sum);
		pitPresent[fr_it->first][fr_it->second] = false;
	}
	if(frontBreezeList[br].first+1 == fr_it->first && frontBreezeList[br].second == fr_it->second) {
		calcPosProb(frontBreezeList, br, frontierSet, next(fr_it), pitPresent, currProb * 0.8, sum);
		pitPresent[fr_it->first][fr_it->second] = true;
		calcPosProb(frontBreezeList, br+1, frontierSet, next(fr_it), pitPresent, currProb * 0.2, sum);
		pitPresent[fr_it->first][fr_it->second] = false;
	}
}
*/
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
		if(knownSet.find(make_pair(x,y)) == knownSet.end()) {
			knownSet.insert(make_pair(x,y));
			pits[x][y] = 1.0;
			if(frontierSet.find(make_pair(x,y)) != frontierSet.end())
				frontierSet.erase(make_pair(x,y));	
		}
		//searchEngine.RemoveSafeLocation(x, y);
	    }
	    if (lastPercept.Stench && (! lastPercept.Breeze)) {
	    	currentState.wumpusLocation = Location(x,y);
	    }
	}
}



