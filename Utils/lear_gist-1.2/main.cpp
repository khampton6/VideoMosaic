#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

int getdir(string dir, vector<string> &files);

int main(int argc, char** argv) {
  
  string target_dir = "../Flickr/images/Truck";
  if(argc == 2)
    target_dir = string(argv[1]);
  vector<string> files;
  
  getdir(string(target_dir), files);
  
  for(int i = 0; i < files.size(); i++) {
    cout << files[i] << endl;
    
    string temp = files[i];
    string begin = temp.substr(0,temp.find_last_of("."));
    
    string cmd = "./convert " + files[i] + " " + begin + ".ppm";
    //cout << cmd << endl;
    system(cmd.c_str());
    
    string gist_cmd = "./compute_gist " + begin + ".ppm >> " + begin + ".gist";
    cout << gist_cmd << endl;
    system(gist_cmd.c_str());
//    break;
    //string sys_cmd = string("ls ") + files[i];
    //system(sys_cmd.c_str());
  }
}


int getdir(string dir, vector<string> &files) {
  DIR* dp;
  struct dirent* dirp;
  if((dp = opendir(dir.c_str())) == NULL) {
    cout << "Error(" << errno << ") opening " << dir << endl;
    return errno;
  }
  
  while((dirp = readdir(dp)) != NULL) {
    string str = string(dirp->d_name);

    
    
    string temp = str;
    
    int index = temp.find_last_of(".")+1;
    string cmp = temp.substr(index);
    string begin = temp.substr(0,index-1);

    if(cmp == "jpg") {
      files.push_back(dir+"/"+str);
      cout << "Begin: " << begin << endl;
      cout << "End: " << cmp << endl;
    }
    
//    string test = str.substr(str.find_last_of(".")+1);
//    string first = str.substr(0,str.find_last_of("."));
//    cout << first << endl;
//    if(test == "jpg") {
//      string new_file = first+".ppm";
//      cout << "Parsing: " << str << endl;
//      cout << "New File: " << new_file << endl;
//    }
  }
  closedir(dp);
  return 0;
}
