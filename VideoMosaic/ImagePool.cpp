#include <dirent.h>
#include <errno.h>
#include <iostream>

#include "ImagePool.h"

using namespace std;

/**
 * Main constructor for ImagePool object. Takes in 
 * a directory path and reads all the gist and rgb
 * files in that directory and loads them in as
 * a vector of InputImage objects.
 */
ImagePool::ImagePool(const char* directory) {
  images = vector<string>();
  dir = string(directory);
  
  cout << dir << endl;
  
  DIR *dp;
  struct dirent *dirp;

  dp = opendir(dir.c_str());

  if(dp == NULL) {
    printf("Error opening directory! %s\n", directory);
    exit(0);
  }
  
  while((dirp = readdir(dp)) != NULL) {
    
    string filename = string(dirp->d_name);
    
    string test = filename.substr(filename.find_last_of(".")+1);
    string first = filename.substr(0,filename.find_last_of("."));
    
    if(test == "gist") {
      string full_path = dir+"/"+filename;
      string rgb_path = dir+ "/"+first+".jpg.rgb";

      InputImage input(full_path, rgb_path);
      
      const vector<float>& gist_desc = input.getGistDesc();
      if(gist_desc.size() == 960) {
        images.push_back(filename);
        input_images.push_back(input);
      }
      else {
        cout << "Error Reading: " << full_path << endl;
      }
    }
  }
  closedir(dp);
}

/**
 * Default stub constructor.
 */
ImagePool::ImagePool() {
}

/**
 * Gets the InputImage vector that was
 * originally read in by the constructor
 */
vector<InputImage> ImagePool::getImages() {
  return input_images;
}

/**
 * Allows the user to add multiple directories
 * for the image pool. Does the same as the
 * constructor by reading in image's gist and
 * rgb data files.
 */
void ImagePool::add(const char* directory) {
  dir = string(directory);
  
  cout << dir << endl;
  
  DIR *dp;
  struct dirent *dirp;
  
  dp = opendir(dir.c_str());
  
  if(dp == NULL) {
    printf("Error opening directory! %s\n", directory);
    exit(0);
  }
  
  while((dirp = readdir(dp)) != NULL) {
    
    string filename = string(dirp->d_name);
    
    string test = filename.substr(filename.find_last_of(".")+1);
    string first = filename.substr(0,filename.find_last_of("."));
    
    if(test == "gist") {
      string full_path = dir+"/"+filename;
      string rgb_path = dir+ "/"+first+".jpg.rgb";
      InputImage input(full_path, rgb_path);
      
      const vector<float>& gist_desc = input.getGistDesc();
      if(gist_desc.size() == 960) {
        images.push_back(filename);
        input_images.push_back(input);
      }
      else {
        cout << "Error Reading: " << full_path << endl;
      }
    }
  }
  closedir(dp);
}