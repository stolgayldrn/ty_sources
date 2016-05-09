#ifndef POSTPROC_H
#define POSTPROC_H

#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include "helpers2.h"

using namespace cv;

typedef enum TScoringMode {T_SCORE_FGC = 0, T_SCORE_FGC_NORM, T_SCORE_FGC_WEIGHTED, T_SCORE_GV};

//Mat estimate_homography( std::vector<Point2f> &_points1, std::vector<Point2f> &_points2, Mat& _mask, std::vector<unsigned int> &Inliers);
int cv_get_descs(const char* FileName, Mat &descriptorMat, std::vector<Point2f> &Coords, std::vector<float> &Orientations, std::vector<float> &Scales);
int cv_FLANN_Matcher(Mat &descriptors_1, Mat &descriptors_2, std::vector< DMatch > &matches, std::vector< DMatch > &good_matches);
void cv_FLANN_MatcherRobust( cv::Mat& descriptors_1, cv::Mat& descriptors_2, std::vector< std::vector<DMatch> > &matches12, std::vector< std::vector<DMatch> > &matches21, std::vector<cv::DMatch>& good_matches);
int cv_GeoRR_Scoring_Location(std::vector<Point2f> &coords_Q, std::vector<Point2f> &coords_S, std::vector< DMatch > &matches, double &score, TScoringMode scoreMode = T_SCORE_FGC_WEIGHTED);
int cv_GeoRR_Scoring_Location_SSE(std::vector<Point2f> &coords_Q,std::vector<Point2f> &coords_S,std::vector< DMatch > &matches, double &score);

#endif