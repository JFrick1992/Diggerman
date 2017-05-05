#include "StudentWorld.h"
using namespace std;

GameWorld* createStudentWorld(string assetDir) { return new StudentWorld(assetDir); }
int StudentWorld::init() {
	currentKey = -1;
	GoldBait = 0; //Created to initialize the goldbait count to zero. This will also reset the amount of gold bait at the end of every level to zero -- not sure if that should be done. 
	              //if it is not meant to be that way, we can use a flag to make sure it only runs once.
	SonarKits = 1; //he is given 1 at the start of every level
	dm = new DiggerMan(this);
	dm->setVisible(true);
	fillDirt();
	p = new Protester(this);
	p->setVisible(true);
	int G = getLevel() * 25 + 300;
	actors.push_back(new Sonar(0, 60, this));
	actors.back()->setVisible(true);
	generateField("Barrel");
	generateField("PermNugget");
	generateField("Boulder");
	return GWSTATUS_CONTINUE_GAME;
}
int StudentWorld::move() {
	// This code is here merely to allow the game to build, run, and terminate after you hit enter a few times.
	// Notice that the return value GWSTATUS_PLAYER_DIED will cause our framework to end the current level.
	HUD();
	getKey(currentKey);
	if (currentKey == KEY_PRESS_ESCAPE) {
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}
	if (dm->getHealth() <= 0) {
		decLives();
		cleanUp();
		return GWSTATUS_PLAYER_DIED;
	}
	dm->doSomething();
	p->doSomething();
	for (Actor* a : actors){
		if (a->isAlive()) 
			a->doSomething();
	}
	for (int i = 0; i < actors.size(); i++) {
		if (!actors[i]->isAlive()) {
			delete actors[i];
			actors.erase(actors.begin() + i);
		}	
	}
	currentKey = 0;
	return GWSTATUS_CONTINUE_GAME;
}
bool StudentWorld::DMinVicinity(int range, int x, int y) {
	double dist = sqrt(pow(dm->getX() - x, 2) + pow(dm->getY() - y, 2));
	return dist <= range;
}
void StudentWorld::fillDirt(){
	for (int i = 0; i < VIEW_WIDTH; i++) {
		for (int j = 0; j < VIEW_HEIGHT; j++) {
			dirt[i][j] = new Dirt(i, j, this);
			if ((i >= 30 && i <= 33 && j >= 4) || (j >= VIEW_HEIGHT - 4))
				dirt[i][j] = nullptr;
			else
				dirt[i][j]->setVisible(true);
		}
	}
}
void StudentWorld::removeDirt(int x, int y) {
	for (int i = x; i < x + 4; i++) {
		for (int j = y; j < y + 4; j++) {
			if (dirt[i][j]) {
				/*****************************************************
				Dig sound is kind of loud and somewhat masks other sound effects, comment out the playSound function call directly below to remove it.
				***************************************************/
				playSound(SOUND_DIG);
				Dirt* temp = dirt[i][j];
				dirt[i][j] = nullptr;
				delete temp;
			}
		}
	}
}
void StudentWorld::dropNugget() {
	actors.push_back(new TempGoldNugget(200, dm->getX(), dm->getY(), this));  //Adjust tempNugg life to whatever yall want
	actors.back()->setVisible(true);
}
int StudentWorld::numOfGoldBait() {
	return GoldBait;
}
void StudentWorld::decrementGoldBait() {
	if (GoldBait > 0)
		GoldBait--;
}
void StudentWorld::incrementSonarKit() { SonarKits++; }
void StudentWorld::decrementSonarKit() {
	if (SonarKits > 0)
		SonarKits--;
}
int StudentWorld::numOfSonarKits() {
	return SonarKits;
}
void StudentWorld::sonarBLAST() { //activates all nuggets and barrels within a radius of 12
	playSound(SOUND_SONAR); //not in spec but found in code
	for (Actor *a : actors) {
		if (DMinVicinity(12, a->getX(), a->getY()))
			a->setVisible(true);
	}
}
void StudentWorld::incrementGoldBait() { GoldBait++; }
int StudentWorld::randXGenerator() {
	int x = rand() % 61;
	while (x > 26 && x < 34)
		x = rand() % 61;
	return x;
}
int StudentWorld::randYGenerator(string type) {
	int y = rand() % 57;
	if (type == "Boulder") { //boulders must spawn between y = 20 and y = 56
		while (y < 20 || y > 56) {
			y = rand() % 57;
		}
	}
	return y;
}
bool StudentWorld::goodSpot(int randX,int randY) {		
		if (isThereDirtVisibleHere(randX, randY)) {
			if (!isThereDirtVisibleHere(randX, randY + 3) || !isThereDirtVisibleHere(randX + 3, randY + 3) || !isThereDirtVisibleHere(randX + 3, randY)) {
				return false;
			}
			return true;
		}
		return false;
}
bool StudentWorld::farAway(int x,int y) {
	for (Actor* a : actors) {
		if( (sqrt(pow(x - a->getX(), 2) + pow(y - a->getY(), 2))) < 7) {  //6 unit space between each spawned object
			return false;
		}
	}
	return true;
}
void StudentWorld::generateField(string type){
	int spawn_amount;
	if      (type == "PermNugget") spawn_amount = numOfGoldNuggets();
	else if (type == "Barrel")     spawn_amount = numOfOilBarrels();
	else if (type == "Boulder")    spawn_amount = numOfBoulders();
	//------------------------------------------------
	//Generating items in acceptable situations
	//-----------------------------------------------
	for (int i = 0; i < spawn_amount; i++) {
		int randX = randXGenerator();
		int randY = randYGenerator(type);
		int ct = 0;
		while (true) {
			if (ct > 200) {
				if (goodSpot(randX, randY))
					break;
			}
			if (goodSpot(randX, randY)) {
				if (farAway(randX, randY))
					break;
			}
			randX = randXGenerator();
			randY = randYGenerator(type);
			ct++;
		}
		
		if (type == "PermNugget") {
			actors.push_back(new PermGoldNugget(this, randX, randY)); 
			actors.back()->setVisible(false);
		} 
		else if (type == "Barrel") {
			actors.push_back(new Barrel(randX, randY, this)); 
			actors.back()->setVisible(false);
		}		
		else if (type == "Boulder") {
			actors.push_back(new Boulder(randX, randY, this));
			removeDirt(randX, randY);                          //remove dirt from where the boulder spawns
			actors.back()->setVisible(true);
		}
	}
}
void StudentWorld::HUD() {
	string HUD =
		"Lvl: " + to_string(getLevel()) + " " +
		"Lives: " + to_string(getLives()) + " " +
		"Hlth: " + to_string(dm->getHealth() * 10) + "% " +
		"Scr: " + to_string(getScore()) + " ";
	setGameStatText(HUD);
}
int StudentWorld::getCurKey() { return currentKey; }
//the following determine how many of each Goodie will be in the current level:
int StudentWorld::numOfGoldNuggets() { return max((int)(5 - getLevel()) / 2, 2); }
int StudentWorld::numOfBoulders() { return min((int)(getLevel()) / 2 + 2, 7); }
int StudentWorld::numOfOilBarrels() { return min((int)(2 + getLevel()), 18); }
int StudentWorld::numOfSonarTicks() { return max(100, int(300-(10*getLevel()))); } //returns how many ticks until sonar kit disappears/expires
bool StudentWorld::isThereDirtVisibleHere(int x, int y){ return dirt[x][y]; }
void StudentWorld::cleanUp() {
	delete dm;
	delete p;
}
bool StudentWorld::isDirtAboveMe(int x, int y, int z) {
	return (isThereDirtVisibleHere(x, y + 4 + z) || isThereDirtVisibleHere(x + 1, y + 4 + z) ||
		isThereDirtVisibleHere(x + 2, y + 4 + z) || isThereDirtVisibleHere(x + 3, y + 4 + z));
}
bool StudentWorld::isDirtLeftOfMe(int x, int y, int z) {
	return (isThereDirtVisibleHere(x - 1 + z,  y) || isThereDirtVisibleHere(x - 1 + z, y + 1) ||
		isThereDirtVisibleHere(x - 1 + z, y + 2) || isThereDirtVisibleHere(x - 1 + z, y + 3));
}
bool StudentWorld::isDirtRightOfMe(int x, int y, int z) {
	return (isThereDirtVisibleHere(x + 4 + z, y) || isThereDirtVisibleHere(x + 4 + z, y + 1) ||
		isThereDirtVisibleHere(x + 4 + z, y + 2) || isThereDirtVisibleHere(x + 4 + z, y + 3));
}
bool StudentWorld::isDirtUnderMe(int x, int y, int z){
	return (isThereDirtVisibleHere(x, y - 1 + z) || isThereDirtVisibleHere(x + 1, y - 1 + z) ||
		isThereDirtVisibleHere(x + 2, y - 1 + z) || isThereDirtVisibleHere(x + 3, y - 1 + z)); 
}
bool StudentWorld::getDistDigManOnX(int x, int y, int & dis) {
	if (dm->getY() == y) {
		dis = dm->getX() - x;
		return true;
	}
	return false;
}
bool StudentWorld::getDistDigManOnY(int x, int y, int & dis) {
	if (dm->getX() == x) {
		dis = dm->getY() - y;
		return true;
	}
	return false;
}

bool StudentWorld::canShout(int x, int y){
	if (DMinVicinity(4, x, y)) {
		playSound(SOUND_PROTESTER_YELL);
		dm->decHealth(1);
		return true;
	}
	return false;
}




/*
--------------------------------------------
PLS DONT DELETE---NOTES FOR PERM GOLD NUGGET
---------------------------------------------

---How to make sure supposed random location has dirt that will cover the nugget
-get rand x and y values of gold 
-first check if that random location is ok
-then generate the four corner x and y values as if it were toe xist there
-then go through vector(?) of dirts and see if dere is dirt with each of those values
-if there is, then we know that the location has dirt covering it from all 4 sides


----checking distance from all other objects



*/

