/*
 * funciones.cpp
 *
 *  Created on: 22/2/2016
 *      Author: edurne
 */

#include <stddef.h>
#include <iostream>

#include "../Utiles/linkedList.h"
#include "FuncionesMapa.h"
#include "lodepng.h"

funciones::funciones() {
	hm = NULL;
}
void funciones::inicializarMapa(int h, int w) {
	hm = heatmap_new(w, h);

}
void funciones::pintarYguardarMapaDB(DB data, std::vector<unsigned char> image,int w, int h, string fechaIni, string fechaFin, string horaIni, string horaFin) {
	linkedList listaRecuperada;
	int x, y;
	if (horaIni == "") {
		listaRecuperada = data.consultarRangoFechas(fechaIni, fechaFin);
	} else {
		listaRecuperada = data.consultarRangoFechasYHoras(fechaIni, fechaFin, horaIni, horaFin);
	}

	while (!listaRecuperada.vacia()) {
	//	cout<<"recuperando!"<<endl;
		x = listaRecuperada.devolverHead(y);
		heatmap_add_point_with_stamp(hm, x, y,heatmap_stamp_gen_nonlinear(10, [](float d){return d*d;}));
	}
	heatmap_render_default_to(hm, &image[0]);
	if (unsigned error = lodepng::encode("heatmap.png", image, w, h)) {
		cout<<"if error"<<endl;
		std::cerr << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
	}
}
void funciones::pintarYguardarMapa(DB data, std::vector<unsigned char> image,int w, int h) {
	linkedList listaRecuperada;
	int x, y;
	listaRecuperada=data.consultar();
	while (!listaRecuperada.vacia()) {
		//	cout<<"recuperando!"<<endl;
		x = listaRecuperada.devolverHead(y);
		heatmap_add_point_with_stamp(hm, x, y,heatmap_stamp_gen_nonlinear(10, [](float d){return d*d;}));
	}
	heatmap_render_default_to(hm, &image[0]);
	if (unsigned error = lodepng::encode("heatmap.png", image, w, h)) {
		cout << "if error" << endl;
		std::cerr << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
	}
}
funciones::~funciones() {
	heatmap_free(hm);
}

