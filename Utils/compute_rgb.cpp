#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cv.h>
#include <highgui.h>

#include <dirent.h>

using namespace std;
using namespace cv;

void computeAvgRGB(Mat, string);
void writeToFile(string old_fn, int r, int g, int b);

int main(int argc, char** argv) {


  DIR* dpdf;
  struct dirent* epdf;

  dpdf = opendir(argv[1]);
  if(dpdf != NULL) {
    while(epdf = readdir(dpdf)) {
      //printf("Filename: %s\n", epdf->d_name);
      
      string fn(epdf->d_name);
      if(fn.substr(fn.find_last_of(".")+1) == "jpg") {
	cout << fn << endl;
	Mat m = imread(fn);

	computeAvgRGB(m,fn);
      }
    }
  }

  //  IplImage* img = cvLoadImage(argv[1]);

  //cvSaveImage(argv[2], img);
  //cvReleaseImage(&img);
  return 0;
}

void writeToFile(string old_fn, int r, int g, int b) {
  string new_fn = old_fn+".rgb";
  cout << new_fn << endl;
  ofstream output;
  output.open(new_fn.c_str());
  
  output << r << " ";
  output << g << " ";
  output << b;
  
  output.close();
}

void computeAvgRGB(Mat mat, string fn) {
  double r = 0, g = 0, b = 0;
  cout << " Rows: " << mat.rows << " Cols: " << mat.cols << endl;
  for(int i = 0; i < mat.rows; i++) {
    for(int j = 0; j < mat.cols; j++) {
      Vec3b v = mat.at<Vec3b>(i,j);
      
      r += v[2];
      g += v[1];
      b += v[0];
    }
  }

  int avgR = (r / (mat.rows*mat.cols));
  int avgG = (g / (mat.rows*mat.cols));
  int avgB = (b / (mat.rows*mat.cols));
  cout << "R: " << avgR << " G: " << avgG << " B: " << avgB << endl;
  writeToFile(fn, avgR, avgG, avgB);
}
