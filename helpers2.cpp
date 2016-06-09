#include "helpers2.h"

const char *getFileNameExt(const char *filename) 
{
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}
//
int isSigFile(const char *filename)
{
	const char* ext = getFileNameExt(filename);
	size_t lenExt = strlen(ext);

	if (lenExt==3)
	{
		if ( 
			((ext[0]=='s' || ext[0]=='S') && (ext[1]=='i' || ext[1]=='I') && (ext[2]=='g' || ext[2]=='G') ) )
		{
			return 1;
		}
	}	
	return 0;
}

int isDscFile(const char *filename)
{
	const char* ext = getFileNameExt(filename);
	size_t lenExt = strlen(ext);

	if (lenExt==3)
	{
		if ( 
			((ext[0]=='d' || ext[0]=='D') && (ext[1]=='s' || ext[1]=='S') && (ext[2]=='c' || ext[2]=='c') ) )
		{
			return 1;
		}
	}	
	return 0;
}

int isImageFile(const char *filename)
{
	const char* ext = getFileNameExt(filename);
	int lenExt = strlen(ext);

	if (lenExt==3)
	{
		if ( 
			((ext[0]=='b' || ext[0]=='B') && (ext[1]=='m' || ext[1]=='M') && (ext[2]=='p' || ext[2]=='P') ) ||
			((ext[0]=='j' || ext[0]=='J') && (ext[1]=='p' || ext[1]=='P') && (ext[2]=='g' || ext[2]=='G') ) ||
			((ext[0]=='p' || ext[0]=='P') && (ext[1]=='n' || ext[1]=='N') && (ext[2]=='g' || ext[2]=='G') )
			)
		{
			return 1;
		}
	}
	else if (lenExt==4)
	{
		if ( 
			((ext[0]=='j' || ext[0]=='J') && (ext[1]=='p' || ext[1]=='P') && (ext[2]=='e' || ext[2]=='E') && (ext[3]=='g' || ext[3]=='G') ) 
			)
		{
			return 1;
		}
	}

	return 0;
}

int fileExist (const char *filename)
{
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}

//// "tcp://192.168.1.2:5555"
//void connect_socket(const char* ip, void** context, void** requester)
//{
//	*context = zmq_ctx_new ();
//
//	//  Socket to talk to server
//    printf ("Listening...\n");
//    *requester = zmq_socket (*context, ZMQ_REP);
//	zmq_bind(*requester, ip);
//}
//
//int recv_socket(void* requester, char** text)
//{
//	zmq_msg_t reply;
//	zmq_msg_init (&reply);
//	zmq_msg_recv (&reply, requester, 0);
//	char* result = (char*)zmq_msg_data (&reply);
//
//	size_t len = zmq_msg_size(&reply);
//	(*text) = new char[len+4];
//	memcpy(*text, result, len*sizeof(char));
//	(*text)[len]='\0';
//
//	//printf ("Received World %s\n", text);
//	//printf ("Received World %s\n", js);
//
//	zmq_msg_close (&reply);
//
//	return len;
//}

//void send_socket(void* requester, const char* text)
//{
//	zmq_msg_t request;
//	zmq_msg_init_size (&request, strlen(text));
//	memcpy (zmq_msg_data (&request), text, strlen(text));
//
//	zmq_msg_send (&request, requester, 0);
//	zmq_msg_close (&request);
//}

//void close_socket(void** requester, void** context)
//{
//	zmq_close (*requester);
//    zmq_ctx_destroy (*context);
//
//	*requester = 0;
//	*context = 0;
//}

/////////////////////////////////////// NEW FUNCTIONS GO BEYOND THIS POINT //////////////////////////////
int getDirectorySignatures(const char* Folder, std::vector<std::string> &fileList)
{
	int i=0;
	DIR* dir;	
	struct dirent *ent;
	int numFiles=0;
	dir = opendir(Folder);

	fileList.clear(); //
	if (dir) 
	{
		/* print all the files and directories within directory */
		while ((ent = readdir (dir))) 
		{
			struct dirent *temp=(struct dirent*)malloc(sizeof(struct dirent));
			strcpy_s(temp->d_name,ent->d_name);
			temp->d_namlen=ent->d_namlen;
			temp->d_type=ent->d_type;

			switch (ent->d_type) 
			{
			case DT_REG:
				if (isSigFile(temp->d_name))
				{
					fileList.push_back(std::string( temp->d_name ));
					numFiles++;
					break;
				}
			default:
				break;
			}
		}
		closedir (dir);
	}
	else
	{
		printf("Couldn't open folder!\n");
		return 0;
	}

	return i;
}

int getDirectoryDSCs(const char* Folder, std::vector<std::string> &fileList)
{
	int i=0;
	DIR* dir;	
	struct dirent *ent;
	int numFiles=0;
	dir = opendir(Folder);

	fileList.clear(); //
	if (dir) 
	{
		/* print all the files and directories within directory */
		while ((ent = readdir (dir))) 
		{
			struct dirent *temp=(struct dirent*)malloc(sizeof(struct dirent));
			strcpy_s(temp->d_name,ent->d_name);
			temp->d_namlen=ent->d_namlen;
			temp->d_type=ent->d_type;

			switch (ent->d_type) 
			{
			case DT_REG:
				if (isDscFile(temp->d_name))
				{
					fileList.push_back(std::string( temp->d_name ));
					numFiles++;
					break;
				}
			default:
				break;
			}
		}
		closedir (dir);
	}
	else
	{
		printf("Couldn't open folder!\n");
		return 0;
	}
	return i;
}

int getDirectoryImages(const char* Folder, std::vector<std::string> &fileList)
{
	int i=0;
	DIR* dir;	
	struct dirent *ent;
	int numFiles=0;
	dir = opendir(Folder);

	fileList.clear();
	if (dir) 
	{
		/* print all the files and directories within directory */
		while ((ent = readdir (dir))) 
		{
			struct dirent *temp=(struct dirent*)malloc(sizeof(struct dirent));
			strcpy_s(temp->d_name,ent->d_name);
			temp->d_namlen=ent->d_namlen;
			temp->d_type=ent->d_type;

			switch (ent->d_type) 
			{
			case DT_REG:
				if (isImageFile(temp->d_name))
				{
					fileList.push_back(std::string( temp->d_name ));
					numFiles++;
					break;
				}
			default:
				break;
			}

			delete temp;
		}
		closedir (dir);
		delete ent;
	}
	else
	{
		printf("Couldn't open folder!\n");
		return 0;
	}

	return i;
}

int getFolderList(const char* Folder, std::vector<std::string> &fileList)
{
	int i=0;
	DIR* dir;	
	struct dirent *ent;
	int numFiles=0;
	dir = opendir(Folder);

	fileList.clear();
	if (dir) 
	{
		/* print all the files and directories within directory */
		while ((ent = readdir (dir))) 
		{
			struct dirent *temp=(struct dirent*)malloc(sizeof(struct dirent));

			//memcpy(temp->d_name,ent->d_name,sizeof(TChar)*ent->d_namlen);
			strcpy_s(temp->d_name,ent->d_name);
			temp->d_namlen=ent->d_namlen;
			temp->d_type=ent->d_type;
			switch (ent->d_type) 
			{
			case DT_DIR:
				
				if(strcmp(ent->d_name,".") && strcmp(ent->d_name , ".."))
				{
					fileList.push_back(std::string( ent->d_name ));
					//t_list_push_list(&fileList,temp);
					numFiles++;
				}
				
			default:
				break;
			}
			delete[] temp;
		}
		closedir (dir);
	}
	else
	{
		printf("Couldn't open folder!\n");
		return 0;
	}

	return i;
}

int read_sig(const char* FileName, unsigned int *numdesc, unsigned char** siftDescByte, float** xCoords, float** yCoords, float** orientations, float** scales)
{
	FILE *f;
	fopen_s(&f,FileName, "rb");

	if (f)
	{
		unsigned long long hash = 0;
		fread(numdesc, sizeof(unsigned int), 1, f);
		unsigned char* siftu = new unsigned char[(*numdesc+4)*128];
		fread(siftu, sizeof(unsigned char), (*numdesc)*128, f);
		fread(&hash, sizeof(unsigned long long), 1, f);

		*xCoords = 0;
		*yCoords = 0;
		*orientations = 0;
		*scales = 0;
		if (!feof(f))
		{
			(*xCoords) = new float[(*numdesc+4)];
			(*yCoords) = new float[(*numdesc+4)];
			(*orientations) = new float[(*numdesc+4)];
			(*scales) = new float[(*numdesc+4)];
			// read the geometry information
			fread((*xCoords), sizeof(float), *numdesc, f);
			fread((*yCoords), sizeof(float), *numdesc, f);
			fread((*orientations), sizeof(float), *numdesc, f);
			fread((*scales), sizeof(float), *numdesc, f);
		}

		fclose(f);

		*siftDescByte = siftu;
	}
	else
	{
		printf("Cannot read file: %s\n", FileName);
		return -1;
	}

	return 0;
}
////////////////////////////////////////////////////////////////
/*Convert*/

std::string int2string(int num)
{
	std::stringstream ss;
	ss << num;
	std::string str = ss.str();
	return str;
}

std::string uint2string(uint num)
{
	std::stringstream ss;
	ss << num;
	std::string str = ss.str();
	return str;
}

std::string longlongint2string(long long int num)
{
	std::stringstream ss;
	ss << num;
	std::string str = ss.str();
	return str;
}

std::string float2string(float number)
{
	std::ostringstream buff;
	buff << number;
	return buff.str();
}
////////////////////////////////////////////////////////////////

cv::Mat makeCanvas(std::vector<cv::Mat>& vecMat, int windowHeight, int nRows) {
	int N = vecMat.size();
	nRows  = nRows > N ? N : nRows; 
	int edgeThickness = 10;
	int imagesPerRow = ceil(double(N) / nRows);
	int resizeHeight = floor(2.0 * ((floor(double(windowHeight - edgeThickness) / nRows)) / 2.0)) - edgeThickness;
	int maxRowLength = 0;

	std::vector<int> resizeWidth;
	for (int i = 0; i < N;) {
		int thisRowLen = 0;
		for (int k = 0; k < imagesPerRow; k++) {
			double aspectRatio = double(vecMat[i].cols) / vecMat[i].rows;
			int temp = int( ceil(resizeHeight * aspectRatio));
			resizeWidth.push_back(temp);
			thisRowLen += temp;
			if (++i == N) break;
		}
		if ((thisRowLen + edgeThickness * (imagesPerRow + 1)) > maxRowLength) {
			maxRowLength = thisRowLen + edgeThickness * (imagesPerRow + 1);
		}
	}
	int windowWidth = maxRowLength;
	cv::Mat canvasImage(windowHeight, windowWidth, CV_8UC3, cv::Scalar(0, 0, 0));

	for (int k = 0, i = 0; i < nRows; i++) {
		int y = i * resizeHeight + (i + 1) * edgeThickness;
		int x_end = edgeThickness;
		for (int j = 0; j < imagesPerRow && k < N; k++, j++) {
			int x = x_end;
			cv::Rect roi(x, y, resizeWidth[k], resizeHeight);
			cv::Size s = canvasImage(roi).size();
			// change the number of channels to three
			cv::Mat target_ROI(s, CV_8UC3);
			if (vecMat[k].channels() != canvasImage.channels()) {
				if (vecMat[k].channels() == 1) {
					cvtColor(vecMat[k], target_ROI, cv::COLOR_GRAY2BGR);
				}
			}
			cv::resize(target_ROI, target_ROI, s);
			if (target_ROI.type() != canvasImage.type()) {
				target_ROI.convertTo(target_ROI, canvasImage.type());
			}
			target_ROI.copyTo(canvasImage(roi));
			x_end += resizeWidth[k] + edgeThickness;
		}
	}
	return canvasImage;
}

int dirExists(const char *path)
{
	if( _access( path, 0 ) == 0 ){

		struct stat status;
		stat( path, &status );

		return (status.st_mode & S_IFDIR) != 0;
	}
	return false;
}
int pathControl(std::string Path)
{
	if(!dirExists(Path.c_str()))
	{
		std::string parentFold = Path.substr(0, Path.rfind("\\"));
		pathControl(parentFold);
		std::wstring stemp = std::wstring(Path.begin(), Path.end());
		LPCWSTR szDirPath = stemp.c_str();
		// Create a new directory.
		if(!CreateDirectoryW(szDirPath, NULL))
		{
			printf("\nDirectory error: Couldn't create %S directory.\n", szDirPath);
			return 0;
		}
		else
		{
			printf("\n%S directory successfully created.\n", szDirPath);
			return 1;		
		}
	}
	else
		return 1;
}

void fileCopy(std::string sourePath, std::string destPath)
{
	std::ifstream source(sourePath, std::ios::binary);
	std::ofstream dest(destPath, std::ios::binary);

	std::istreambuf_iterator<char> begin_source(source);
	std::istreambuf_iterator<char> end_source;
	std::ostreambuf_iterator<char> begin_dest(dest);
	std::copy(begin_source, end_source, begin_dest);

	source.close();
	dest.close();
}
//////////////////////////////////////////////////////////////////////////
// WRITE AS CSV
void writeToCSV(std::vector<std::vector<std::string>> dataVV, std::string CSV_Path, int fileNum)
{
	std::ofstream myfile;
	//std::string CSV_Path = fileName;
	myfile.open(CSV_Path.c_str());
	for (int i = 0; i < dataVV.size(); i++){
		std::string lineStr = "";
		for (int ii = 0; ii < fileNum; ii++){
			lineStr += dataVV[i][ii] + ";";
		}
		lineStr += "\n";
		myfile << lineStr;
	}
}

void writeToCSV(std::vector<std::vector<float>> dataVV, char* fileName, int fileNum)
{
	std::ofstream myfile;
	std::string CSV_Path = fileName;
	myfile.open(CSV_Path.c_str());
	for (int i = 0; i < dataVV.size(); i++){
		std::string lineStr = "";
		for (int ii = 0; ii < fileNum; ii++){
			lineStr += std::to_string(dataVV[i][ii]) + ";";
		}
		lineStr += "\n";
		myfile << lineStr;
	}
}

void writeToCSV(std::vector<std::vector<int>> dataVV, char* fileName, int fileNum)
{
	std::ofstream myfile;
	std::string CSV_Path = fileName;
	myfile.open(CSV_Path.c_str());
	for (int i = 0; i < dataVV.size(); i++){
		std::string lineStr = "";
		for (int ii = 0; ii < fileNum; ii++){
			lineStr += std::to_string(dataVV[i][ii]) + ";";
		}
		lineStr += "\n";
		myfile << lineStr;
	}
}
//////////////////////////////////////////////////////////////////////////
// JSON
std::size_t callback(
	const char* in,
	std::size_t size,
	std::size_t num,
	std::string* out)
{
	const std::size_t totalBytes(size * num);
	out->append(in, totalBytes);
	return totalBytes;
}
