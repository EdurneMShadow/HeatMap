#include <core/core.hpp>
#include <core/mat.hpp>
#include <core/operations.hpp>
#include <highgui/highgui.hpp>
#include <highgui/highgui_c.h>
#include <time.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "../BaseDeDatos/DB.h"
#include "../FicherosConfiguracion/json/json.h"
#include "../ProcesamientoImagen/Deteccion.h"
#include "../ProcesamientoImagen/Trackbar.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
	ifstream in("config.json");
	Json::Value root;   // 'root' will contain the root value after parsing.
	in >> root;

	bool isVideo = root.get("isVideo", "true" ).asBool();
	funciones funcion;
	Deteccion det;
	Trackbar trackbar;
	DB data("heatmap", "coordenadas");


	Mat fondoprev;
	Mat captura;
	time_t timer;
	time_t timer2;
	Mat fondoMostrar;
	Mat imagenConMapa;
	Mat mapaResultado;
	struct tm *localTime;

	time(&timer);
	localTime = localtime(&timer);
	int actualSeconds = localTime->tm_sec;
	int seconds = 0;
	bool coger = true;

//===================ABRIR VIDEO========================
	VideoCapture camara;
	std::string source = root.get("source", "moscu.mp4" ).asString();
	if (isVideo) {
		camara.open(source);
	}
	else {
		camara.open(source);
		//		"http://edurne:edurne@158.49.245.71:27654/mjpg/video.mjpg");
		if (!camara.isOpened()) {
			cout << "No ha sido posible abrir la cámara" << endl;
			return -1;
		}
		else{
				cout<<"camara abierta"<<endl;
			}
	}

	trackbar.crearTrackbar();
	camara >> fondoprev;
	fondoprev.copyTo(fondoMostrar);

	det.setFondoprev(fondoprev);
	det.setW(fondoprev.cols);
	det.setH(fondoprev.rows);
	std::vector<unsigned char> image(det.getH() * det.getW() * 4);

//=============INICIALIZACIÓN DEL MAPA DE CALOR============
	bool tReal=root.get("mapRTime", false).asBool();
	if(tReal){
		det.hm = heatmap_new(det.getW(), det.getH());
		cout<<"SIIIIIIIIIIIIIIIIII, TIEMPO REAL!!!"<<cout;
	}
	else{
		funcion.inicializarMapa(det.getH(), det.getW());
		cout<<"sin tiempo real"<<endl;
	}

//=============BUCLE PARA COGER CADA FRAME============
	bool primero=true;
	while (true) {
		time(&timer2);
		localTime = localtime(&timer2);
		seconds = localTime->tm_sec;

		if (coger) {
			camara >> captura;
			if (!captura.data) {
				if(!tReal) {
					funcion.pintarYguardarMapa(data, image, det.getW(), det.getH());
					det.guardarImagen(mapaResultado, "mapaResultado");
				}
				data.desconectarBD();
				cout << "nada" << endl;
				return 0;
			}

			det.setUmbral(trackbar.getUmbral());
			det.setErosion(trackbar.getErosion());
			det.setDilatacion(trackbar.getDilatacion());
			det.setTipo(trackbar.getTipo());
			bool pintarDesdeBD = root.get("drawMapFromBD", false).asBool();
			if (!pintarDesdeBD) {
				//para analizar un único frame por segundo
				if (seconds - actualSeconds == 1
						|| seconds - actualSeconds < 0) {
					cout << "procesando" << endl;
					actualSeconds = seconds;
					time(&timer2);
					det.detectar(captura, data, primero, image);
					if (primero) {
						primero = false;
					}
					//cargar la imagen generada en png a un mat y sumarla con el fondo para poder mostrarla.
					imagenConMapa = imread("heatmap.png", CV_LOAD_IMAGE_COLOR);

					addWeighted(fondoMostrar, 0.4, imagenConMapa, 0.6, 0.0,
							mapaResultado);
					imshow("mapa", mapaResultado);
					det.guardarImagen(mapaResultado, "mapaResultado");
				} else {
					cout << "no ha pasado un segundo" << endl;
				}
			}
			else{
				cout<<"entrando"<<endl;
				string fechaInicio = root.get("initDate", " ").asString();
				string fechaFin = root.get("endDate", " ").asString();
				string horaInicio = root.get("initHour", " ").asString();
				string horaFin = root.get("endHour", " ").asString();
				funcion.pintarYguardarMapaDB(data, image, det.getW(), det.getH(), fechaInicio, fechaFin, horaInicio, horaFin);
				break;
			}
			char c = cvWaitKey(33);			//al pulsar esc se sale del programa
			if (c == 27) {
				break;
			}
			coger = false;
			captura.release();
		} else
			coger = true;
	}


//	data.eliminarTabla();
	in.close();
	data.desconectarBD();
	//===============LIBERACIÓN DE MEMORIA=================
	camara.release();
	fondoprev.release();
	captura.release();
	fondoMostrar.release();
	imagenConMapa.release();
	mapaResultado.release();
	cvDestroyAllWindows();
	return 0;
}
