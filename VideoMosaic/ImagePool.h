#ifndef _ImagePool_h
#define _ImagePool_h

//Includes
#include <stdlib.h>
#include <vector>
#include <string>
#include "InputImage.h"

class ImagePool {
  //instance variables
  std::vector<std::string> images;
  std::string dir;
  std::vector<InputImage> input_images;
  
 public:
  ImagePool(const char*);
  ImagePool();
  std::vector<InputImage> getImages();
  void add(const char*);
  std::string translatePathFromGist(std::string);
};

#endif