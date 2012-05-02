#ifndef _MSquare_h
#define _MSquare_h

#include <stdlib.h>
#include <vector>
#include <cv.h>
#include "InputImage.h"

class MSquare {
  
  std::vector<InputImage> matches;
  int avg_r, avg_b, avg_g;
  InputImage motionMatch;
  bool reused;
  int prev_choosen;
 
public:
  MSquare(int r, int g, int b);
  MSquare();
  std::vector<InputImage> getMatches();
  void setMatches(std::vector<InputImage>);
  void setAvgRGB(int r, int g, int b);
  void addMotionFrame(InputImage );
  InputImage getMotionMatch();
  int getAvgR();
  int getAvgG();
  int getAvgB();
  void setIsReused(bool);
  bool isReused();
  void setPrevChoosen(int);
  int getPrevChoosen();
};
#endif
