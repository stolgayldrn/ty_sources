/*
Copyright (C) 2015-2020 S.Tolga Yildiran.
All rights reserved.

This file is part of Tolga Yildiran Video Search library and is made available under
the terms of the BSD license (see the COPYING file).
*/
/************************************************************************/
/* Tolga Yildiran														*/
/* 24/05/2016 															*/
/* stolgayldrn@gmail.com												*/
/************************************************************************/
#ifndef HELPERS2_H
#define HELPERS2_H

#include <vector>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <Windows.h>
#include <io.h>					// For access().
#include <sys/types.h>			// For stat().
#include <sys/stat.h>			// For stat().
#include "dirent.h"

// directory
const char *getFileNameExt(const char *filename) ;
int isSigFile(const char *filename);
int isDscFile(const char *filename);
int isImageFile(const char *filename);
int fileExist ( const char *filename);
int getDirectorySignatures(const char* Folder, std::vector<std::string> &fileList);
int getDirectoryDSCs(const char* Folder, std::vector<std::string> &fileList);
int getDirectoryImages(const char* Folder, std::vector<std::string> &fileList);
int getFolderList(const char* Folder, std::vector<std::string> &fileList);
int read_sig(const char* FileName, unsigned int *numdesc, unsigned char** siftDescByte, float** xCoords, float** yCoords, float** orientations, float** scales);
std::string int2string(int num);
std::string uint2string(uint num);
std::string longlongint2string(long long int num);
std::string float2string(float number);
cv::Mat makeCanvas(std::vector<cv::Mat>& vecMat, int windowHeight, int nRows);
int dirExists(const char *path);
int pathControl(std::string Path);
void fileCopy(std::string sourePath, std::string destPath);
void writeToCSV(std::vector<std::vector<std::string>> dataVV, std::string CSV_Path, int fileNum);
void writeToCSV(std::vector<std::vector<float>> dataVV, char* fileName, int fileNum);
void writeToCSV(std::vector<std::vector<int>> dataVV, char* fileName, int fileNum);
std::size_t callback( const char* in, std::size_t size, std::size_t num, std::string* out);

//// socket
//void connect_socket(const char* ip, void** context, void** requester);
//void send_socket(void* requester, const char* text);
//int recv_socket(void* requester, char** text);
//void close_socket(void** requester, void** context);
//

//// Act like matlab's [Y,I] = SORT(X)
//// Input:
////   unsorted  unsorted vector
//// Output:
////   sorted     sorted vector, allowed to be same as unsorted
////   index_map  an index map such that sorted[i] = unsorted[index_map[i]]
//template <class T>
//void indexed_sort(
//				  std::vector<T> &unsorted,
//				  std::vector<T> &sorted,
//				  std::vector<int> &index_map);
//
//// Act like matlab's Y = X[I]
//// where I contains a vector of indices so that after,
//// Y[j] = X[I[j]] for index j
//// this implies that Y.size() == I.size()
//// X and Y are allowed to be the same reference
//template< class T >
//void reorder(
//			 std::vector<T> & unordered, 
//			 std::vector<size_t> const & index_map, 
//			 std::vector<T> & ordered);
//
//// Comparison struct used by sort
//// http://bytes.com/topic/c/answers/132045-sort-get-index
//template<class T> struct index_cmp 
//{
//	index_cmp(T arr) : arr(arr) {}
//	bool operator()(const size_t a, const size_t b) const
//	{ 
//		return arr[a] < arr[b];
//	}
//	T arr;
//};
//
//
//// Comparison struct used by sort
//// http://bytes.com/topic/c/answers/132045-sort-get-index
//template<class T> struct index_cmp_descend 
//{
//	index_cmp_descend(T arr) : arr(arr) {}
//	bool operator()(const size_t a, const size_t b) const
//	{ 
//		return arr[a] > arr[b];
//	}
//	T arr;
//};
//
//
//// sortType = 0 : Ascend
//// sortType = 1 : Descend
//template <class T>
//void indexed_sort( std::vector<T> & unsorted, std::vector<T> & sorted, std::vector<int> & index_map, int sortType=0)
//{
//	// Original unsorted index map
//	index_map.resize(unsorted.size());
//	for(size_t i=0;i<unsorted.size();i++)
//	{
//		index_map[i] = i;
//	}
//
//	if (sortType==0)
//	{
//	// Sort the index map, using unsorted for comparison
//	sort(
//		index_map.begin(), 
//		index_map.end(), 
//		index_cmp<std::vector<T>& >(unsorted));
//	}
//	else
//	{
//		sort(
//		index_map.begin(), 
//		index_map.end(), 
//		index_cmp_descend<std::vector<T>& >(unsorted));
//	}
//
//	sorted.resize(unsorted.size());
//	reorder(unsorted,index_map,sorted);
//}
//
//// This implementation is O(n), but also uses O(n) extra memory
//template< class T >
//void reorder( std::vector<T> & unordered,  std::vector<int> const & index_map,  std::vector<T> & ordered)
//{
//	// copy for the reorder according to index_map, because unsorted may also be
//	// sorted
//	std::vector<T> copy = unordered;
//	ordered.resize(index_map.size());
//	for(unsigned int i = 0; i<index_map.size();i++)
//	{
//		ordered[i] = copy[index_map[i]];
//	}
//}

#endif