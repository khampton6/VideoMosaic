#include "ProcessorPipeline.h"

#include <time.h>

using namespace std;
using namespace cv;

//Defined Gist Size.
#define GIST_SIZE 128

//Types of Filtering
#define NEIGHBOR_NONE 0
#define NEIGHBOR_ONCE 1
#define NEIGHBOR_TOPBOTTOM 2
#define NEIGHBOR_HALFMAT 3
#define RANDOM_CHOICE true

/**
 * Processor Pipeline Main constructor
 * Takes in video url and dimensions. Then
 * it sets up the object's variables.
 * processFirstFrame() and processFrame() are the methods
 * to call to get work done. :)
 */
ProcessorPipeline::ProcessorPipeline(string vid_url, int m_dim) : vid_path(vid_url), vid_reader(vid_path) {

  mosaic_dim = m_dim;

  num_frames = vid_reader.get(CV_CAP_PROP_FRAME_COUNT);
  
  frame_width = vid_reader.get(CV_CAP_PROP_FRAME_WIDTH);
  frame_height = vid_reader.get(CV_CAP_PROP_FRAME_HEIGHT);

  square_h = frame_height/mosaic_dim;
  square_w = frame_width/mosaic_dim;

  vector<int**> optical_flow;
  
  frame_num = 0;
  
  firstFrame = true;
  
  vector<int> rMat(mosaic_dim,0);
  reusedMat = vector<vector<int> >(mosaic_dim, rMat);
}

/**
 * Returns the number of frames returned from the Video parser.
 */
int ProcessorPipeline::getNumFrames() {
  return num_frames;
}

/**
 * Second initialzier and processes the first frame
 * while also doing the Gist training.
 */
vector<vector<MSquare> > ProcessorPipeline::processFirstFrame(ImagePool pool) {  
  //Read First Frame
  Mat first;
  bool res = vid_reader.read(first);
  frame_num = 0;
  
  if(!res)
    return vector<vector<MSquare> >();
  
  
  cvtColor(first, orig_grey, CV_RGB2GRAY);
  goodFeaturesToTrack(orig_grey, motion_keypoints, NUMFEATUREPOINTS, 0.04, 1.0);
    
  curr_frame = first;
  
  //Train ML 
  flannMatcher = FlannBasedMatcher();
  
  images = pool.getImages();
  
  int rows = images.size();
  int cols = 960;
  
  Mat trainData(rows, cols, DataType<float>::type);
  
  for( int row = 0; row < rows; row++) {
    InputImage image = images[row];
    vector<float> gist = image.getGistDesc();
    for(int col = 0; col < 960; col++) {
      trainData.at<float>(row, col) = gist[col];
    }
  }
  
  std::vector<cv::Mat> mats;
  mats.push_back(trainData);
  flannMatcher.add(mats);
  flannMatcher.train();
  
  //Create Lear representation and copy over data
  color_image_t *lear = color_image_new(first.cols, first.rows);
  assert(lear);
  int l = 0;
  for(int i = 0; i < first.rows; i++) {
    for(int j = 0; j < first.cols; j++) {
      Vec3b rgb = first.at<Vec3b>(i,j);
      uchar b = rgb[0];
      uchar g = rgb[1];
      uchar r = rgb[2];
      
      lear->c1[l] = r;
      lear->c2[l] = g;
      lear->c3[l] = b;
      l++;
    }
  }
    
  //Compute Gist descriptor for square and store
  float* desc = color_gist_scaletab(lear, gist_nblocks, gist_n_scale, orientations_per_scale);
  
  //Convert Gist to Mat for matching
  Mat query = convertToMat(desc);
  
  vector<vector<DMatch> > matches;
  
  //Find the top 10 results
  flannMatcher.knnMatch(query, matches, 5000);
  
  free(lear);
  free(desc);
    
  rgbMatcher = FlannBasedMatcher();
  
  Mat rgb_train(matches[0].size(), 3, DataType<float>::type);
  
  vector<int> trainIdxs;
  
  for(size_t i = 0; i < matches[0].size(); i++) {
    DMatch match = matches[0][i];
    int img_index = match.trainIdx;
    trainIdxs.push_back(img_index);
    
    InputImage ii = images[img_index];
    
    rgb_train.at<float>(i,0) = ii.getAvgR();
    rgb_train.at<float>(i,1) = ii.getAvgG();
    rgb_train.at<float>(i,2) = ii.getAvgB();
  }
  
  std::vector<cv::Mat> trainingData;
  trainingData.push_back(rgb_train);
  rgbMatcher.add(trainingData);
  rgbMatcher.train();
  
  vector<vector<MSquare> > results;
  for(int i = 0; i < mosaic_dim; i++)
    results.push_back(vector<MSquare>());
  
  int row_index = 0;
  //For every Square in the image read the gist descriptor
  for(int row = 0; row < mosaic_dim; row += 1) {
    for(int col = 0; col < mosaic_dim; col += 1) {
      //Get ROI Square image
      Rect roi(col*square_w, row*square_h, square_w, square_h);
      Mat square(first, roi);
      
      double avg_r = 0, avg_b = 0, avg_g = 0;
      
      for(int i = 0; i < square.rows; i++) {
        for(int j = 0; j < square.cols; j++) {
          Vec3b vec = square.at<Vec3b>(i,j);
          
          avg_r += vec[2];
          avg_g += vec[1];
          avg_b += vec[0];
        }
      }
      
      int r = avg_r / (square.rows * square.cols);
      int g = avg_g / (square.rows * square.cols);
      int b = avg_b / (square.rows * square.cols);
            
      Mat query(1,3,DataType<float>::type);
      query.at<float>(0,0) = r;
      query.at<float>(0,1) = g;
      query.at<float>(0,2) = b;
      vector<vector<DMatch> > rgb_matches;
      rgbMatcher.knnMatch(query, rgb_matches, 10);

      vector<InputImage> input_images;
      for(size_t i = 0; i < rgb_matches[0].size(); i++) {
        DMatch d_match = rgb_matches[0][i];
        int index = d_match.trainIdx;
        int before_index = trainIdxs[index];
        InputImage img = images[before_index];
        input_images.push_back(img);
      }
      
      MSquare sqr(r,g,b);
      sqr.setMatches(input_images);
      results[row_index].push_back(sqr);
    }
    row_index++;
  }
  prev_frame = results;
  frame_num++;
  return results;
}

/**
 * Reads and computes the vector grid output 
 * for each iteration.
 */
vector<vector<MSquare> > ProcessorPipeline::processFrame() {
  
  //Read Current Frame
  Mat first;
  bool res = vid_reader.read(first);
  
  if(!res)
    return vector<vector<MSquare> >();
  
  // --------------------
  // Optical Flow step
  // --------------------
  
  //Convert to Black and white for optical flow
  cvtColor(first, orig_grey, CV_RGB2GRAY);
  goodFeaturesToTrack(orig_grey, motion_keypoints, NUMFEATUREPOINTS, 0.04, 1.0);
    
  curr_frame = first;
  
  
  // --------------------
  // Gist Calculation Step
  // --------------------
  
  //Create Lear representation and copy over data
  color_image_t *lear = color_image_new(first.cols, first.rows);
  assert(lear);
  int l = 0;
  for(int i = 0; i < first.rows; i++) {
    for(int j = 0; j < first.cols; j++) {
      Vec3b rgb = first.at<Vec3b>(i,j);
      uchar b = rgb[0];
      uchar g = rgb[1];
      uchar r = rgb[2];
      
      lear->c1[l] = r;
      lear->c2[l] = g;
      lear->c3[l] = b;
      l++;
    }
  }
    
  //Compute Gist descriptor for Image
  float* desc = color_gist_scaletab(lear, gist_nblocks, gist_n_scale, orientations_per_scale);
  
  //Convert Gist to Mat for matching
  Mat query = convertToMat(desc);
  
  vector<vector<DMatch> > matches;
  //Find the top 10 results
  flannMatcher.knnMatch(query, matches, 5000);
  
  //Free up memory
  free(lear);
  free(desc);

  //Create Flann Based Matcher and train
  rgbMatcher = FlannBasedMatcher();
  
  //Create training data by copying over avg rgb of squares in to 
  //each row of the Mat
  Mat rgb_train(matches[0].size(), 3, DataType<float>::type);
  vector<int> trainIdxs;
  for(size_t i = 0; i < matches[0].size(); i++) {
    DMatch match = matches[0][i];
    int img_index = match.trainIdx;
    trainIdxs.push_back(img_index);
    
    InputImage ii = images[img_index];
    
    rgb_train.at<float>(i,0) = ii.getAvgR();
    rgb_train.at<float>(i,1) = ii.getAvgG();
    rgb_train.at<float>(i,2) = ii.getAvgB();
  }
  
  //Train
  std::vector<cv::Mat> trainingData;
  trainingData.push_back(rgb_train);
  rgbMatcher.add(trainingData);
  rgbMatcher.train();
  
  vector<vector<MSquare> > results;
  for(int i = 0; i < mosaic_dim; i++)
    results.push_back(vector<MSquare>());
  

  int row_index = 0;
  //For every Square in the image compute avg rgb
  for(int row = 0; row < mosaic_dim; row += 1) {
    for(int col = 0; col < mosaic_dim; col += 1) {
      
      //Get ROI Square tile image
      Rect roi(col*square_w, row*square_h, square_w, square_h);
      Mat square(first, roi);
      
      double avg_r = 0, avg_b = 0, avg_g = 0;
      
      for(int i = 0; i < square.rows; i++) {
        for(int j = 0; j < square.cols; j++) {
          Vec3b vec = square.at<Vec3b>(i,j);
          
          avg_r += vec[2];
          avg_g += vec[1];
          avg_b += vec[0];
        }
      }
      
      int r = avg_r / (square.rows * square.cols);
      int g = avg_g / (square.rows * square.cols);
      int b = avg_b / (square.rows * square.cols);
      
      //Add input into system to get best results
      Mat query(1,3,DataType<float>::type);
      query.at<float>(0,0) = r;
      query.at<float>(0,1) = g;
      query.at<float>(0,2) = b;
      vector<vector<DMatch> > rgb_matches;
      rgbMatcher.knnMatch(query, rgb_matches, 10);
      
      vector<InputImage> input_images;
      for(size_t i = 0; i < rgb_matches[0].size(); i++) {
        DMatch d_match = rgb_matches[0][i];
        int index = d_match.trainIdx;
        int before_index = trainIdxs[index];
        InputImage img = images[before_index];
        input_images.push_back(img);
      }
      
      MSquare sqr(r,g,b);
      sqr.setMatches(input_images);
      results[row_index].push_back(sqr);
    }
    row_index++;
  }
  
  //We're done. Increment and return
  prev_frame = results;
  frame_num++;
  return results;
}

/**
 * Compute the differences between avg rgb's of squares 
 * and if they are different then use current else if
 * they are similar then use previous frame to preserve
 * consistency.
 */
vector<vector<MSquare> > ProcessorPipeline::differenceFrames(vector<vector<MSquare> > prev, vector<vector<MSquare> > current) {
  
  //Go through all of them. Should be same size.
  for(size_t r = 0; r < prev.size(); r++) {
    for(size_t c = 0; c < prev[r].size(); c++) {
      
      //Get MSquares
      MSquare prev_sqr = prev[r][c];
      MSquare curr_sqr = current[r][c];
      
      int prev_avg_r = prev_sqr.getAvgR();
      int prev_avg_g = prev_sqr.getAvgG();
      int prev_avg_b = prev_sqr.getAvgB();
      
      int curr_avg_r = curr_sqr.getAvgR();
      int curr_avg_g = curr_sqr.getAvgG();
      int curr_avg_b = curr_sqr.getAvgB();
      
      //Compute distance or difference between channels
      double dist = pow(curr_avg_r - prev_avg_r, 2.0) + 
        pow(curr_avg_g - prev_avg_g , 2.0) +
        pow(curr_avg_b - prev_avg_b , 2.0); 
      
      //If less than threshold, then use previous
      if(dist <= 50) {
        current[r][c] = prev_sqr; 
      }
    }
  }
  //Return results
  return current;
}

/**
 * Utility method to convert gist to mat representation
 */
Mat ProcessorPipeline::convertToMat(float* gist) {
  Mat query(1,960,DataType<float>::type);
  for(int i = 0; i < 960; i++) {
    query.at<float>(0,i) = gist[i];
  }
  return query;
}

/**
 * Builds mat final frame output.
 * Takes in the Squares matrix, colorization amount, and noise matrix.
 */
Mat ProcessorPipeline::buildMat(vector<vector<MSquare> > squares, float colorize_amt, vector<vector<int> > noise) {
  //Outputed Mat  
  Mat superMat(mosaic_dim*square_h, mosaic_dim*square_w, CV_8UC3, Scalar::all(0));
  
  //For every square in the output img
  for(size_t row = 0; row < squares.size(); row++) {
    for(size_t col = 0; col < squares[row].size(); col++) {
      
      //Get the MSquare
      MSquare square = squares[row][col];
      
      //Choose first choice by default
      vector<InputImage> choices = square.getMatches();
      
      InputImage topChoice;
      int selection_index = 0;
      
      selection_index = noise[row][col];
      topChoice = choices[selection_index];
      
      //Ignoring motion for now
//      InputImage betterChoice = square.getMotionMatch();
//      
//      if(betterChoice.getPath() != "") {
//        topChoice = betterChoice;
//        cout << "Choose motion estimate" << endl;
//        InputImage empty;
//        square.addMotionFrame(empty);
//      }
      
      //Get img path
      string gist_path = topChoice.getPath();
      string img_path = translatePathFromGist(gist_path);
      
      //Read image and scale down. 
      Mat input = imread(img_path);
      Mat resizedMat;
      resize(input, resizedMat, Size(square_w,square_h));
      
      //Fix the colorization if needed.
      colorize(curr_frame, row, col, resizedMat, colorize_amt);
      
      //Draw Image to frame.
      int row_n = row*square_h;
      int col_n = col*square_w;
      Range row_range(row_n, row_n+square_h);
      Range col_range(col_n,col_n+square_w);
      Mat range(superMat, row_range, col_range);
      resizedMat.copyTo(range);
    }
  }
  
  firstFrame = false;
  
  writeFrame(superMat);
  return superMat;
}

/**
 * Filters the list of possible matches for each square.
 * Instead I use noise matrix. Can be added to later. 
 * Too slow to be useful now.
 */
vector<InputImage> ProcessorPipeline::filterChoices(vector<vector<InputImage> > choosen, MSquare curr_sqr, 
                                                    int row, int col, int type) {
  vector<InputImage> return_matches;
  vector<InputImage> matches = curr_sqr.getMatches();
  
  if(type == NEIGHBOR_ONCE) {
    //Search the whole image to see if anything is already picked. 
    for(size_t k = 0; k < matches.size(); k++) {
      InputImage match_choice = matches[k];
      bool found = choosenContain(match_choice, choosen);
      if(!found) {
        return_matches.push_back(match_choice);
      }
    }
  }
  else if(type == NEIGHBOR_TOPBOTTOM) {
    InputImage top, left;
    if(row > 0) {
      top = choosen[row-1][col];
    }
    if(col > 0) {
      left = choosen[row][col-1];
    }
    for(size_t i = 0; i < matches.size(); i++) {
      InputImage curr_match = matches[i];
      if(!curr_match.equals(top) && !curr_match.equals(left)) {
        return_matches.push_back(curr_match);
      }
    }
  }
  else if(type == NEIGHBOR_HALFMAT) {
    InputImage leftTopDiag, top, rightTopDiag, left;
    if(row > 0 && col > 0) {
      leftTopDiag = choosen[row-1][col-1];
      left = choosen[row][col-1];
      top = choosen[row-1][col];
    }
    else if(row > 0) {
      top = choosen[row-1][col];
    }
    else if(col > 0) {
      left = choosen[row][col-1];
    }
    
    if(col < (int)choosen[0].size()-1 && row > 0) {
      rightTopDiag = choosen[row-1][col+1];
    }
    
    for(size_t k = 0; k < matches.size(); k++) {
      InputImage match = choosen[row][col];
      if(!match.equals(leftTopDiag) && !match.equals(top) 
         && !match.equals(rightTopDiag) && !match.equals(left)) {
        return_matches.push_back(match);
      }
    }
  }
  else {
    return curr_sqr.getMatches();
  }
  
  if(return_matches.size() == 0) {
    return_matches = matches;
  }
  
  return return_matches;
}

/**
 * Utility method to see if an InputImage is already choosen.
 */
bool ProcessorPipeline::choosenContain(InputImage check, vector<vector<InputImage> > choosen) {
  for(size_t i = 0; i < choosen.size(); i++) {
    for(size_t j = 0; j < choosen[i].size(); j++) {
      InputImage verifier = choosen[i][j];
      if(verifier.equals(check))
        return true;
    }
  }
  return false;
}

/**
 * Colorization method that colorizes the alpha blend between
 * the target image square and the mosaic tile.
 */
void ProcessorPipeline::colorize(Mat orig_frame, int row_indx, int col_indx, Mat new_frame, float colorize_amt) {
  for(int c_r = 0; c_r < new_frame.rows; c_r++) {
    for(int c_c = 0; c_c < new_frame.cols; c_c++) {
      
      //Get starting points in the original big image
      int orig_r = row_indx*square_h+c_r;
      int orig_c = col_indx*square_w+c_c;
      
      Vec3b newf = new_frame.at<Vec3b>(c_r, c_c);
      int new_r = newf[2];
      int new_g = newf[1];
      int new_b = newf[0];
      
      Vec3b orig = orig_frame.at<Vec3b>(orig_r, orig_c);
      int orig_rc = orig[2];
      int orig_gc = orig[1];
      int orig_bc = orig[0];
      
      Vec3b new_vec;
      new_vec[2] = new_r*(1-colorize_amt) + colorize_amt*orig_rc;
      new_vec[1] = new_g*(1-colorize_amt) + colorize_amt*orig_gc;
      new_vec[0] = new_b*(1-colorize_amt) + colorize_amt*orig_bc;
      new_frame.at<Vec3b>(c_r, c_c) = new_vec;
    }
  }
}

/**
 * Write's opencv Mat to file.
 */
void ProcessorPipeline::writeFrame(Mat frame) {
  
  stringstream stream;
  stream.str();
  stream << "output/";
  stream << frame_num;
  stream << ".jpg";
  
  string outputstr = stream.str();
  cout << outputstr << endl;
  bool res = imwrite(outputstr,frame);
  if(res) {
    cout <<"Wrote!" << endl;
    return;
  }
  else {
    cout <<"error Writing" << endl;
    return;
  }  
}

/**
 * Get's image path by removing .gist and adding .jpg
 */
string ProcessorPipeline::translatePathFromGist(string path) {
  string cleaned = path.substr(0, path.find_last_of("."));
  cleaned.append(".jpg");

  return cleaned;
}

