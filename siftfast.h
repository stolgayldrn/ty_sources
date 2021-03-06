// Copyright (C) zerofrog(@gmail.com), 2008-2009
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//Lesser GNU General Public License for more details.
//
//You should have received a copy of the GNU Lesser General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.
#ifndef SIFT_FAST_H
#define SIFT_FAST_H

typedef struct ImageSt {
    int rows, cols;          // Dimensions of image.
    float *pixels;          // 2D array of image pixels.
    int stride;             // how many floats until the next row
                            // (used to add padding to make rows aligned to 16 bytes)
} *Image;

typedef struct KeypointSt {
    float row, col;             // Subpixel location of keypoint.
    float scale, ori;           // Scale and orientation (range [-PI,PI])
    float descrip[128];     // Vector of descriptor values
    struct KeypointSt *next;    // Pointer to next keypoint in list.
} *Keypoint;

#ifdef __cplusplus
extern "C" {
#endif

//Keypoint getOpencvKeypoints(Image porgimage);
Keypoint GetKeypoints(Image porgimage, int Scales, float InitSigma);
Image CreateImage(int rows, int cols);
Image CreateImageFromMatlabData(double* pdata, int rows, int cols);
void DestroyAllImages();
void DestroyAllResources();
void FreeKeypoints(Keypoint keypt);
void sift_aligned_free(void* pmem);
long __declspec(dllexport) get_sift_keypoints(unsigned char* img, int width, int ws, int height, int scale, double sigma, void* keys);
long __declspec(dllexport) release_sift(void* keys);
Keypoint __declspec(dllexport) extract_ezsift(unsigned char* image, int width, int height, int widthStep, int* numKeypts);

#ifdef __cplusplus
}
#endif

#endif
