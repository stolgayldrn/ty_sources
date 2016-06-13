#ifndef AA_HELPERS_H
#define AA_HELPERS_H

#include "helpers2.h"
#include "uchar_descriptors.h"
#include <stdio.h>
#include "vl/random.h"
#include "math.h"
#include <omp.h>
#include <ctime>

int readImagesOnYearFolderDecider(std::vector<std::string>yearsList, std::string readFolderPath, unsigned char* descs,
	vl_size &numTotalDesc, vl_size maxTotalDesc, unsigned int dimOfDesc, FeatureType ft, unsigned int sizeOfDescType);
int readImagesOnYearFolder(std::string readFolderPath, unsigned char* buffer, vl_size &numTotalDesc, vl_size maxTotalDesc,
	unsigned int dimOfDesc, FeatureType ft, unsigned int sizeOfDescType);

#endif

