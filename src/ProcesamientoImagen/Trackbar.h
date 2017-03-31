/*
 * trackbar.h
 *
 *  Created on: 24/2/2016
 *      Author: edurne
 */

#ifndef PROCESAMIENTOIMAGEN_TRACKBAR_H_
#define PROCESAMIENTOIMAGEN_TRACKBAR_H_

class Trackbar {
private:
	int umbral;
	int tipo;
	int erosion;
	int dilatacion;
public:
	Trackbar();

	int getDilatacion();
	void setDilatacion(int dilatacion);
	int getErosion();
	void setErosion(int erosion);
	int getTipo();
	void setTipo(int tipo);
	int getUmbral();
	void setUmbral(int umbral);

	void crearTrackbar();

	 ~Trackbar();
};

#endif /* PROCESAMIENTOIMAGEN_TRACKBAR_H_ */
