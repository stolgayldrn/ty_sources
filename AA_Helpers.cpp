#include "AA_Helpers.h"

int readImagesOnYearFolderDecider(std::vector<std::string>yearsList, std::string readFolderPath, unsigned char* descs,
	vl_size &numTotalDesc, vl_size maxTotalDesc, unsigned int dimOfDesc, FeatureType ft, unsigned int sizeOfDescType)
{
	for (int y = 0; y < yearsList.size(); y++)
	{
		std::string yearPath = readFolderPath + "/" + yearsList[y];
		if (FileExist(yearPath.c_str()))
			readImagesOnYearFolder(yearPath, descs, numTotalDesc, maxTotalDesc, dimOfDesc, ft, sizeOfDescType);
	}
	return 1;
}

int readImagesOnYearFolder(std::string readFolderPath, unsigned char* descs, vl_size &numTotalDesc, vl_size maxTotalDesc,
	unsigned int dimOfDesc, FeatureType ft, unsigned int sizeOfDescType)
{
	int totalFoldCnt = 365;
	std::vector<std::string> monthList;
	GET_FolderList(readFolderPath.c_str(), monthList);
	int iCnt = 0;
	int dayCnt = 0;
	int nthr = omp_get_max_threads();
	omp_set_dynamic(0);     // Explicitly disable dynamic teams
	omp_set_num_threads(nthr); // Use 4 threads for all consecutive parallel regions
	bool descLimitReached = false;
	for (int m = 0; m < monthList.size() && !descLimitReached; m++)
	{
		std::string monthPath = readFolderPath + "/" + monthList[m];
		std::vector<std::string> dayList;
		GET_FolderList(monthPath.c_str(), dayList);
		clock_t start = clock();
		double duration;

		for (int d = 0; d < dayList.size() && !descLimitReached; d++)
		{
			std::string dayPath = monthPath + "/" + dayList[d];
			std::vector<std::string> imgList;
			GET_DirectoryImages(dayPath.c_str(), imgList);
#pragma omp parallel for
			for (int i = 0; i < imgList.size(); i++)
			{
				std::string imgPath = dayPath + "/" + imgList[i];

				/*uchar_descriptors imgDescriptor(imgPath.c_str(), "", ft);
				imgDescriptor.setResizeImage(true);
				int excCode = imgDescriptor.ExtractAKAZE();

				iCnt++;
				if (excCode == 1){
				unsigned int numDesc = imgDescriptor.GetNumOfDescriptors();
				// using try catch in here can slow down process
				memcpy(descs + (numTotalDesc * dimOfDesc), imgDescriptor.GetUCHAR_descriptors(), (numDesc)*dimOfDesc*sizeOfDescType);
				numTotalDesc += numDesc;
				}*/

				std::vector<cv::DMatch > gm_12, gm_13, gm_23;
				std::vector<cv::DMatch > inMatches;
				cv::Mat img1 = cv::imread(imgPath.c_str());
				
				uchar_descriptors::resizeImage(&img1, 1200);
				uchar_descriptors descriptor_1(imgPath.c_str(), img1, "", AKAZE_FEATS);
				int status = findIntersectedFeatures(imgPath.c_str(), img1, descriptor_1, inMatches);
				unsigned int numDesc = inMatches.size();
				if (numTotalDesc + numDesc >= maxTotalDesc)
				{
					printf("\n!!!!!!!! Number of total descriptors is reached to limit.");
					descLimitReached = true;
					continue;
				}
				if (numDesc > 0)
				{
					for (int f = 0; f < inMatches.size(); f++)
					{
						memcpy(descs + (numTotalDesc * dimOfDesc), descriptor_1.GetUCHAR_descriptors() + (inMatches[f].queryIdx*dimOfDesc), dimOfDesc*sizeOfDescType);
						numTotalDesc++;
					}
				}
				iCnt++;
			}
			dayCnt++;
			printf("\nProcess Rate in a year folder: %2f", (dayCnt / (totalFoldCnt / 100.0)));
			printf("\nTotal processed image count is: %d .", static_cast<int>(iCnt));
			printf("\nLast folder path is: %s ", dayPath.c_str());
		}
		printf("\nTotal num of Descriptors: %d .", static_cast<int>(numTotalDesc));
		printf("\nTotal processed image count is: %d .", static_cast<int>(iCnt));
		duration = (std::clock() - start) / double(CLOCKS_PER_SEC);
		printf("\nDuration: %f\n\n", duration);
	}
	return 1;
}
