#include "InputImage.h"
#include <iostream>
#include <fstream>

using namespace std;

/**
 * Default stub constructor
 */
InputImage::InputImage() {};

/**
 * Takes in an image's gist and rgb path
 * and then parses the files given.
 */
InputImage::InputImage(const string path, const string rgb_p) : img_path(path), rgb_path(rgb_p) {
  readData(img_path);
  readRGB(rgb_path);
}

/**
 * Helper method to read in the RGB data from file.
 * Which is just 3 chars separated by spaces.
 */
int InputImage::readRGB(const string path) {
  ifstream stream(path.c_str(), ifstream::in);  
  int f;
  stream >> f;
  avg_r = f;
  stream >> f;
  avg_g = f;
  stream >> f;
  avg_b = f;
  
  //Return 3 for 3 chars read in.
  //Segfault if file can't be read.
  return 3;
}

/**
 * These methods return each
 * averaged R,G,and B channel.
 */
int InputImage::getAvgR() {
  return avg_r;
}
int InputImage::getAvgB() {
  return avg_b;
}
int InputImage::getAvgG() {
  return avg_g;
}

/**
 * Helper method to read in the Gist data from file.
 * Which is just 960 floats separated by spaces.
 */int InputImage::readData(string path) {

  ifstream stream(path.c_str(), ifstream::in);
    
  int cnt = 0;
  while(stream.good() && cnt < 960) {
    float f; 
    stream >> f;
    cnt++;
    gist_desc.push_back(f);
  }
  
  stream.close();

  return gist_desc.size();
}

/**
 * Returns a vector of 960 floats for
 * the Gist desc.
 */
vector<float> InputImage::getGistDesc() {
  return gist_desc;
}

/**
 * Simple equals method used to see if an 
 * InputImage has been read in and declared.
 */
bool InputImage::equals(InputImage i_img) {
  return img_path == i_img.getPath();
}

/**
 * Returns the image path associated
 * with the gist descriptor
 */
string InputImage::getPath() {
  return img_path;
}
