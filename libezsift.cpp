

#include "ezsift.h"
#include "siftfast.h"

Keypoint extract_ezsift(unsigned char* image, int width, int height, int widthStep, int* numKeyPts)
{
	int i=0, j=0;
	Keypoint keypts=0;

	ImageObj<uchar> image1;
	image1.init(width, height);
	
	for (i=0; i<height; i++)
	{
		unsigned char* i1row = &image1.data[i*width];
		unsigned char* irow = &image[i*widthStep];
		for (j=0; j<width; j++)
			i1row[j] = irow[j];
	}

	list<SiftKeypoint> kpt_list;
	sift_cpu(image1, kpt_list, true);
	int listSize = kpt_list.size();

	if (listSize>0)
	{
		keypts = (Keypoint)calloc(listSize, sizeof(KeypointSt));

		list<SiftKeypoint>::iterator p;

		for (i=0, p = kpt_list.begin(); p != kpt_list.end(); p ++)
		{
			keypts[i].scale = p->scale;
			keypts[i].ori = p->ori;
			keypts[i].row = p->r;
			keypts[i].col = p->c;

			for (j=0; j<128; j++)
				keypts[i].descrip[j] = p->descriptors[j];

			i++;
		}
	}
	

	kpt_list.clear();
	image1.release();
	*numKeyPts = listSize;
	return keypts;
}