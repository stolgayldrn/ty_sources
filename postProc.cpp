#include "postProc.h"

void computeReprojError( const std::vector<Point2f> m1, const std::vector<Point2f> m2, Mat model, std::vector<double> &repjErr, double &meanErr, double &var )
{
    int i, count = m1.size();
    const double* H = (double*)model.data;
	meanErr = 0;

	repjErr.resize(count);

#pragma omp parallel for
    for( i = 0; i < count; i++ )
    {
        double ww = 1./(H[6]*m1[i].x + H[7]*m1[i].y + 1.);
        double dx = (H[0]*m1[i].x + H[1]*m1[i].y + H[2])*ww - m2[i].x;
        double dy = (H[3]*m1[i].x + H[4]*m1[i].y + H[5])*ww - m2[i].y;
        repjErr[i] = (dx*dx + dy*dy);
		meanErr += repjErr[i];
    }
	meanErr /= (double)count; 

	var = 0;
	// also compute the variance
	// for now I compute the max deviation
	// this is safer to normalize
	for( i = 0; i < count; i++ )
    {
		const double d = meanErr - repjErr[i];
		//const double devSqr = d*d;
		//var += devSqr;

		const double dev = abs(d);
		
		if ( dev > var )
			var = dev;
	}

	// unbiased estimate
	//var /= (count-1);
}

void ransacTestFund(const std::vector<DMatch>& matches, const std::vector<Point2f>& keypoints1, 
	const std::vector<Point2f>& keypoints2, std::vector<DMatch>& outMatches) 
{
	// Convert keypoints into Point2f
	std::vector<cv::Point2f> points1, points2;

	for (std::vector<DMatch>::const_iterator it = matches.begin(); it != matches.end(); ++it) 
	{	
		// assert(it->queryIdx<keypoints1.size());
		// assert(it->trainIdx<keypoints2.size());
		// Get the position of left keypoints
		float x = keypoints1[it->queryIdx].x;
		float y = keypoints1[it->queryIdx].y;
		points1.push_back(Point2f(x,y));

		// Get the position of right keypoints
		x = keypoints2[it->trainIdx].x;
		y = keypoints2[it->trainIdx].y;
		points2.push_back(Point2f(x,y));
	}

	// Compute F matrix using RANSAC
	std::vector<uchar> inliers(points1.size(),0);

	Mat fundemental= findFundamentalMat(
		Mat(points1), Mat(points2), // matching points
		inliers,                    // match status (inlier or outlier)
		FM_RANSAC);    

	if (inliers.size() > 1)
	{
		// extract the surviving (inliers) matches
		std::vector<uchar>::const_iterator  itIn = inliers.begin();
		std::vector<DMatch>::const_iterator itM  = matches.begin();

		// for all matches
		for ( ; itIn != inliers.end(); ++itIn, ++itM)
		{
			if (*itIn) // it is a valid match
			{ 
				outMatches.push_back(*itM);
			}
		}
	}
	else
		outMatches.clear();

	/*if (refineF) 
	{
		// The F matrix will be recomputed with
		// all accepted matches
		// Convert keypoints into Point2f
		// for final F computation
		points1.clear();
		points2.clear();

		for (vector<DMatch>::const_iterator it = outMatches.begin(); it != outMatches.end(); ++it)
		{
			// Get the position of left keypoints
			float x = keypoints1[it->queryIdx].x;
			float y = keypoints1[it->queryIdx].y;
			points1.push_back(Point2f(x,y));

			// Get the position of right keypoints
			x = keypoints2[it->trainIdx].x;
			y = keypoints2[it->trainIdx].y;
			points2.push_back(Point2f(x,y));
		}

		// Compute 8-point F from all accepted matches
		fundemental = findFundamentalMat(Mat(points1),Mat(points2), CV_FM_8POINT);
	}*/

	return ;
}


bool niceHomography(const double* hp)
{
	const double det = hp[0] * hp[4] - hp[3] * hp[1];
	if (det < 0)
		return false;

	const double N1 = sqrt(hp[0] * hp[0] + hp[3] * hp[3]);
	if (N1 > 4 || N1 < 0.1)
		return false;

	const double N2 = sqrt(hp[1] * hp[1] + hp[4] * hp[4]);
	if (N2 > 4 || N2 < 0.1)
		return false;

	const double N3 = sqrt(hp[6] * hp[6] + hp[7] * hp[7]);
	if (N3 > 0.002)
		return false;

	return true;
}

double estimate_homography(std::vector<Point2f> &points1, std::vector<Point2f> &points2, std::vector<unsigned int> &Inliers, const double meanThreshold = 10.0)
{
	Mat _mask;
	//double ransacReprojThreshold = 3;
    Mat inliersMat;

    Mat H = findHomography(Mat(points1), Mat(points2), 0, 3, _mask);
	findNonZero(_mask, inliersMat);

	if ( max(inliersMat.rows, inliersMat.cols) > 3)
	{
		const double meanThresholdSqr = meanThreshold*meanThreshold;

		inliersMat = inliersMat.reshape(1).colRange(1,2);
		Inliers.resize( max( inliersMat.rows, inliersMat.cols) );

		std::vector<double> repjErr;

		double meanErr=0, var=0;
		computeReprojError(points1, points2, H, repjErr, meanErr, var);

		// this could potentially be a match
		if ( (meanErr < meanThresholdSqr) ) // && ;
		{
			if ( niceHomography ( (double*)H.data) )
			{
				// convert to score : greedy
				if (var > 0.0001)
					return (1 - (meanErr)/var );
				else
					return 1;
			}
		}		

		// compute mean reprojection error of the matches
		/*std::vector<Point2f> points1In, points2In ;
		for (unsigned int i=0; i<Inliers.size(); i++)
		{
			unsigned int ind = (unsigned int)inliersMat.at<int>(i);
			Inliers[i] = ind;
			points1In[i] = points1[ind];
			points2In[i] = points2[ind];
		}*/
	}
	
    return -1;
}

double ransacTestHomography(const std::vector<DMatch>& matches, const std::vector<Point2f>& keypoints1, 
	const std::vector<Point2f>& keypoints2, std::vector<DMatch>& outMatches, const double threshold = 10)
{
	std::vector<Point2f> points1, points2;
	std::vector<unsigned int> Inliers;

	for (std::vector<DMatch>::const_iterator it = matches.begin(); it != matches.end(); ++it) 
	{	
		assert(it->queryIdx<keypoints1.size());
		assert(it->trainIdx<keypoints2.size());

		// Get the position of left keypoints
		float x = keypoints1[it->queryIdx].x;
		float y = keypoints1[it->queryIdx].y;
		points1.push_back(Point2f(x,y));

		// Get the position of right keypoints
		x = keypoints2[it->trainIdx].x;
		y = keypoints2[it->trainIdx].y;
		points2.push_back(Point2f(x,y));
	}

	double error = estimate_homography(points1, points2, Inliers, threshold);

	if (error >= 0)
	{
		for (unsigned int i=0; i<Inliers.size(); i++)
		{
			outMatches.push_back( matches[Inliers[i] ] );
		}
	}

	return error;
}


int cv_get_descs(const char* FileName, Mat &descriptorMat, std::vector<Point2f> &Coords, std::vector<float> &Orientations, std::vector<float> &Scales)
{
	int status = -1;
	unsigned int numdesc = 0;
	unsigned char* siftDescByte = 0;
	float* xCoords = 0;
	float* yCoords = 0;
	float* orientations = 0;
	float* scales = 0;
	
	if ( read_sig(FileName, &numdesc, &siftDescByte, &xCoords, &yCoords, &orientations, &scales) >=0 )
	{
		descriptorMat = Mat(numdesc, 128, CV_32F);
		for (unsigned int i=0; i<numdesc; i++)
		{
			const unsigned char* srcRow = &siftDescByte[i*128];
			float* descRowCV = (float*)(&descriptorMat.data[descriptorMat.step*i]);
			for (int j=0; j<128; j++)
			{
				descRowCV[j] = (float)srcRow[j];
			}
		}

		delete[] siftDescByte;
		siftDescByte=0;

		if (xCoords)
		{
			Coords.reserve(numdesc);
			Orientations.reserve(numdesc);
			Scales.reserve(numdesc);

			for (unsigned int i=0; i<numdesc; i++)
			{
				Coords.push_back(Point2f(xCoords[i], yCoords[i]));
				Orientations.push_back(orientations[i]);
				Scales.push_back(scales[i]);
			}
		}

		delete[] xCoords; xCoords=0;
		delete[] yCoords; yCoords=0;
		delete[] orientations; orientations=0;
		delete[] scales; scales=0;

		status = 0;
	}

	return status;
}

int cv_FLANN_Matcher(Mat &descriptors_1, Mat &descriptors_2,std::vector< DMatch > &matches,std::vector< DMatch > &good_matches)
{
	int status = -1;
	FlannBasedMatcher matcher;	
	good_matches.clear();

	if (descriptors_1.rows>4 && descriptors_2.rows>4)
	{
		matcher.match( descriptors_1, descriptors_2, matches );

		if (matches.size()>0)
		{
			//-- Quick calculation of max and min distances between keypoints
			double max_dist = 0; double min_dist = matches[0].distance;

			for( int i = 0; i < matches.size(); i++ )
			{ double dist = matches[i].distance;
			if( dist < min_dist ) min_dist = dist;
			if( dist > max_dist ) max_dist = dist;
			}

			for( int i = 0; i < matches.size(); i++ )
			{ if( matches[i].distance <= max(2*min_dist, 250.0) )
			{ good_matches.push_back( matches[i]); }
			}
		}

		matcher.clear();
	}

	return status;
}

int ratioTest(std::vector<std::vector<cv::DMatch> > &matches)
{
	double ratio_ = 0.6;

	//-- Quick calculation of max and min distances between keypoints
	/*double max_dist = 0; double min_dist = 100;
	for( int i = 0; i < matches.size(); i++ )
	{ double dist = matches[i][0].distance;
	if( dist < min_dist ) min_dist = dist;
	if( dist > max_dist ) max_dist = dist;
	dist = matches[i][1].distance;
	if( dist < min_dist ) min_dist = dist;
	if( dist > max_dist ) max_dist = dist;
	}

	ratio_ = max(2*min_dist, 0.02);*/

	int removed = 0;
	// for all matches
	for ( std::vector<std::vector<cv::DMatch> >::iterator
		matchIterator= matches.begin(); matchIterator!= matches.end(); ++matchIterator)
	{
		// if 2 NN has been identified
		if (matchIterator->size() > 1 && (*matchIterator)[1].distance>0)
		{
			// check distance ratio
			if ((*matchIterator)[0].distance / (*matchIterator)[1].distance > ratio_)
			{
				matchIterator->clear(); // remove match
				removed++;
			}
		}
		else
		{ // does not have 2 neighbours
			matchIterator->clear(); // remove match
			removed++;
		}
	}
	return removed;
}

void symmetryTest( const std::vector<std::vector<cv::DMatch> >& matches1,
                     const std::vector<std::vector<cv::DMatch> >& matches2,
                     std::vector<cv::DMatch>& symMatches )
{

  // for all matches image 1 -> image 2
   for (std::vector<std::vector<cv::DMatch> >::const_iterator
       matchIterator1 = matches1.begin(); matchIterator1 != matches1.end(); ++matchIterator1)
   {
      // ignore deleted matches
      if (matchIterator1->empty() || matchIterator1->size() < 2)
         continue;

      // for all matches image 2 -> image 1
      for (std::vector<std::vector<cv::DMatch> >::const_iterator
          matchIterator2 = matches2.begin(); matchIterator2 != matches2.end(); ++matchIterator2)
      {
        // ignore deleted matches
        if (matchIterator2->empty() || matchIterator2->size() < 2)
           continue;

        // Match symmetry test
        if ((*matchIterator1)[0].queryIdx ==
            (*matchIterator2)[0].trainIdx &&
            (*matchIterator2)[0].queryIdx ==
            (*matchIterator1)[0].trainIdx)
        {
            // add symmetrical match
            symMatches.push_back(
              cv::DMatch((*matchIterator1)[0].queryIdx,
                         (*matchIterator1)[0].trainIdx,
                         (*matchIterator1)[0].distance));
            break; // next match in image 1 -> image 2
        }
      }
   }

}

void cv_FLANN_MatcherRobust( cv::Mat& descriptors_1, cv::Mat& descriptors_2, std::vector< std::vector<DMatch> > &matches12, std::vector< std::vector<DMatch> > &matches21, std::vector<cv::DMatch>& good_matches)
{
	FlannBasedMatcher matcher;	
	good_matches.clear();

	if (descriptors_1.rows>4 && descriptors_2.rows>4)
	{

		// 2. Match the two image descriptors

		// 2a. From image 1 to image 2
		matcher.knnMatch(descriptors_1, descriptors_2, matches12, 2); // return 2 nearest neighbours

		if (matches12.size()>1)
		{
				// 2b. From image 2 to image 1
				matcher.knnMatch(descriptors_2, descriptors_1, matches21, 2); // return 2 nearest neighbours

				if (matches21.size()>1)
				{
					// 3. Remove matches for which NN ratio is > than threshold
					// clean image 1 -> image 2 matches
					//int removed1 = ratioTest(matches12);
					// clean image 2 -> image 1 matches
					//int removed2 = ratioTest(matches21);

					// 4. Remove non-symmetrical matches
					symmetryTest(matches12, matches21, good_matches);
				}
		}
	}

}

int cv_GeoRR_Scoring_Location(std::vector<Point2f> &coords_Q, std::vector<Point2f> &coords_S, std::vector< DMatch > &matches, double &score, TScoringMode scoreMode)
{
	int status=-1;

	if ( scoreMode != T_SCORE_GV)
	{
		//Histogram vector for log distance ratios
		std::vector<float> bins(110, 0.0);
		score = 0;
		if(matches.size()>1)
		{		
			for(unsigned int i=0; i<matches.size()-1; i++)
			{
				double Q_i_x = (double)coords_Q[matches[i].queryIdx].x;
				double Q_i_y = (double)coords_Q[matches[i].queryIdx].y;
				double S_i_x = (double)coords_S[matches[i].trainIdx].x;
				double S_i_y = (double)coords_S[matches[i].trainIdx].y;

				for(unsigned int si=i+1; si<matches.size(); si++)
				{
					double Q_si_x = (double)coords_Q[matches[si].queryIdx].x;
					double Q_si_y = (double)coords_Q[matches[si].queryIdx].y;
					double S_si_x = (double)coords_S[matches[si].trainIdx].x;
					double S_si_y = (double)coords_S[matches[si].trainIdx].y;

					/////Distances ratios////
					double distanceQ=(((Q_si_x-Q_i_x)*(Q_si_x-Q_i_x))+((Q_si_y-Q_i_y)*(Q_si_y-Q_i_y)));
					double distanceS=(((S_si_x-S_i_x)*(S_si_x-S_i_x))+((S_si_y-S_i_y)*(S_si_y-S_i_y)));

					if (distanceS>0.00000001)
					{
						double log_dist_ratio = log(distanceQ/distanceS);

						if( (log_dist_ratio>=-20) && (log_dist_ratio<= 20) )
						{
							log_dist_ratio=floorf((log_dist_ratio * 2.75f) + 55);  //That means the logaritmic ratios bw -10 to 10 are altered bw 0 to 50.
							//All rational numbers from specific ranges(0.4) are altered to integer, in other word normalized.
							//For exm, [-10,-9.6) as 0 and [-9.6, -9.2) as 1.... [0,0.4) as 25 .........
							bins[(int)log_dist_ratio]++;
						}
					}
				}
			}

			score = *max_element(bins.begin(),bins.end());

			if (scoreMode > 0)
			{
				double sum = 0;
				for (int i=0; i < bins.size(); i++)
					sum += (double)bins[i];

				if(sum)
				{
					if ( scoreMode == T_SCORE_FGC_WEIGHTED )
						score = (score/sum) * ((double)matches.size() / sqrt((double)coords_Q.size()*(double)coords_S.size()));//max bin size*(match/descriptor)	
					else if (T_SCORE_FGC_NORM)
						score = (score/sum) * ((double)matches.size() / (double)coords_Q.size() );
				}
			}
			status=0;
		}
		bins.clear();
	}
	else
	{
		std::vector<DMatch> outMatches; // maybe we'll use this some day
		score = (int)ransacTestHomography(matches, coords_Q, coords_S, outMatches) ;
		status = 0;
	}	

	return status;

}