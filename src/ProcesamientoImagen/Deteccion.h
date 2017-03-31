#ifndef DETECCION_H_
#define DETECCION_H_

#include <core/core.hpp>
#include <time.h>
#include <string>


#include "Trackbar.h"
#include "../MapaCalor/FuncionesMapa.h"

class DB;
using namespace cv;
using namespace std;

class Deteccion{

private:
	Mat fondoprev;
	int h;
	int w;
	Trackbar trackbar;
	int umbral, erosion, dilatacion, tipo;
public:
	heatmap_t* hm;
	Deteccion();
	int getH();
	void setH(int h);
	int getW();
	void setW(int w);
	void setFondoprev(const Mat& fondo);
	void setDilatacion(int dilatacion);
	void setErosion(int erosion);
	void setUmbral(int umbral);
	void setTipo(int tipo);

	void detectar(Mat frameMat, DB data,bool primero, std::vector<unsigned char >image);
	void guardarImagen(const Mat & imagen, string nombre);

	~Deteccion();

};
#endif
