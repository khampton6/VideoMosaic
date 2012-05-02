#include "MSquare.h"

using namespace std;
using namespace cv;

/**
 * Default stub constructor
 */
MSquare::MSquare() {}

/**
 * Main constructor taking in the average RGB data
 * of each mosaic square tile.
 */
MSquare::MSquare(int r, int g, int b) : matches() {
  avg_r = r;
  avg_b = b;
  avg_g = g;
  reused  = false;
}

/**
 * Each Mosaic tile contains a previously 
 * choosen variable that represents which
 * index of the matches array was choosen.
 */
void MSquare::setPrevChoosen(int input) {
  prev_choosen = input;
}

int MSquare::getPrevChoosen() {
  return prev_choosen;
}

/**
 * Ability to add Motion estimates between
 * frames
 */
void MSquare::addMotionFrame(InputImage newInput) {
  motionMatch = newInput;
}

InputImage MSquare::getMotionMatch() {
  return motionMatch;
}

/**
 * Method to set reused flag.
 * Used to tell if a tile is being
 * reused.
 */
void MSquare::setIsReused(bool input) {
  reused = input;
  prev_choosen = 0;
}

bool MSquare::isReused() {
  return reused;
}

/**
 * Getter and setter for
 * the matches given to each tile.
 */
vector<InputImage> MSquare::getMatches() {
  return matches;
}

void MSquare::setMatches(vector<InputImage> match_set) {
  matches = match_set;
}

/**
 * Getters and Setters for
 * average RGB information
 */
void MSquare::setAvgRGB(int r, int g, int b) {
  avg_r = r;
  avg_b = b;
  avg_g = g;
}

int MSquare::getAvgR() {
  return avg_r;
}
int MSquare::getAvgG() {
  return avg_g;
}
int MSquare::getAvgB() {
  return avg_b;
}
