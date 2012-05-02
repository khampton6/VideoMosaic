//System Includes
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#include <time.h>

//Opencv Includes
#include <cv.h>
#include <highgui.h>

//Gist Includes
extern "C" {
#include "gist.h"
}

//Mosaic Library Includes
#include "ImagePool.h"
#include "ProcessorPipeline.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv) {

  //Target video and image pool paths
  const char* movie = "../Mosaic/videos/truck2.mov";
  const char* img_dir = "image_database/tennis";
  
  //Dimensions the mosaic will be divided into
  int mosaic_dim = 64;
  
  //How much colorization to use. Between 0 and 1
  float colorize_amt = .40;

  if(argc == 2) {
    printf("Loading movie: %s\n", argv[1]);
    movie = argv[1];
  }
  else if(argc == 3) {
    movie = argv[1];
    img_dir = argv[2];
    printf("Loading image files at %s\n", img_dir);
  }
  else if(argc == 4) {
    movie = argv[1];
    img_dir = argv[2];
    mosaic_dim = atoi(argv[3]);
    printf("Loading movie: %s with Dimensions: %d\n", movie, mosaic_dim);
  }
  
  //Prepare image pool -> reads in gist desc from each image
  ImagePool ipool(img_dir);
  ipool.add("image_database/georgiatech");
  ipool.add("image_database/got");
  ipool.add("image_database/gt");
  ipool.add("image_database/stonehenge");
  ipool.add("image_database/Toy");
  ipool.add("image_database/Truck");

  //Set up processor pipeline to get started.
  string movie_path(movie);
  ProcessorPipeline pipe(movie_path, mosaic_dim);
  
  //Get number of frames to parse. -> Usually wrong number.
  //But it's only an estimate anyways. 
  int num_frames_to_parse = pipe.getNumFrames();
  
  //Build random noise matrix.
  srand(time(NULL));
  vector<int> noise_row(mosaic_dim, 10);
  vector<vector<int> > noise(mosaic_dim,noise_row);
  for(size_t i = 0; i < noise.size(); i++) {
    for(size_t j = 0; j < noise[i].size(); j++) {
      noise[i][j] = rand()%10;
    }
  }
  
  //Process first frame.
  vector<vector<MSquare> > prev = pipe.processFirstFrame(ipool);
  Mat output = pipe.buildMat(prev, colorize_amt, noise);
  
  //Process following frames. 
  for(int i = 0; i < num_frames_to_parse; i++) {
    vector<vector<MSquare> > current = pipe.processFrame();
    
    vector<vector<MSquare> > difference_res = pipe.differenceFrames(prev, current);
    
    pipe.buildMat(difference_res, colorize_amt, noise);
    prev = difference_res;
  }
  return 0;
}