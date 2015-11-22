/*	Copyright (c) 2013, Robert Wang, email: robertwgh (at) gmail.com
	All rights reserved. https://sourceforge.net/p/ezsift

	Description:
	This image class is a helper class designed for image related operations. 
	There is definitely room to optimize these functions. 

	Revision history:
		September, 15, 2013: initial version.
*/

#ifndef IMAGE_H
#define IMAGE_H


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <typeinfo>

//#include "img_io.h"
#include "common.h"

#define IMG_RESIZE_NEAREST	0
#define IMG_RESIZE_LINEAR	1

using namespace std;
//typedef unsigned char uchar;

#undef uchar
typedef unsigned char	uchar;

template <typename T> 
class ImageObj
{
public:
	int w;
	int h;
	T * data;

	ImageObj();

	ImageObj(int _w, int _h);

	// Copy construction function
	ImageObj(const ImageObj<T> & img);

	~ImageObj();

	ImageObj<T> & operator=(const ImageObj<T> & img);

	void init(int _w, int _h);

	void reinit(int _w, int _h);

	void release();

	int read_pgm(const char * filename);
	int write_pgm(const char * filename);

	ImageObj<uchar> to_uchar() const;

	ImageObj<float> to_float() const;

	// Upsample the image by 2x, linear interpolation.
	ImageObj<T> upsample_2x() const;

	// Downsample the image by 2x, nearest neighbor interpolation.
	ImageObj<T> downsample_2x() const;

	// Resize image to arbitrary size.
	ImageObj<T> resize(int dstW, int dstH, int resizeMethod = IMG_RESIZE_NEAREST);
};


// Member function definition
template <typename T> 
ImageObj<T>::ImageObj()
{
	w = 0;
	h = 0;
	data = NULL;
}

template <typename T> 
ImageObj<T>::ImageObj(int _w, int _h)
{
	w = _w;
	h = _h;
	data = new T[w * h];
}

// Copy construction function
template <typename T> 
ImageObj<T>::ImageObj(const ImageObj<T> & img)
{
	w = img.w;
	h = img.h;
	data = new T[w * h];
	memcpy(data, img.data, w * h * sizeof(T));
}

template <typename T> 
ImageObj<T>::~ImageObj()
{
	if (data)
	{
		delete [] data;
		data = NULL;
	}
}

template <typename T> 
ImageObj<T> & ImageObj<T>::operator=(const ImageObj<T> & img)
{
	init(img.w, img.h);
	memcpy(data, img.data, img.w * img.h * sizeof(T));
	return *this;
}


template <typename T> 
void ImageObj<T>::init(int _w, int _h)
{
	w = _w;
	h = _h;
	//if (data) delete [] data;
	data = new T[w * h];
}

template <typename T> 
void ImageObj<T>::reinit(int _w, int _h)
{
	w = _w;
	h = _h;
	if (data) delete [] data;
	data = new T[w * h];
}

template <typename T> 
void ImageObj<T>::release()
{
	w = 0;
	h = 0;
	if (data) 
	{
		delete [] data;
		data = NULL;
	}
}


template <typename T> 
int ImageObj<T>::read_pgm(const char * filename)
{
	FILE* in_file;
	char ch, type;
	int dummy;
	uchar * _data;

	in_file = fopen(filename, "rb");
	if (! in_file)
	{
		fprintf(stderr, "ERROR(0): Fail to open file %s\n", filename);
		return -1;
	}
	/* Determine pgm image type (only type three can be used)*/
	ch = getc(in_file);
	if(ch != 'P')
	{
		printf("ERROR(1): Not valid pgm/ppm file type\n");
		return -1;
	}
	ch = getc(in_file);
	/* Convert the one digit integer currently represented as a character to
	* an integer(48 == '0') */
	type = ch - 48;
	if(type != 5)
	{
		printf("ERROR(2): this file type (P%d) is not supported!\n", type);
		return -1;
	}
	while(getc(in_file) != '\n');      /* Skip to end of line*/
	while (getc(in_file) == '#'){       /* Skip comment lines */
		while (getc(in_file) != '\n');
	}
	fseek(in_file, -1, SEEK_CUR);     /* Backup one character*/

	fscanf(in_file,"%d", &w);
	fscanf(in_file,"%d", &h);
	fscanf(in_file,"%d", &dummy);  /* Skipped here */
	while(getc(in_file) != '\n');

	init(w, h);
	if (typeid(T) == typeid(uchar))
	{
		fread(data, sizeof(uchar), (w)*(h), in_file);
	}else{
		_data = new uchar[w * h];
		fread(_data, sizeof(unsigned char), (w)*(h), in_file);
		
		for (int i = 0; i < h; i ++)
		{
			for (int j = 0; j < w; j ++)
			{
				data[i * w + j] = (T)(_data[i * w + j]);
			}
		}
		delete [] _data;
	}

	return 0;
}


template <typename T> 
int ImageObj<T>::write_pgm(const char * filename)
{
	FILE* out_file;

	if (w <= 0 || h <= 0)
	{
		fprintf(stderr, "write_pgm(%s):Invalid image width or height\n", filename);
		return -1;
	}

	out_file = fopen(filename, "wb");
	if (! out_file){
		fprintf(stderr, "Fail to open file: %s\n", filename);
		return -1;
	}

	fprintf(out_file, "P5\n");
	fprintf(out_file, "%d %d\n255\n", w, h);

	ImageObj<uchar> tmpImage;
	tmpImage = this->to_uchar();
	fwrite(tmpImage.data, sizeof(uchar), w * h, out_file);

	fclose(out_file);
	return 0;
}


template <typename T> 
ImageObj<uchar> ImageObj<T>::to_uchar() const
{
	ImageObj<uchar> dstImage(w, h);

	for (int r = 0; r < h; r ++)
	{
		for(int c = 0; c < w; c ++)
		{
			// Negative number, truncate to zero.
			float temp = data[r * w  + c];
			dstImage.data[r * w + c] = temp >= 0 ? (uchar)temp : 0;
		}
	}
	return dstImage;
}

template <typename T> 
ImageObj<float> ImageObj<T>::to_float() const
{
	ImageObj<float> dstImage(w, h);

	for (int i = 0; i < h; i ++)
	{
		for (int j = 0; j < w; j ++)
		{
			dstImage.data[i * w + j] = (float)this->data[i * w + j];
		}
	}
	return dstImage;
}

// Upsample the image by 2x, linear interpolation.
template <typename T> 
ImageObj<T> ImageObj<T>::upsample_2x() const
{
	float scale = 2.0f;

	int srcW = w, srcH = h;
	int dstW = srcW << 1, dstH = srcH << 1;
	ImageObj<T> out_image(dstW, dstH);

	T * srcData = data;
	T * dstData = out_image.data;

	for (int r = 0; r < dstH; r ++)
	{
		for (int c = 0; c < dstW; c ++)
		{
			float ori_r = r / scale;
			float ori_c = c / scale;
			int r1 = (int) ori_r;
			int c1 = (int) ori_c;
			float dr = ori_r - r1;
			float dc = ori_c - c1;

			int idx = r1 * srcW + c1;
			dstData[r * dstW + c] = uchar ((1-dr) * (1-dc) * srcData[idx]
			+ dr * (1-dc) *srcData[idx + srcW]
			+ (1-dr)* dc * srcData[idx + 1]
			+ dr * dc *srcData[idx + srcW + 1]);
		}
	}
	return out_image;
}

// Downsample the image by 2x, nearest neighbor interpolation.
template <typename T> 
ImageObj<T> ImageObj<T>::downsample_2x() const
{
	int srcW = w, srcH = h;
	int dstW = srcW >> 1, dstH = srcH >> 1;
	ImageObj<T> out_image(dstW, dstH);

	T * srcData = data;
	T * dstData = out_image.data;

	for (int r = 0; r < dstH; r ++)
	{
		for (int c = 0; c < dstW; c ++)
		{
			int ori_r = r << 1;
			int ori_c = c << 1;
			dstData[r * dstW + c] = srcData[ori_r * srcW + ori_c];
		}
	}
	return out_image;
}

// Resize image to arbitrary size.
template <typename T> 
ImageObj<T> ImageObj<T>::resize(int dstW, int dstH, int resizeMethod)
{
	ImageObj<T> out_image(dstW, dstH);

	int srcW = w, srcH = h;
	float scaleW = (float) srcW / dstW;
	float scaleH = (float) srcH / dstH;

	uchar * srcData = data;
	uchar * dstData = out_image.data;

	for (int r = 0; r < dstH; r ++)
		for (int c = 0; c < dstW; c ++)
		{
			float ori_r = r * scaleH;
			float ori_c = c * scaleW;
			int r1 = (int) ori_r;
			int c1 = (int) ori_c;

			if (resizeMethod == IMG_RESIZE_LINEAR)
			{
				float dr = ori_r - r1;
				float dc = ori_c - c1;
				dstData[r * dstW + c] = uchar ((1-dr) * (1-dc) * srcData[r1 * srcW + c1]
				+ dr * (1-dc) *srcData[(r1+1)*srcW + c1]
				+ (1-dr)* dc * srcData[r1 * srcW + c1 + 1]
				+ dr * dc *srcData[(r1+1)*srcW + c1 + 1]);
			}else if (resizeMethod == IMG_RESIZE_NEAREST)
			{
				dstData[r * dstW + c] = srcData[r1 * srcW + c1];
			}else // default IMG_RESIZE_NEAREST
			{
				dstData[r * dstW + c] = srcData[r1 * srcW + c1];
			}
		}
		return out_image;
}
#endif