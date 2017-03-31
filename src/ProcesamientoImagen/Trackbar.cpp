/*
 * Trackbar.cpp
 *
 *  Created on: 24/2/2016
 *      Author: edurne
 */

#include <highgui/highgui.hpp>
#include <highgui/highgui_c.h>
#include "Trackbar.h"

using namespace cv;

Trackbar::Trackbar() {
	umbral = 50;
	tipo = 0;
	erosion = 6;
	dilatacion = 1;

}

void Trackbar::crearTrackbar() {
	namedWindow("Controlador", CV_WINDOW_AUTOSIZE);
	createTrackbar("umbral", "Controlador", &umbral, 255); //creacion del trackbar
	createTrackbar("tipo", "Controlador", &tipo, 4);
	createTrackbar("erosión", "Controlador", &erosion, 200);
	createTrackbar("dilatación", "Controlador", &dilatacion, 200);
}

int Trackbar::getDilatacion() {
	return dilatacion;
}

void Trackbar::setDilatacion(int dilatacion) {
	this->dilatacion = dilatacion;
}

int Trackbar::getErosion() {
	return erosion;
}

void Trackbar::setErosion(int erosion) {
	this->erosion = erosion;
}

int Trackbar::getTipo(){
	return tipo;
}

void Trackbar::setTipo(int tipo) {
	this->tipo = tipo;
}

int Trackbar::getUmbral(){
	return umbral;
}

void Trackbar::setUmbral(int umbral) {
	this->umbral = umbral;
}

Trackbar::~Trackbar() {
	// TODO Auto-generated destructor stub
}

