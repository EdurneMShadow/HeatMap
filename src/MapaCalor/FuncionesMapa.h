/*
 * funciones.h
 *
 *  Created on: 22/2/2016
 *      Author: edurne
 */

#ifndef MAPACALOR_FUNCIONESMAPA_H_
#define MAPACALOR_FUNCIONESMAPA_H_

#include <vector>
#include "heatmap.h"
#include "../BaseDeDatos/DB.h"

using namespace std;

class funciones {
	heatmap_t* hm;
public:
	funciones();
	void inicializarMapa(int h, int w);
	void pintarYguardarMapaDB(DB data, std::vector<unsigned char >image,int w, int h,string fechaIni, string fechaFin, string horaIni, string horaFin);
	void pintarYguardarMapa(DB data, std::vector<unsigned char >image,int w, int h);
	virtual ~funciones();
};

#endif /* MAPACALOR_FUNCIONESMAPA_H_ */
