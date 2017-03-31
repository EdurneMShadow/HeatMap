/*
 * blob.h
 *
 *  Created on: 6/3/2015
 *      Author: edurne
 */

#ifndef BLOB_H_
#define BLOB_H_

#include <core/core.hpp>
#include <core/types_c.h>
#include <vector>
#include <cvblob.h>

using namespace cv;
using namespace std;
using namespace cvb;

class blob {
public:
	blob();
	void detectarObjetos(const Mat & imagen, vector<CvPoint2D64f> &coordenadas);
	~blob();
};

#endif /* BLOB_H_ */
