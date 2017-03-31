/*
 * blob.cpp
 *
 *  Created on: 6/3/2015
 *      Author: edurne
 */

#include "blob.h"

#include <core/core_c.h>
#include <core/operations.hpp>
#include <highgui/highgui_c.h>
#include <imgproc/imgproc_c.h>
#include <imgproc/types_c.h>
#include <list>
#include <map>
#include <utility>

blob::blob() {
	// TODO Auto-generated constructor stub

}

blob::~blob() {
	// TODO Auto-generated destructor stub
}
void blob::detectarObjetos(const Mat & imagen,vector<CvPoint2D64f> &coordenadas) {
	cout<<"detectando objetos"<<endl;
	CvBlobs blobs;
	IplImage img = imagen;
	IplImage grey = imagen;
	cvThreshold(&grey, &grey, 100, 255, CV_THRESH_BINARY);
	IplImage *labelImg = cvCreateImage(Size(imagen.cols, imagen.rows),IPL_DEPTH_LABEL, 1);
	unsigned int result = cvLabel(&grey, labelImg, blobs);
	IplImage *imgOut = cvCreateImage(Size(imagen.cols, imagen.rows),
			IPL_DEPTH_8U, 3);
	cvZero(imgOut);

	cvRenderBlobs(labelImg, blobs, &img, imgOut);
	CvPoint2D64f c;
	//cvFilterByArea(blobs, 0, 2000);
	int cont=0;
	for (CvBlobs::const_iterator it = blobs.begin(); it != blobs.end(); ++it) {
		cont++;
		double moment10 = it->second->m10;
		double moment01 = it->second->m01;
		double area = it->second->area;
		//cout<<"area "<<area<<endl;

		//Variable for holding position
		int x1;
		int y1;
		//Calculating the current position
		//	  if(area>394 && area<600 ){
		x1 = moment10 / area;
		y1 = moment01 / area;

		//Mapping to the screen coordinates
//		  int x=(int)(x1*1716/300);
//		  int y=(int)(y1*901/250);
		//Printing the position information
		c.x = x1;
		c.y = y1;
		coordenadas.push_back(c);

		CvContourPolygon *polygon = cvConvertChainCodesToPolygon(&(*it).second->contour);

		CvContourPolygon *sPolygon = cvSimplifyPolygon(polygon, 10.);
		CvContourPolygon *cPolygon = cvPolygonContourConvexHull(sPolygon);

		cvRenderContourChainCode(&(*it).second->contour, imgOut);
		cvRenderContourPolygon(sPolygon, imgOut, CV_RGB(0, 0, 255));
		cvRenderContourPolygon(cPolygon, imgOut, CV_RGB(0, 255, 0));

		delete cPolygon;
		delete sPolygon;
		delete polygon;

		// Render internal contours:
		for (CvContoursChainCode::const_iterator jt =(*it).second->internalContours.begin();
				jt != (*it).second->internalContours.end(); ++jt)
			cvRenderContourChainCode((*jt), imgOut);
	}
	cout<<"numero de coordenadas: "<<cont<<endl;
//	cvShowImage("Blob", imgOut);
	cvReleaseImage(&imgOut);
	cvReleaseImage(&labelImg);

	cvReleaseBlobs(blobs);

}
