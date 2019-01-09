//Bianca Tang -> code begins under "Your code begins"
// ======================================================================
// FILE:        MyAI.hpp
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

#ifndef MYAI_LOCK
#define MYAI_LOCK

#include "Agent.hpp"
#include <list>
#include <map>
#include <set>

class MyAI : public Agent
{
public:
	MyAI ( void );
	
	Action getAction
	(
		bool stench,
		bool breeze,
		bool glitter,
		bool bump,
		bool scream
	);
	
	// ======================================================================
	// YOUR CODE BEGINS - Bianca Tang
	// ======================================================================

private:

	//---------------------------NODE CLASS-------------------------------------
	class Node {
	public:

		//sets the x and y coordinates, explored, and the parent node
		Node(int x, int y, Node* parent); //Constructor - NODES ARE CREATED WHEN YOU EXPLORE NODES ADJACENT TO THEM
		~Node();
		
		int getX(); //get x coordinate
		int getY(); //get y coordinate
		bool wasExplored(); //returns if node is explored or not
		int breezeCt(); //gets adjBreezeCt. When this reaches 4, set pit to 2
		int stenchCt(); //gets adjStenchCt. When this reaches 4, set wumpus to 2
		int isWumpus(); //gets wumpus. -1 for clear, 1 for possible wumpus, 2 for wumpus
		int isPit(); //gets pit. -1 for clear, 1 for possible pit, 2 for pit

		void exploredTrue(); //changes explored to True. (can't change to false, can't unexplore)
		void breezeCtInc(); //increase adjBreezeCt by 1
		void setBreezeCt(int num);
		void stenchCtInc(); //increase adjStenchCt by 1
		void setStenchCt(int num);
		void pitAlert(int level); //change pit to 0, 1, or 2 // you CANNOT change a known, safe node (0) to an unsafe node.
		void setStench();
		void setBreeze();

		//!!!!!DANGER!!!!!WUMPUS ALERT!!!!!DANGER!!!!!
		void wumpusAlert(int level); //change "wumpus" danger level to 0, 1, or 2

		Node* getParent(); //returns a pointer to the parent of the node
		Node* getNorth();
		Node* getSouth();
		Node* getEast();
		Node* getWest();

		void setNorth(Node* node);
		void setSouth(Node* node);
		void setEast(Node* node);
		void setWest(Node* node);

	private:
		//Location related variables
		int xCoord; //set on creation. Unchanging
		int yCoord; //set on creation. Unchanging

		//Pit/breeze related variables
		bool breeze; //set on exploration once. Unchanging
		int adjBreezeCt; //set on exploration (multiple times)
		int pit; //set on exploration (multiple times)

		//Wumpus/stench related variables
		bool stench; //set on exploration once. Unchanging
		int adjStenchCt; //set on exploration multiple times
		int wumpus; //set on exploration multiple times

		bool explored; //to differentiate unexplored but existing frontier nodes from explored ones. Set on creation, again on exploration

		Node* parent; //to backtrack towards goal. Set on creation. Unchanging
		Node* north; //pointers to nodes in each of these directions
		Node* south;
		Node* east;
		Node* west;
	};

	//-----------------------------------------------------------
	Node* current; //has pointer to parent node, also has x,y coordinates. Starts at 0,0 (entrance, lower left)
	Node* previous; //pointer to node that you were previously on. Used for quick backtracking. Also used in case of bumps
	Node* entrance; //keeps track of the starting ladder node to make it simple to go back after we get the gold.
	int direction; //can be 0(North), 1(East), 2(South), or 3(West). Starts at 1 (starting square, facing east)
	Node* nextNode; //the next (closest) frontier node that we want to travel to, that is also the closest to the root (start) node
	int maxX; //used to limit the number of walks into walls. Unset until a bump is felt
	int maxY; //used to limit the number of walks into walls. Unset until a bump is felt
	std::list<Node*> nodes; //keeps track of all nodes that were created
	std::list<Node*> frontier; //keeps track of UNEXPLORED nodes
	std::list<Node*> nodePath; //keeps track of the nodes that we must go through to get to the nextNode from our current node
	int turnCounter;
	bool toEntrancePath; //keeps track of if the path to the entrance has been put into nodePath yet

	bool arrowShot; //only "true" for one turn, the turn immediately after shooting the arrow
	bool haveArrow; 
	bool wumpusDead; //oh no
	int leftTurnsLeft; //times you need to turn left to face the correct direction. Can be 0,1, or 2 (turn right instead of 3)
	
	bool toEntrance; //true if we've (a) found the gold or (b) taken more than 150 actions without finding the gold or (c) explored every "reachable" (safe) node


	//The two main functions that are called, representing the two parts of the problem: retrieving the gold, and then finding a way back
	Agent::Action findGold(bool stench, bool breeze, bool gold, bool bump, bool scream);
	Agent::Action returnToEntrance(Node* current);

	//FindGold consists of two sub-parts: updateWorld, and decideMove
	void updateWorld(bool stench, bool breeze, bool gold, bool bump, bool scream);
	Agent::Action decideMove(bool stench, bool breeze, bool gold, bool bump, bool scream);

	//createFrontierNodes takes the current (just explored) node and creates empty Nodes representing the newly revealed, unexplored frontier nodes
	//it also adds all newly created nodes to the "frontier" stack, prioritizing [UNFINISHED]
	void createFrontier(Node* current);
	void trimNodes(); //whenever maxX or maxY is decreased, goes through Nodes and Frontier and takes out anything that has out of bounds coordinates
	void raiseStenchCount(Node* current); //assumes that current node is already connected to neighbor nodes and empty, unexplored nodes where applicable
	void raiseBreezeCount(Node* current);
	int nodeCost(Node* current, Node* dest); //calculates the cost of a dest node based on distance from the current node and distance from the entrance.
	Agent::Action getTo(Node* current, Node* dest); //assumes current is an adjacent node to dest. turns towards the node or moves forward into the node.
	std::list<Node*> findPath(Node* current, Node* dest);
	std::list<Node*> createPath(std::map<Node*, Node*> map, Node* node);
	void checkNeighbor( //called in createPath only
		Node* neighbor,
		Node* current,
		Node* dest,
		std::set<Node*> &explored,
		std::set<Node*> &unexplored,
		std::map<Node*, Node*> &parentOf,
		std::map<Node*, int> &startThroughNodeCost,
		std::map<Node*, int> &startToNodeCost);
	void createNodePath();
	// ======================================================================
	// YOUR CODE ENDS
	// ======================================================================
};
#endif