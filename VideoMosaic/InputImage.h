#ifndef _InputImage_h
#define _InputImage_h

#include <vector>
#include <string>

class InputImage {
  
  int avg_r, avg_g, avg_b;
  int width, height;
  std::string img_path;
  std::string rgb_path;
  std::vector<float> gist_desc;
 public:
  InputImage(const std::string, const std::string);
  InputImage();
  
  int readData(std::string path);
  int readRGB(std::string path);
  
  std::vector<float> getGistDesc();
  std::string getPath();
  int getAvgR();
  int getAvgB();
  int getAvgG();
  
  bool equals(InputImage);
};
#endif
