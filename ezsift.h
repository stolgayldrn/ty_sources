/*	Copyright (c) 2013, Robert Wang, email: robertwgh (at) gmail.com
	All rights reserved. https://sourceforge.net/p/ezsift

	Some algorithms used in this code referred to:
	1. OpenCV: http://opencv.org/
	2. VLFeat: http://www.vlfeat.org/

	The SIFT algorithm was developed by David Lowe. More information can be found from:
	David G. Lowe, "Distinctive image features from scale-invariant keypoints," 
	International Journal of Computer Vision, 60, 2 (2004), pp. 91-110.

	Pay attention that the SIFT algorithm is patented. It is your responsibility to use the code
	in a legal way. Patent information:
	Method and apparatus for identifying scale invariant features in an image 
	and use of same for locating an object in an image	David G. Lowe, US Patent 6,711,293 
	(March 23, 2004). Provisional application filed March 8, 1999. Asignee: The University of 
	British Columbia.

	Revision history:
		September, 15, 2013: initial version.
*/

#ifndef EZSIFT_H
#define EZSIFT_H

#include "image.h"
//#include "img_io.h"
#include <vector>
#include <list>
using namespace std;

/****************************************
 * Constant parameters
 ***************************************/

// default number of sampled intervals per octave
static int SIFT_INTVLS = 3;

// default sigma for initial gaussian smoothing
static float SIFT_SIGMA = 1.6f;

// the radius of Gaussian filter kernel; 
// Gaussian filter mask will be (2*radius+1)x(2*radius+1).
// People use 2 or 3 most.
static float SIFT_GAUSSIAN_FILTER_RADIUS = 3.0f;

// default threshold on keypoint contrast |D(x)|
static float SIFT_CONTR_THR = 8.0f; //8.0f;

// default threshold on keypoint ratio of principle curvatures
static float SIFT_CURV_THR = 10.0f;

// The keypoint refinement smaller than this threshold will be discarded.
static float SIFT_KEYPOINT_SUBPiXEL_THR = 0.6f;

// double image size before pyramid construction?
static bool SIFT_IMG_DBL = false; //true;

// assumed gaussian blur for input image
static float SIFT_INIT_SIGMA = 0.5f;

// width of border in which to ignore keypoints
static int SIFT_IMG_BORDER = 5;

// maximum steps of keypoint interpolation before failure
static int SIFT_MAX_INTERP_STEPS = 5;

// default number of bins in histogram for orientation assignment
static int SIFT_ORI_HIST_BINS = 36;

// determines gaussian sigma for orientation assignment
static float SIFT_ORI_SIG_FCTR = 1.5f; // Can affect the orientation computation.

// determines the radius of the region used in orientation assignment
static float SIFT_ORI_RADIUS = 3 * SIFT_ORI_SIG_FCTR; // Can affect the orientation computation.

// orientation magnitude relative to max that results in new feature
static float SIFT_ORI_PEAK_RATIO = 0.8f;

// maximum number of orientations for each keypoint location
//static const float SIFT_ORI_MAX_ORI = 4;

// determines the size of a single descriptor orientation histogram
static float SIFT_DESCR_SCL_FCTR = 3.f;

// threshold on magnitude of elements of descriptor vector
static float SIFT_DESCR_MAG_THR = 0.2f;

// factor used to convert floating-point descriptor to unsigned char
static float SIFT_INT_DESCR_FCTR = 512.f;

// default width of descriptor histogram array
static int SIFT_DESCR_WIDTH = 4;

// default number of bins per histogram in descriptor array
static int SIFT_DESCR_HIST_BINS = 8;

// default value of the nearest-neighbour distance ratio threshold
// |DR_nearest|/|DR_2nd_nearest|<SIFT_MATCH_NNDR_THR is considered as a match.
static float SIFT_MATCH_NNDR_THR = 0.65f;

#if 0
// intermediate type used for DoG pyramids
typedef short sift_wt;
static const int SIFT_FIXPT_SCALE = 48;
#else
// intermediate type used for DoG pyramids
typedef float sift_wt;
static const int SIFT_FIXPT_SCALE = 1;
#endif

/****************************************
 * Definitions
 ***************************************/
#define DEGREE_OF_DESCRIPTORS (128)
typedef struct _SiftKeypoint
{
	int octave; // octave number
	int layer;  // layer number
	float rlayer; // real number of layer number

	float r; // normalized row coordinate
	float c; // normalized col coordinate
	float scale; // normalized scale

	float ri;	//row coordinate in that layer. 
	float ci;	//column coordinate in that layer.
	float layer_scale; // the scale of that layer

	float ori; // orientation in degrees.
	float mag; // magnitude
//	float response; //D(x) = D + 1/2 * dD/dx * x
	float descriptors[DEGREE_OF_DESCRIPTORS];
} SiftKeypoint;

typedef struct _MatchPair
{
	int r1;
	int c1;
	int r2;
	int c2;
} MatchPair;

/****************************************
 * Functions
 ***************************************/
// Image operations
// Get pixel from an image with uchar datatype.
inline uchar get_pixel(
	uchar * imageData, 
	int w, int h, 
	int r, int c)
{	
	uchar val; 
	if ( c >= 0 && c < w && r >= 0 && r < h)
	{
		val = imageData[r * w + c];
	}else if (c < 0){
		val = imageData[r * w];
	}else if (c >= w){
		val = imageData[r * w + w - 1];
	}else if (r < 0){
		val = imageData[c];
	}else if (r >= h){
		val = imageData[(h-1) * w + c];
	}else{
		val = 0;
	}
	return val;
}

// Get pixel value from an image with float data type.
inline float get_pixel_f(
	float * imageData, 
	int w, int h, 
	int r, int c)
{	
	float val; 
	if ( c >= 0 && c < w && r >= 0 && r < h)
	{
		val = imageData[r * w + c];
	}else if (c < 0){
		val = imageData[r * w];
	}else if (c >= w){
		val = imageData[r * w + w - 1];
	}else if (r < 0){
		val = imageData[c];
	}else if (r >= h){
		val = imageData[(h-1) * w + c];
	}else{
		val = 0.0f;
	}
	return val;
}


int combine_image(
	ImageObj<uchar> & out_image,
	const ImageObj<uchar> & image1,
	const ImageObj<uchar> & image2);

void  draw_keypoints_to_ppm_file(
	char* out_filename,
	const ImageObj<uchar> & image, 
	list<SiftKeypoint> kpt_list);

int draw_match_lines_to_ppm_file(
	char * filename,
	ImageObj<uchar> & image1,
	ImageObj<uchar> & image2,
	list<MatchPair> & match_list);

int draw_line_to_rgb_image(
	uchar* & data,
	int w, int h,
	MatchPair & mp);

int draw_line_to_image(
	ImageObj<uchar> & image, 
	MatchPair & mp);


// SIFT processing helper functions
void init_sift_parameters(
	bool doubleFirstOctave = true,
	float contrast_threshold = 8.0f,
	float edge_threshold = 10.0f,
	float match_NDDR_threshold = 0.6f);

void double_original_image(bool doubleFirstOctave);

int gaussian_blur(
	const ImageObj<float> & in_image, 
	ImageObj<float> & out_image, 
	std::vector<float> coef1d);

// More efficient implementation
// 1. Use row buf to handle border pixel.
// 2. hori processing and transpose
int gaussian_blur_improved(
	const ImageObj<float> & in_image, 
	ImageObj<float> & out_image, 
	std::vector<float> coef1d);

int row_filter_transpose(
	float * src, 
	float * dst, 
	int w, int h,
	float * coef1d, int gR);

int extract_descriptor( 
	vector<ImageObj<float> > & gpyr, 
	int nOctaves,
	int nGpyrLayers,
	list<SiftKeypoint> & kpt_list);

int extract_descriptor_with_gradient(
	vector< ImageObj<float> > & grdPyr, 
	vector< ImageObj<float> > & rotPyr, 
	int nOctaves,
	int nGpyrLayers,
	list<SiftKeypoint> & kpt_list);

int extract_descriptor_with_gradient_circular_buf(
	vector< ImageObj<float> > & grdPyr, 
	vector< ImageObj<float> > & rotPyr, 
	int nOctaves,
	int nGpyrLayers,
	list<SiftKeypoint> & kpt_list);

int build_octaves(const ImageObj<uchar> &image, 
	vector<ImageObj<uchar> > & octaves, 
	int firstOctave, 
	int nOctaves);

vector< vector<float> > init_sigmad(int nOctaves, int nGpyrLayers);
vector< vector<float> > compute_gaussian_coefs(int nOctaves, int nGpyrLayers, vector<float> sig_vec);
vector< vector<float> > compute_gaussian_coefs2(int nOctaves, int nGpyrLayers);

int build_gaussian_pyramid(
	vector<ImageObj<uchar> > & octaves,
	vector<ImageObj<float> > & gpyr, 
	int nOctaves, 
	int nGpyrLayers);

int build_gaussian_pyramid2(
	vector<ImageObj<uchar> > & octaves,
	vector<ImageObj<float> > & gpyr, 
	int nOctaves, 
	int nGpyrLayers);

int build_dog_pyr(
	vector<ImageObj<float> > & gpyr, 
	vector<ImageObj<float> > & dogPyr, 
	int nOctaves, 
	int nDogLayers);


int build_grd_rot_pyr(
	vector<ImageObj<float> > & gpyr, 
	vector<ImageObj<float> > & grdPyr, 
	vector<ImageObj<float> > & rotPyr, 
	int nOctaves, 
	int nLayers);

int build_dog_grd_rot_pyr(
	vector<ImageObj<float> > & gpyr, 
	vector<ImageObj<float> > & dogPyr,
	vector<ImageObj<float> > & grdPyr, 
	vector<ImageObj<float> > & rotPyr, 
	int nOctaves, 
	int nLayers);

int detect_keypoints(
	vector< ImageObj<float> > & dogPyr, 
	vector< ImageObj<float> > & grdPyr, 
	vector< ImageObj<float> > & rotPyr, 
	int nOctaves, 
	int nDogLayers, 
	list<SiftKeypoint> & kpt_list);

bool refine_local_extrema(
	vector<ImageObj<float> > & dogPyr, 
	int nOctaves, 
	int nDogLayers, 
	SiftKeypoint & kpt);

int export_kpt_list_to_file(
	const char * filename, 
	list<SiftKeypoint> & kpt_list,
	bool bIncludeDescpritor);

float compute_orientation_hist(
	const ImageObj<float> & image, 
	SiftKeypoint & kpt,
	float * & hist);

// SIFT major function
int sift_cpu(
	const ImageObj<uchar> &image, 
	list<SiftKeypoint> & kpt_list, 
	bool bExtractDescriptors);

int sift_cpu_benchmark_time(
	BenchmarkTime & bt,
	const ImageObj<uchar> &image, 
	list<SiftKeypoint> & kpt_list, 
	bool bExtractDescriptors);

int match_keypoints(
	list<SiftKeypoint> & kpt_list1,
	list<SiftKeypoint> & kpt_list2,
	list<MatchPair> & match_list);


// Benchmark
int extract_descriptor_count_gradient_usage(
	vector< ImageObj<float> > & gpyr, 
	int nOctaves,
	int nGpyrLayers,
	list<SiftKeypoint> & kpt_list);

#endif