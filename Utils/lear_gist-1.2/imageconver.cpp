#include "highgui.h"

#include <dirent.h>

int main(int argc, char** argv) {


  DIR* dpdf;
  struct dirent* epdf;

  dpdf = opendir("./");
  if(dpdf != NULL) {
    while(epdf = readdir(dpdf)) {
      printf("Filename: %s\n", epdf->d_name);
    }
  }

  IplImage* img = cvLoadImage(argv[1]);

  cvSaveImage(argv[2], img);
  cvReleaseImage(&img);
  return 0;
}
