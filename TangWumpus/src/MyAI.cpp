//Bianca Tang - Code begins under "your code begins"
// ======================================================================
// FILE:        MyAI.cpp
//
// AUTHOR:      Abdullah Younis
//
// DESCRIPTION: This file contains your agent class, which you will
//              implement. You are responsible for implementing the
//              'getAction' function and any helper methods you feel you
//              need.
//
// NOTES:       - If you are having trouble understanding how the shell
//                works, look at the other parts of the code, as well as
//                the documentation.
//
//              - You are only allowed to make changes to this portion of
//                the code. Any changes to other portions of the code will
//                be lost when the tournament runs your code.
// ======================================================================
#include "MyAI.hpp"
#include <iostream>

MyAI::MyAI() : Agent()
{
	// ======================================================================
	// YOUR CODE BEGINS
	// ======================================================================
	//When creating a new agent, the first thing to do is to create a starting node and create its first frontier nodes!
	current = new Node(0, 0, nullptr);
	direction = 1; //agent starts out facing east (right)
	arrowShot = false;
	haveArrow = true;
	wumpusDead = false;
	toEntrance = false;
	maxX = 20; //arbitrary value that's greater than max possible board size
	maxY = 20; //arbitrary value that's greater than max possible board size
	turnCounter = 1; //once it hits a certain number turns, go back to start. Probably got stuck in a loop somewhere.
	nodes.push_front(current);
	// ======================================================================
	// YOUR CODE ENDS
	// ======================================================================
}
	
Agent::Action MyAI::getAction
(
	bool stench,
	bool breeze,
	bool glitter,
	bool bump,
	bool scream
)
{
	// ======================================================================
	// YOUR CODE BEGINS
	// ======================================================================
	Agent::Action move;

	if (!toEntrance && turnCounter <= 120) { move = findGold(stench, breeze, glitter, bump, scream); }
	else { move = returnToEntrance(current); }
	//set up any changes for the next turn
	if (move == FORWARD) {
		previous = current;
		if (direction == 0) { current = current->getNorth(); }
		if (direction == 1) {current = current->getEast();}
		if (direction == 2) { current = current->getSouth(); }
		if (direction == 3) { current = current->getWest(); }
	}
	if (move == TURN_RIGHT) {
		if (direction == 3) { direction = 0;}
		else { direction++; }
	}
	if (move == TURN_LEFT) {
		if (direction == 0) { direction = 3; }
		else { direction--; }
	}
	if (move == GRAB) { toEntrance = true; } //we now have the gold.
	if (move == SHOOT) { 
		arrowShot = true;
		haveArrow = false;
	}

	turnCounter++;
	return move;
	// ======================================================================
	// YOUR CODE ENDS
	// ======================================================================
}

// ======================================================================
// YOUR CODE BEGINS
// ======================================================================
Agent::Action MyAI::findGold(bool stench, bool breeze, bool glitter, bool bump, bool scream){
	//First, UPDATE WORLD STATE ACCORDING TO PERCEPTS
	updateWorld(stench, breeze, glitter, bump, scream);
	//AFTER FINISHING ALL WORLD STATE UPDATES POSSIBLE, DECIDE ON AN ACTION
	return decideMove(stench, breeze, glitter, bump, scream);
}

void MyAI::updateWorld(bool stench, bool breeze, bool gold, bool bump, bool scream) {

	if (scream) { 
		wumpusDead = true; 
		arrowShot = false;
		for (Node* node: nodes) { //if the wumpus is dead, iterate through all nodes and set wumpus alert to 0.
			node->wumpusAlert(-1);
			if (!node->wasExplored() && node->isPit() == -1) { frontier.push_front(node); } //ADDED this line 
		}
	}
	if (arrowShot && !scream) { //if the arrow was JUST shot, but no scream was heard, then we can assume that all tiles in the direction we are facing, in a line, are wumpus-free
		arrowShot = false;
		if (direction == 0) { //we shot to the north -> then anything with the same xCoord, but increasing yCoords are wumpus free
			for (Node* node : nodes) {
				if (node->getX() == current->getX() && node->getY() > current->getY()) {
					node->wumpusAlert(-1);
					if (!node->wasExplored() && node->isPit() == -1) { frontier.push_front(node); } //ADDED
				}
			}
		}
		if (direction == 1) { //we shot to the east -> then anything with the same yCoord, but increasing xCoords are wumpus free
			for (Node* node : nodes) {
				if (node->getY() == current->getY() && node->getX() > current->getX()) {
					node->wumpusAlert(-1);
					if (!node->wasExplored() && node->isPit() == -1) { frontier.push_front(node); } //ADDED
				}
			}
		}
		if (direction == 2) { //we shot to the south -> then anything with the same xCoord, but decreasing yCoords are wumpus free
			for (Node* node : nodes) {
				if (node->getX() == current->getX() && node->getY() < current->getY()) {
					node->wumpusAlert(-1);
					if (!node->wasExplored() && node->isPit() == -1) { frontier.push_front(node); } //ADDED
				}
			}
		}
		if (direction == 3) { //we shot to the west -> then anything with the same yCoord, but decreasing xCoords are wumpus free
			for (Node* node : nodes) {
				if (node->getY() == current->getY() && node->getX() < current->getX()) {
					node->wumpusAlert(-1);
					if (!node->wasExplored() && node->isPit() == -1) { frontier.push_front(node); } //ADDED
				}
			}
		}
	}
	if (bump) {
		//if you ran into a wall, set current to the previous node (you can't walk past the wall) (you are still going to face the same direction - towards the found wall)
		current = previous;
		//set maxX or maxY if applicable (ex: you felt a bump and you were facing in direction 0 (north) or 1 (east)
		if (direction == 0) { 
			maxY = current->getY(); 
			trimNodes(); //remove any "nonexistent" out of bounds nodes from nodes. Also remove these nodes from frontier.
		}
		if (direction == 1) { 
			maxX = current->getX();
			trimNodes();
		}
	}
	if (current->wasExplored() == false) {
		//if you JUST moved into the current tile, CREATE ALL NEW FRONTIER NODES. then, take percepts and update tile node
		createFrontier(current);
		//Take percepts and update current tile node, as well as adjacent tile nodes
		current->exploredTrue();
		frontier.remove(current); //remove current node from frontier to show that it has been explored
		if (stench && !wumpusDead) { //will automatically ignore stenches once wumpus is dead
			current->setStench();
			raiseStenchCount(current);
			//any tile with 1-3 adjacent stenches has a Wumpus alert of 1. Any tile with 4 adjStenches has an alert of 2.
			//When a tile goes to Wumpus Level 2, it automatically clears all other tiles of AdjStenches and lowers their Wumpus Alert to -1, as we have found the wumpus
			for (Node* node: nodes) {
				if (node->stenchCt() >= 1) {
					node->wumpusAlert(1);
					frontier.remove(node);
				}
				if (node->stenchCt() == 4) {
					node->wumpusAlert(2);
					frontier.remove(node);
					for (Node* tile : nodes) { //INCOMPLETE - changes that could be made here: instead of going through each one to mark them as safe, just ignore all stenches once wumpusDead
						if (tile->isWumpus() != 2) { 
							tile->wumpusAlert(-1); 
							if (!tile->wasExplored() && tile->isPit() == -1) { frontier.push_front(tile); } //ADDED
						}
					}
				}
			}
		}
		if (!stench && !wumpusDead) { //if there is no stench on the current tile, then we know that every adjacent tile can be set to wumpus = -1 (safe)
			if (current->getEast() != nullptr) {
				current->getEast()->wumpusAlert(-1);
				if (!current->getEast()->wasExplored() && current->getEast()->isPit() == -1) { frontier.push_front(current->getEast()); } //ADDED
			}
			if (current->getNorth() != nullptr) {
				current->getNorth()->wumpusAlert(-1);
				if (!current->getNorth()->wasExplored() && current->getNorth()->isPit() == -1) { frontier.push_front(current->getNorth()); } //ADDED
			}
			if (current->getWest() != nullptr) {
				current->getWest()->wumpusAlert(-1);
				if (!current->getWest()->wasExplored() && current->getWest()->isPit() == -1) { frontier.push_front(current->getWest()); } //ADDED
			}
			if (current->getSouth() != nullptr) {
				current->getSouth()->wumpusAlert(-1);
				if (!current->getSouth()->wasExplored() && current->getSouth()->isPit() == -1) { frontier.push_front(current->getSouth()); } //ADDED
			}
		}
		if (breeze) {
			current->setBreeze();
			raiseBreezeCount(current);
			for (Node* node : nodes) {
				if (node->breezeCt() >= 1 && node->isPit() != -1) {
					node->pitAlert(1);
					frontier.remove(node);
				}
				if (node->breezeCt() == 4) {
					node->pitAlert(2);
					frontier.remove(node);//remove the pit node from frontier, as it is permanently unsafe to go into.
				}
			}
		}
		else { //if there is no breeze on the current tile, then we know that every adjacent tile can be set to pit = -1 (safe)
			if (current->getEast() != nullptr) {
				current->getEast()->pitAlert(-1);
				if (!current->getEast()->wasExplored() && current->getEast()->isWumpus() == -1) {frontier.push_front(current->getEast()); } //ADDED
			}
			if (current->getNorth() != nullptr) {
				current->getNorth()->pitAlert(-1);
				if (!current->getNorth()->wasExplored() && current->getNorth()->isWumpus() == -1) { frontier.push_front(current->getNorth()); } //ADDED
			}
			if (current->getWest() != nullptr) {
				current->getWest()->pitAlert(-1);
				if (!current->getWest()->wasExplored() && current->getWest()->isWumpus() == -1) { frontier.push_front(current->getWest()); } //ADDED
			}
			if (current->getSouth() != nullptr) {
				current->getSouth()->pitAlert(-1);
				if (!current->getSouth()->wasExplored() && current->getSouth()->isWumpus() == -1) { frontier.push_front(current->getSouth()); } //ADDED

			}
		}
		current->pitAlert(-1);
		if (!wumpusDead) {
			current->wumpusAlert(-1);
		}
	}
}

Agent::Action MyAI::decideMove(bool stench, bool breeze, bool gold, bool bump, bool scream) { //UNFINISHED, INCOMPLETE
	if (gold) {
		toEntrance = true; //we've completed the first part of the problem and would now like to go to the entrance
		return GRAB;
	}
	if (breeze && current->getX() == 0 && current->getY() == 0) { return CLIMB; } //if you're on tile one and there's a breeze, it's safer to just leave.
	if (stench && current->getX() == 0 && current->getY() == 0 && haveArrow && !wumpusDead) { 
		arrowShot = true;
		haveArrow = false;
		return SHOOT; 
	} //if you're on the first tile and there's a stench, try to eliminate danger by shooting east. assume that the wumpus is not dead when you observe the stench, and you have an arrow
	if (frontier.size() == 0) { //if frontier is empty, go to entrance (no more nodes to explore)
		toEntrance = true;
		return returnToEntrance(current);
	}

	//-------------------------------------------------------------------------Unsure about the stuff below------------------------------------------------------
	//ITS EXPLORATION TIME!
	else { //there are still unexplored nodes
		if (nodePath.size() == 0 || bump) { //if there's a bump, the destination node is unreachable. find a new node to go to. 
			//if we've emptied out the nodePath -> aka, we've reached our destination frontier node, find a new frontier node, make its path, and return the first step we need to get there
			createNodePath();
		}
		if (nodePath.back() == current) { //if we've reached a node in nodePath, remove the node
			nodePath.pop_back(); //stack of nodes to go through. remove the current node and then turn in the correct direction to get to the next node
			if (nodePath.size() == 0) {
				createNodePath();
				if (toEntrance) return returnToEntrance(current);

				nodePath.pop_back();
			}
		}
		//return an action we need to take to get to the next node in nodePath
		return getTo(current, nodePath.back());
	}
}

void MyAI::createNodePath() {
	Node* minNode = nullptr; //the node we want to go to next
	int minCost = 10000000;
	for (Node* node : frontier) {
		if (nodeCost(current, node) < minCost && node->isWumpus() <= 0) {
			minNode = node;
			minCost = nodeCost(current, node);
		}
	}
	nextNode = minNode;
	if (nextNode == nullptr) {
		toEntrance = true;
		return;
	}
	else {
		nodePath = findPath(current, nextNode);
	}
}

std::list<MyAI::Node*> MyAI::findPath(Node* current, Node* dest) { //returns a list of nodes (each is adjacent to the next) that we must travel through to get from src to dest
	std::set<Node*> explored; //set of nodes that are already explored
	std::set<Node*> unexploredFrontier = { current }; //set of safe nodes that can be explored
	std::map<Node*, Node*> parentOf; //Node* child, Node* parent - parent is the most efficient previous step

	std::map<Node*, int> startToNodeCost; //keeps track of cost from start to the specified node
	startToNodeCost[current] = 0;

	std::map<Node*, int> startThroughNodeCost; //keeps track of cost from start to dest while GOING THROUGH the specified node
	startThroughNodeCost[current] = nodeCost(current, dest); //nodeCost gives manhattan distance

	while (!unexploredFrontier.empty()) { //we still have nodes that can be explored
		Node* minCostNode = nullptr; //the node in unexploredFrontier with the minimum startThroughNodeCost
		int minCost = 10000000;
		for (Node* node : unexploredFrontier) {
			if (startThroughNodeCost.count(node) == 1 && startThroughNodeCost[node] < minCost) { 
				minCost = startThroughNodeCost[node]; 
				minCostNode = node; 
			}
		}
		if (minCostNode == dest) {
			return createPath(parentOf, minCostNode);
		}
		unexploredFrontier.erase(minCostNode);
		explored.insert(minCostNode);
		//check each neighbor (n,s,e,w) of "node"
		checkNeighbor(minCostNode->getNorth(), minCostNode, dest, explored, unexploredFrontier, parentOf, startThroughNodeCost, startToNodeCost);
		checkNeighbor(minCostNode->getSouth(), minCostNode, dest, explored, unexploredFrontier, parentOf, startThroughNodeCost, startToNodeCost);
		checkNeighbor(minCostNode->getEast(), minCostNode, dest, explored, unexploredFrontier, parentOf, startThroughNodeCost, startToNodeCost);
		checkNeighbor(minCostNode->getWest(), minCostNode, dest, explored, unexploredFrontier, parentOf, startThroughNodeCost, startToNodeCost);
	}
}

void MyAI::checkNeighbor( //only called in findPath
	Node* neighbor, 
	Node* current, 
	Node* dest,
	std::set<Node*> &explored, 
	std::set<Node*> &unexplored,
	std::map<Node*, Node*> &parentOf,
	std::map<Node*, int> &startThroughNodeCost,
	std::map<Node*, int> &startToNodeCost
) 
{
	if (neighbor != nullptr && explored.count(neighbor) == 0 && unexplored.count(neighbor) == 0 && neighbor->isWumpus() == -1 && neighbor->isPit() == -1 && neighbor->getX() <= maxX && neighbor->getY() <= maxY) {
		//then we've found a new node
		unexplored.insert(neighbor);
		int startToNode = startToNodeCost[current] + 1; //because each step FORWARD to a new node COSTS 1
		if (startToNodeCost.count(neighbor) == 0 || startToNode < startToNodeCost[neighbor]) {
			//if north isn't a key in startToNodeCost yet, or if it's less than the value, enter it into the map
			startToNodeCost[neighbor] = startToNode;
			startThroughNodeCost[neighbor] = startToNode + nodeCost(neighbor, dest);
			parentOf[neighbor] = current;
		}
	}
}

std::list<MyAI::Node*> MyAI::createPath(std::map<Node*, Node*> map, Node* node) { //only called in findPath
	std::list<Node*> path = {node};
	while (map.count(node) == 1) {//while the node exists as a key of map
		node = map[node];
		path.push_back(node);
	}
	return path;
}

Agent::Action MyAI::getTo(Node* src, Node* dest) { //dest is guaranteed to always be an adjacent node
	if (dest == src->getNorth()) { //if our destination node is to the north
		if (direction == 0) { return FORWARD; }
		if (direction == 1) { return TURN_LEFT; }
		if (direction == 2) { return TURN_LEFT; }
		if (direction == 3) { return TURN_RIGHT; }
	}
	if(dest == src->getEast()){
		if (direction == 0) { return TURN_RIGHT; }
		if (direction == 1) { return FORWARD; }
		if (direction == 2) { return TURN_LEFT; }
		if (direction == 3) { return TURN_LEFT; }
	}
	if (dest == src->getSouth()) {
		if (direction == 0) { return TURN_LEFT; }
		if (direction == 1) { return TURN_RIGHT; }
		if (direction == 2) { return FORWARD; }
		if (direction == 3) { return TURN_LEFT; }
	}
	if (dest == src->getWest()) {
		if (direction == 0) { return TURN_LEFT; }
		if (direction == 1) { return TURN_LEFT; }
		if (direction == 2) { return TURN_RIGHT; }
		if (direction == 3) { return FORWARD; }
	}
}

int MyAI::nodeCost(Node* src, Node* dest) {//find the manhattan distance between the 2 nodes
	return 0;//changed to djikstra's on this line
	int cost = abs(src->getX() - dest->getX()) + abs(src->getY() - dest->getY());
	if (dest->isWumpus() > 0) {
		cost += 1000000;
	}
	if (direction == 0 && dest->getY() < src->getY()) { cost += 2; } //must turn twice to go south if the agent is facing north
	else if (direction == 0 && dest->getX() != src->getX()) { cost += 1; } //if agent faces north, must turn once to go east or west
	else if (direction == 1 && dest->getY() != src->getY()) { cost += 1; } //if agent faces east, must turn once to go N or S
	else if (direction == 1 && dest->getX() < src->getX()) { cost += 2; } //if agent faces east, must turn twice to go W
	else if (direction == 2 && dest->getY() > src->getY()) { cost += 2; } //if agent faces S, must turn twice to go N
	else if (direction == 2 && dest->getX() != src->getX()) { cost += 1; }//if agent faces S, must turn once to go E, W
	else if (direction == 3 && dest->getY() != src->getY()) { cost += 1; }//if agent faces W, must turn once to go N, S
	else if (direction == 3 && dest->getX() > src->getX()) { cost += 2; }//if agent faces W, must turn twice to go E.
	return cost;
}

void MyAI::trimNodes() {
	std::list<Node*>::iterator i = nodes.begin();
	while (i != nodes.end()) {
		if ((*i)->getX() > maxX || (*i)->getY() > maxY) {
			nodes.erase(i++);
		}
		else { i++; }
	}
	std::list<Node*>::iterator j = frontier.begin();
	while (j != frontier.end()) {
		if ((*j)->getX() > maxX || (*j)->getY() > maxY) {
			frontier.erase(j++);
		}
		else { j++; }
	}
}

void MyAI::createFrontier(Node* current) {
	//Create north node
	if (current->getY() + 1 <= maxY) { //make sure we're not creating a node that's outside of known map size bounds
		for (Node* node : nodes) { //check to see if there'a already a node xCoord, yCoord+1
			if (node->getX() == current->getX() && node->getY() == current->getY() + 1) {
				current->setNorth(node);
				node->setSouth(node);
				if (wumpusDead) { node->wumpusAlert(-1); }
			}
		}
		if (current->getNorth() == nullptr) { //no matching node was found. create an empty north node
			Node* newNorth = new Node(current->getX(), current->getY() + 1, current);
			current->setNorth(newNorth);
			newNorth->setSouth(current);
			frontier.push_front(newNorth); //add newly created nodes to frontier, and to nodes
			nodes.push_front(newNorth);
			if (wumpusDead) { newNorth->wumpusAlert(-1); }
		}
	}

	//Create south node
	if (current->getY() - 1 >= 0) {//make sure you don't go below y=0
		for (Node* node : nodes) { //check to see if there'a already a node xCoord, yCoord-1
			if (node->getX() == current->getX() && node->getY() == current->getY() - 1) {
				current->setSouth(node);
				node->setNorth(current);
				if (wumpusDead) { node->wumpusAlert(-1); }
			}
		}
		if (current->getSouth() == nullptr) { //no matching node was found. create an empty south node
			Node* newSouth = new Node(current->getX(), current->getY() - 1, current);
			current->setSouth(newSouth);
			newSouth->setNorth(current);
			frontier.push_front(newSouth);
			nodes.push_front(newSouth);
			if (wumpusDead) { newSouth->wumpusAlert(-1); }
		}
	}

	//Create east node
	if(current->getX() + 1 <= maxX) { //make sure you don't go above known x bounds
		for (Node* node : nodes) { //check to see if there'a already a node xCoord+1, yCoord
			if (node->getX() == current->getX() + 1 && node->getY() == current->getY()) {
				current->setEast(node);
				node->setWest(current);
				if (wumpusDead) { node->wumpusAlert(-1); }
			}
		}
		if(current->getEast() == nullptr) { //no matching node was found. create an empty east node
			Node* newEast = new Node(current->getX() + 1, current->getY(), current);
			current->setEast(newEast);
			newEast->setWest(current);
			frontier.push_front(newEast);
			nodes.push_front(newEast);
			if (wumpusDead) { newEast->wumpusAlert(-1); }
		}
	}

	//create west node
	if(current->getX() - 1 >= 0) {
		for (Node* node : nodes) { //check to see if there'a already a node xCoord-1, yCoord
			if (node->getX() == current->getX() - 1 && node->getY() == current->getY()) {
				current->setWest(node);
				node->setEast(current);
				if (wumpusDead) { node->wumpusAlert(-1); }
				//if this node was already created before, it would already be in Nodes and it would already be in frontier if it were unexplored
			}
		}
		if (current->getWest() == nullptr) { //no matching node was found. create an empty west node
			Node* newWest = new Node(current->getX() - 1, current->getY(), current);
			current->setWest(newWest);
			newWest->setEast(current);
			frontier.push_front(newWest);
			nodes.push_front(newWest);
			if (wumpusDead) { newWest->wumpusAlert(-1); }
		}
	}
}

Agent::Action MyAI::returnToEntrance(Node* current){
	//Should return either FORWARD, TURN_LEFT, or TURN_RIGHT
	if (leftTurnsLeft > 0) {
		leftTurnsLeft--;
		return TURN_LEFT; 
	}
	if (current->getX() == 0 && current->getY() == 0) { return CLIMB; } //get out of the cave
	if (current->getParent() == current->getNorth() && direction != 0) {
		//if the parent of the current node is to the north, but direction != 0 (north) TURN TO THE NORTH
		if (direction == 3) { 
			return TURN_RIGHT; 
		}
		leftTurnsLeft = direction-1;
		return TURN_LEFT;
	}
	if (current->getParent() == current->getEast() && direction != 1) {
		if (direction == 0) {
			return TURN_RIGHT;
		}
		leftTurnsLeft = direction - 2;
		return TURN_LEFT;
	}
	if (current->getParent() == current->getSouth() && direction != 2) {
		if (direction == 1) {
			return TURN_RIGHT;
		}
		if (direction == 0) {
			leftTurnsLeft = 1;
			return TURN_LEFT;
		}
		return TURN_LEFT;
	}
	if (current->getParent() == current->getWest() && direction != 3) {
		if (direction == 2) {
			return TURN_RIGHT;
		}
		if (direction == 0) {
			return TURN_LEFT;
		}
		leftTurnsLeft = 1;
		return TURN_LEFT;
	}
	//if none of the above match, that means that we already face in the direction of the parent node. Just move forward
	return FORWARD;
}

void MyAI::raiseStenchCount(Node* node){ //goes to adjacent nodes in the graph and raises adjacent stench counts accordingly
	if (node->getEast() != nullptr && node->getEast()-> isWumpus() != -1) { //if a node has a stench count of -1, it is safe, and doesn't need to be modified
		node->getEast()->stenchCtInc();
	}
	if (node->getNorth() != nullptr && node->getNorth()->isWumpus() != -1) {
		node->getNorth()->stenchCtInc();
	}
	if (node->getWest() != nullptr && node->getWest()->isWumpus() != -1) {
		node->getWest()->stenchCtInc();
	}
	if (node->getSouth() != nullptr && node->getSouth()->isWumpus() != -1) {
		node->getSouth()->stenchCtInc();
	}
}

void MyAI::raiseBreezeCount(Node* node) { //goes to adjacent nodes in the graph and raises adjecent breeze counts accordingly (except if it is proven to be safe (count is -1))
	if (node->getNorth() != nullptr && node->getNorth()->isPit() != -1) {
		node->getNorth()->breezeCtInc();
	}
	if (node->getSouth() != nullptr && node->getSouth()->isPit() != -1) {
		node->getSouth()->breezeCtInc();
	}
	if (node->getEast() != nullptr && node->getEast()->isPit() != -1) {
		node->getEast()->breezeCtInc();
	}
	if (node->getWest() != nullptr && node->getWest()->isPit() != -1) {
		node->getWest()->breezeCtInc();
	}
}

//NODE CLASS DEFINITIONS
MyAI::Node::Node(int x, int y, Node* parentNode) {
	xCoord = x;
	yCoord = y;
	explored = false;
	parent = parentNode;

	north = nullptr;
	south = nullptr;
	east = nullptr;
	west = nullptr;
	wumpus = 0; //uncertain - no assumptions
	adjStenchCt = 0;
	pit = 0; // -1 = definitely safe, 0 = no assumptions, 1 = possible pit, 2 = definite pit
	adjBreezeCt = 0;
}

MyAI::Node::~Node() {
}

int MyAI::Node::getX() { return xCoord; } //get x coordinate
int MyAI::Node::getY() { return yCoord; } //get y coordinate
bool MyAI::Node::wasExplored() { return explored; } //returns if node is explored or not
int MyAI::Node::breezeCt() { return adjBreezeCt; } //gets adjBreezeCt. When this reaches 4, set pit to 2
int MyAI::Node::stenchCt() { return adjStenchCt; } //gets adjStenchCt. When this reaches 4, set wumpus to 2
int MyAI::Node::isWumpus() { return wumpus; } //gets wumpus. -1 for clear, 1 for possible wumpus, 2 for wumpus
int MyAI::Node::isPit() { return pit; } //gets pit. -1 for clear, 1 for possible pit, 2 for pit

void MyAI::Node::exploredTrue() { explored = true; } //changes explored to True. (can't change to false, can't unexplore)
void MyAI::Node::breezeCtInc() { adjBreezeCt++; } //increase adjBreezeCt by 1
void MyAI::Node::stenchCtInc() { adjStenchCt++; } //increase adjStenchCt by 1
void MyAI::Node::pitAlert(int level) { if (pit > -1 && level >= -1 && level <= 2) { pit = level; } }
void MyAI::Node::setStench() { stench = true; }
void MyAI::Node::setStenchCt(int num) { adjStenchCt = num; }
void MyAI::Node::setBreeze() { breeze = true; }
void MyAI::Node::setBreezeCt(int num) { adjBreezeCt = num; }

//!!!!!DANGER!!!!!WUMPUS ALERT!!!!!DANGER!!!!!
void MyAI::Node::wumpusAlert(int level) { if (wumpus > -1 && level >= -1 && level <= 2) { wumpus = level; } } //change "wumpus" danger level to 0, 1, or 2

MyAI::Node* MyAI::Node::getParent() { return parent; } //returns a pointer to the parent of the node
MyAI::Node* MyAI::Node::getNorth() { return north; }
MyAI::Node* MyAI::Node::getSouth() { return south; }
MyAI::Node* MyAI::Node::getEast() { return east; }
MyAI::Node* MyAI::Node::getWest() { return west; }

void MyAI::Node::setNorth(Node* node) { north = node; }
void MyAI::Node::setSouth(Node* node) { south = node; }
void MyAI::Node::setEast(Node* node) { east = node; }
void MyAI::Node::setWest(Node* node) { west = node; }
// ======================================================================
// YOUR CODE ENDS
// ======================================================================