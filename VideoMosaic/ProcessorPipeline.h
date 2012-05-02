
#ifndef _ProcessorPipeline_h
#define _ProcessorPipeline_h

#include <stdlib.h>
#include <stdio.h>
#include <string>

#include <cv.h>
#include <highgui.h>

extern "C" {
#include "gist.h"
#include "standalone_image.h"
}

#include "MSquare.h"
#include "ImagePool.h"

#define NUMFEATUREPOINTS 100

static const int gist_nblocks = 4;
static const int gist_n_scale = 3;
static const int orientations_per_scale [50]={8,8,4};

class ProcessorPipeline {
  std::string vid_path;
  cv::VideoCapture vid_reader;
  
  double num_frames;
  double frame_width;
  double frame_height;
  int square_h;
  int square_w;
  int mosaic_dim;
  std::vector<std::vector<int> > reusedMat;
  
  cv::FlannBasedMatcher flannMatcher;
  cv::FlannBasedMatcher rgbMatcher;
  
  std::vector<InputImage> images;
  int frame_num;
  
  bool firstFrame;
  std::vector<std::vector<InputImage> > orig_choosen;
  std::vector<std::vector<MSquare> > prev_frame;
  cv::Mat curr_frame;
  cv::Mat orig_grey;
  std::vector<cv::Point2f> motion_keypoints;
public:
  ProcessorPipeline(std::string,int);
  std::vector<std::vector<MSquare> > processFrame();
  std::vector<std::vector<MSquare> > processFirstFrame(ImagePool);
  cv::Mat convertToMat(float*);
  cv::Mat buildMat(std::vector<std::vector<MSquare> >, float, std::vector<std::vector<int> >);
  std::string translatePathFromGist(std::string);
  void writeFrame(cv::Mat frame);
  int getNumFrames();
  void colorize(cv::Mat, int, int, cv::Mat,float);
  std::vector<InputImage> filterChoices(std::vector<std::vector<InputImage> >, MSquare, int, int, int) ;
  bool choosenContain(InputImage, std::vector<std::vector<InputImage> >);
  std::vector<std::vector<MSquare> > differenceFrames(std::vector<std::vector<MSquare> >, std::vector<std::vector<MSquare> > );
};
#endif
