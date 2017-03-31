#include "Deteccion.h"

#include <core/core_c.h>
#include <core/mat.hpp>
#include <core/operations.hpp>
#include <core/types_c.h>
#include <highgui/highgui.hpp>
#include <imgproc/imgproc.hpp>
#include <imgproc/types_c.h>
#include <iostream>
#include <sstream>
#include <vector>

#include "../BaseDeDatos/DB.h"
#include "../MapaCalor/lodepng.h"
#include "blob.h"

Deteccion::Deteccion() {

}
int Deteccion::getH() {
	return h;
}
int Deteccion::getW() {
	return w;
}
void Deteccion::setH(int h){
	this->h=h;
}
void Deteccion::setW(int w){
	this->w=w;
}
void Deteccion::setDilatacion(int dilatacion) {
	this->dilatacion = dilatacion;
}

void Deteccion::setErosion(int erosion) {
	this->erosion = erosion;
}

void Deteccion::setUmbral(int umbral) {
	this->umbral = umbral;
}

void Deteccion::setTipo(int tipo) {
	this->tipo = tipo;
}

void Deteccion::setFondoprev(const Mat& nuevoFondo) {
	this->fondoprev = nuevoFondo;
}
void Deteccion::guardarImagen(const Mat & imagen, string nombre) {
	string fullPath;
	stringstream ss;
	ss << "./capturas/" << nombre << ".jpg";
	fullPath = ss.str();
	imwrite(fullPath, imagen);
}

void Deteccion::detectar(Mat frameMat, DB data, bool primero, std::vector<unsigned char >image){
	blob blobs;
	Mat imgroi, resta, gris, umbralizada, corregida;
	Mat fondo;
	Point coord;
	vector<CvPoint2D64f> p;
	imshow("original", frameMat);
//	imshow("fondo", fondoprev);
	absdiff(frameMat, fondoprev, resta);//resta de fondo (como es una resta absoluta da igual lo que se reste con qué)
//	imshow("resta", resta);
	cvtColor(resta, gris, CV_BGR2GRAY);	//pasar la imagen a escala de grises (bgr to gray)
//	imshow("gris", gris);
	threshold(gris, umbralizada, umbral, 255, tipo);//umbralizar la imagen
//	imshow("umbralizada", umbralizada);
	erode(umbralizada, corregida,getStructuringElement(MORPH_RECT,Size(erosion, erosion)));
	dilate(corregida, corregida, getStructuringElement(MORPH_RECT,Size(dilatacion, dilatacion)));
//	imshow("corregida", corregida);
	if(!primero){
		blobs.detectarObjetos(corregida, p);
		//cout<<"se añaden: "<<p.size()<<" puntos"<<endl;
		for (int i = 0; i < p.size(); i++) {
			coord.x = p[i].x;
			coord.y = p[i].y;
			if (coord.x < w && coord.y< h && coord.x>0 && coord.y>0) {
			//	cout<<"insertado: "<<coord.x<<" "<<coord.y<<endl;
				heatmap_add_point_with_stamp(hm, coord.x, coord.y,heatmap_stamp_gen_nonlinear(10, [](float d){return d*d;}));
				data.insertar(coord.x, coord.y);
			}
		}
	}
		heatmap_render_default_to(hm, &image[0]);
		if (unsigned error = lodepng::encode("heatmap.png", image, w, h)) {
			std::cerr << "encoder error " << error << ": "
					<< lodepng_error_text(error) << std::endl;
		}

	frameMat.copyTo(fondoprev);

	frameMat.release();
	imgroi.release();
	gris.release();
	resta.release();
	umbralizada.release();
	corregida.release();
	p.clear();
}

Deteccion::~Deteccion() {
	fondoprev.release();
}
