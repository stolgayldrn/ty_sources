#include "AA_Helpers.h"

int readImagesOnYearFolderDecider(std::vector<std::string>yearsList, std::string readFolderPath, unsigned char* descs,
	vl_size &numTotalDesc, vl_size maxTotalDesc, unsigned int dimOfDesc, FeatureType ft, unsigned int sizeOfDescType)
{
	for (int y = 0; y < yearsList.size(); y++)
	{
		std::string yearPath = readFolderPath + "/" + yearsList[y];
		if (fileExist(yearPath.c_str()))
			readImagesOnYearFolder(yearPath, descs, numTotalDesc, maxTotalDesc, dimOfDesc, ft, sizeOfDescType);
	}
	return 1;
}

void addDescriptorsOfResizedImageToBuffer(unsigned char* descs, vl_size& numTotalDesc, unsigned dimOfDesc, unsigned sizeOfDescType, std::string imgPath, cv::Mat imgOrg, int& excCode, double sizeImg)
{
	cv::Mat img1200 = imgOrg.clone();
	UcharDescriptors::resizeImage(&img1200, sizeImg);
	UcharDescriptors desc1200(imgPath.c_str(), img1200, "", AKAZE_FEATS);
	excCode = desc1200.extractAKAZE();
	if (excCode == 1){
		unsigned int numDesc = desc1200.getNumOfDescriptors();
		// using try catch in here can slow down process
		memcpy(descs + (numTotalDesc * dimOfDesc), desc1200.getUcharDescriptors(), (numDesc)*dimOfDesc*sizeOfDescType);
		numTotalDesc += numDesc;
	}
}

void writeClusterOfDescriptors(unsigned char* buffer, vl_size& numTotalDesc, unsigned dimOfDesc, unsigned sizeOfDescType, std::string wrName)
{
	std::string wrPath = wrName + ".dat";
	FILE* f = new FILE();
	fopen_s(&f, wrPath.c_str(), "wb");
	fwrite(buffer, sizeof(unsigned char), numTotalDesc * 61, f);
	fclose(f);
	numTotalDesc = 0;
	printf("\nThread: %d,  numTotalDesc: %d _______________", omp_get_thread_num(), (int)numTotalDesc);
}

void addBufferAndWriteCluster(unsigned char* buffer, vl_size& numTotalDesc, unsigned dimOfDesc, unsigned sizeOfDescType, 
	std::string wrName, UcharDescriptors& descOrg, bool wrFile)
{
	unsigned int numOfDesc = descOrg.getNumOfDescriptors();
	memcpy(buffer + (numTotalDesc * dimOfDesc), descOrg.getUcharDescriptors(), (numOfDesc)*dimOfDesc*sizeOfDescType);
	numTotalDesc += numOfDesc;
	if (wrFile)
	writeClusterOfDescriptors(buffer, numTotalDesc, dimOfDesc, sizeOfDescType, wrName);
}

int readImagesOnYearFolder(std::string readFolderPath, unsigned char* buffer, vl_size &numTotalDesc, vl_size maxTotalDesc,
	unsigned int dimOfDesc, FeatureType ft, unsigned int sizeOfDescType)
{
	int n_thread = 32;
	omp_set_dynamic(0);     // Explicitly disable dynamic teams
	omp_set_num_threads(n_thread); // Use 4 threads for all consecutive parallel regions
	bool desc_limit_reached = false;

	int total_fold_cnt = 365;
	std::vector<std::string> month_list;
	getFolderList(readFolderPath.c_str(), month_list);
	int i_cnt = 0;
	int day_cnt = 0;
	
	vl_size max_allocated= 1000000;//1M
	//vl_size block_size_orig = max_allocated * 61 * 100;
	vl_size block_size_1200 = max_allocated * 61 * 10;
	vl_size block_size__800 = max_allocated * 61;
	vl_size block_size__600 = max_allocated * 61;


	//unsigned char* buffer_orig = new unsigned char[n_thread * block_size_orig];
	unsigned char* buffer_1200 = new unsigned char[n_thread * block_size_1200];
	unsigned char* buffer__800 = new unsigned char[n_thread * block_size__800];
	unsigned char* buffer__600 = new unsigned char[n_thread * block_size__600];

	for (int t = 0; t < (n_thread*block_size_1200); t++)
	{
		unsigned char temp = 3;
		try{ memcpy(buffer_1200 + t, &temp, 1); }
		catch (cv::Exception e){printf("\nFailed on memory writing test: %s", e.what());}
	}

	std::string w_base_dir = "C:\\Users\\Administrator\\MU5\\";
	//std::string w_base_dir = "C:\\Users\\Tolga\\MU4\\";
	

	for (int m = 0; m < month_list.size() && !desc_limit_reached; m++)
	{
		std::string month_path = readFolderPath + "/" + month_list[m];
		std::vector<std::string> day_list;
		getFolderList(month_path.c_str(), day_list);
		clock_t start = clock();
		double duration;
#pragma omp parallel for
		for (int d = 0; d < day_list.size(); d++)
		{
			std::string day_path = month_path + "/" + day_list[d];
			std::vector<std::string> img_list;
			getDirectoryImages(day_path.c_str(), img_list);

			/**/
			//std::string wr_name_orig = w_base_dir + "descs_orig__" + month_list[m] + "_" + day_list[d] + ".dat";
			std::string wr_name_1200 = w_base_dir + "descs_1200__" + month_list[m] + "_" + day_list[d] + ".dat";
			std::string wr_name__800 = w_base_dir + "descs__800__" + month_list[m] + "_" + day_list[d] + ".dat";
			std::string wr_name__600 = w_base_dir + "descs__600__" + month_list[m] + "_" + day_list[d] + ".dat";
			vl_size n_total_desc_orig = 0;
			vl_size n_total_desc_1200 = 0;
			vl_size n_total_desc__800 = 0;
			vl_size n_total_desc__600 = 0;
			for (int i = 0; i < img_list.size(); i++)
			{
				std::string img_path = day_path + "/" + img_list[i];
				//1
				/*uchar_descriptors imgDescriptor(imgPath.c_str(), "", ft);
				imgDescriptor.setResizeImage(true);
				int excCode = imgDescriptor.ExtractAKAZE();

				iCnt++;
				if (excCode == 1){
				unsigned int numDesc = imgDescriptor.GetNumOfDescriptors();
				// using try catch in here can slow down process
				memcpy(buffer + (numTotalDesc * dimOfDesc), imgDescriptor.GetUCHAR_descriptors(), (numDesc)*dimOfDesc*sizeOfDescType);
				numTotalDesc += numDesc;
				}//*/
				//2
				i_cnt++;
				cv::Mat img_orig = cv::imread(img_path.c_str());

				//UcharDescriptors desc_orig(img_path.c_str(), img_orig, "", AKAZE_FEATS);
				//int excCode = desc_orig.extractAKAZE();
				
				cv::Mat img_1200 = img_orig.clone();
				UcharDescriptors::resizeImage(&img_1200, 1200);
				UcharDescriptors desc_1200(img_path.c_str(), img_1200, "", AKAZE_FEATS);
				int exc_code_1200 = desc_1200.extractAKAZE();

				cv::Mat img__800 = img_orig.clone();
				UcharDescriptors::resizeImage(&img__800, 800);
				UcharDescriptors desc__800(img_path.c_str(), img__800, "", AKAZE_FEATS);
				int exc_code__800 = desc__800.extractAKAZE();

				cv::Mat img__600 = img_orig.clone();
				UcharDescriptors::resizeImage(&img__600, 600);
				UcharDescriptors desc__600(img_path.c_str(), img__600, "", AKAZE_FEATS);
				int exc_code__600 = desc__600.extractAKAZE();
				bool wFile = false;
				if (i == img_list.size() - 1) wFile = true;
				
				if (exc_code_1200 == 1){
					// using try catch in here can slow down process
					printf("\nThread: %d, numOfDesc: %d, numTotalDesc: %d >>>>", omp_get_thread_num(), desc_1200.getNumOfDescriptors(), (int)n_total_desc_1200);
					int this_thread = omp_get_thread_num();
					//addBufferAndWriteCluster(buffer_orig + (this_thread * block_size_orig), n_total_desc_orig, dimOfDesc, sizeOfDescType, wr_name_orig, desc_orig, wFile);
					try
					{
						addBufferAndWriteCluster(buffer_1200 + (this_thread * block_size_1200), n_total_desc_1200, dimOfDesc, sizeOfDescType, wr_name_1200, desc_1200, wFile);
						addBufferAndWriteCluster(buffer__800 + (this_thread * block_size__800), n_total_desc__800, dimOfDesc, sizeOfDescType, wr_name__800, desc__800, wFile);
						addBufferAndWriteCluster(buffer__600 + (this_thread * block_size__600), n_total_desc__600, dimOfDesc, sizeOfDescType, wr_name__600, desc__600, wFile);
					}
					catch (cv::Exception e){printf("\nFailed at memory writing: %s", e.what());}
					printf("\nThread: %d,  numTotalDesc: %d <<<<", omp_get_thread_num(), (int)n_total_desc_1200);
				}
				int progres = (i * 100) / img_list.size();
				if ((i % 100) == 0 ) printf("\n_______________________________Progres in day folder: %d, Image List Size: %d", progres, img_list.size());
				//Intersected Features
				/*
				std::vector<cv::DMatch > gm_12, gm_13, gm_23;
				std::vector<cv::DMatch > inMatches;
				cv::Mat img = cv::imread(imgPath.c_str());
				cv::Mat img1 = img.clone();
				UcharDescriptors::resizeImage(&img1, 1200);
				UcharDescriptors descriptor_1(imgPath.c_str(), img1, "", AKAZE_FEATS);
				int status = findIntersectedFeatures(imgPath.c_str(), img, descriptor_1, inMatches);
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
						memcpy(buffer + (numTotalDesc * dimOfDesc), descriptor_1.getUcharDescriptors() + (inMatches[f].queryIdx*dimOfDesc), dimOfDesc*sizeOfDescType);
						numTotalDesc++;
					}
				}
				iCnt++;
				//*/
			}
			day_cnt++;
			printf("\nProcess Rate in a year folder: %2f", (day_cnt / (total_fold_cnt / 100.0)));
			printf("\nTotal processed image count is: %d .", static_cast<int>(i_cnt));
			printf("\nLast folder path is: %s ", day_path.c_str());
		}
		printf("\nTotal num of Descriptors: %d .", static_cast<int>(numTotalDesc));
		printf("\nTotal processed image count is: %d .", static_cast<int>(i_cnt));
		duration = (std::clock() - start) / double(CLOCKS_PER_SEC);
		printf("\nDuration: %f\n\n", duration);
	}
	return 1;
}
