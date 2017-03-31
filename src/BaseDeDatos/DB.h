/*
 * db.h
 *
 *  Created on: 10/2/2016
 *      Author: edurne
 */
#ifndef DB_H_
#define DB_H_

#include <mysql.h>
#include <string>

#include "../Utiles/linkedList.h"

using namespace std;

class DB{
private:
	MYSQL *myData;
public:
	DB(std::string bd, std::string tabla);
	void conectarBD(string bd);
	void desconectarBD();
	void comprobarConexion();
	void crearBD(char* db);
	void crearTabla(char* db, char*tabla);
	void insertar(int x, int y);
	void mostrar();
	linkedList consultar();
	linkedList consultarRangoFechas(string fechaInicio, string fechaFin);
	linkedList consultarRangoFechasYHoras(string fechaInicio, string fechaFin, string horaInicio, string horaFin);
	void eliminarTabla();
	void eliminarBD();
	~DB();
	};




#endif /* DB_H_ */
