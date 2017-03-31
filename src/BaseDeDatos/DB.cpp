/*
 * db.cpp
 *
 *  Created on: 10/2/2016
 *      Author: edurne
 */

#include "DB.h"

#include <mysql_version.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
using namespace std;

DB::DB(std::string bd, std::string tabla) {
	conectarBD(bd);

	char *nombreBD = new char[bd.length() + 1];
	std::strcpy(nombreBD, bd.c_str());
	char *nombreTabla = new char[tabla.length() + 1];
	std::strcpy(nombreTabla, tabla.c_str());

	crearBD(nombreBD);
	crearTabla(nombreBD, nombreTabla);
}

void DB::conectarBD(string bd) {
	if (!(myData = mysql_init(0))) {
		cout << "No se puede crear el objeto myData" << endl;
	} else {
		cout << "CASI CONECTADO!!" << endl;
	}

	if (!mysql_real_connect(myData, NULL, "root", bd.c_str(), NULL, MYSQL_PORT,
	NULL, 0)) {
		cout << "Imposible conectar con servidor mysql en el puerto "
				<< MYSQL_PORT << endl;
		mysql_close(myData);
	} else {
		cout << "CONECTADO!!" << endl;
	}
}
void DB::desconectarBD() {
	mysql_close(myData);
	cout << "DESCONEXIÓN!!!!" << endl;
}

void DB::comprobarConexion() {
	if (mysql_ping(myData)) {
		cout << "Error: conexión imposible" << endl;
		mysql_close(myData);
	}
}

void DB::crearBD(char* db) {
	//Primero comprobar si existe una base de datos con ese nombre
	char *consulta;
	char *plantilla = "SHOW DATABASES LIKE \'%s\'";
	MYSQL_RES *res;
	consulta = new char[strlen(db) + strlen(plantilla)];
	sprintf(consulta, plantilla, db);

	if (!mysql_query(myData, consulta)) {
		if ((res = mysql_store_result(myData))) {
			// Procesar resultados
			if (!mysql_num_rows(res)) {
				//la base de datos no existe, hay que crearla.
				mysql_query(myData, "CREATE DATABASE heatmap");
				cout << "se ha creado la bd" << endl;
			} else {
				cout << "La base de datos ya existe" << endl;
			}
			// Liberar el resultado de la consulta:
			mysql_free_result(res);
		}
	}
	delete[] consulta;
}

void DB::crearTabla(char* db, char*tabla) {
	//Primero se comprueba si la tabla ya existe.
	char *consulta;
	char *plantilla = "SHOW TABLES FROM %s LIKE \'%s\'";
	MYSQL_RES *res;
	bool valorret = true;

	consulta = new char[strlen(db) + strlen(tabla) + strlen(plantilla) - 1];
	sprintf(consulta, plantilla, db, tabla);

	if (!mysql_query(myData, consulta)) {
		if ((res = mysql_store_result(myData))) {
			// Procesar resultados
			if (!mysql_num_rows(res)) { //si no existe la tabla, se crea
				mysql_query(myData,
						"CREATE TABLE heatmap.coordenadas (id INT NOT NULL AUTO_INCREMENT,x INT, y INT, time TIMESTAMP, PRIMARY KEY (id)) ENGINE=InnoDB");
				cout << "tabla creada" << endl;
			} else {
				cout << "La tabla ya existe y tiene datos" << endl;
			}
			// Liberar el resultado de la consulta:
			mysql_free_result(res);
		}
	}
	delete[] consulta;
}
string NumberToString(int Number) {
	stringstream ss;
	ss << Number;
	return ss.str();
}
void DB::insertar(int x, int y) {
	string consulta;
	string xConvertida = NumberToString(x);
	string yConvertida = NumberToString(y);

	consulta = "INSERT INTO heatmap.coordenadas (x,y) VALUES (" + xConvertida
			+ "," + yConvertida + ")";
//	sprintf(consulta,  (%d, %d);", x, y);
//	cout<<"consulta despues: "<<consulta<<endl;
	mysql_query(myData, consulta.c_str());
	//cout<<"insertado!!"<<endl;
}

void DB::mostrar() {
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	cout << "RESULTADOS" << endl;
	if (mysql_query(myData, "SELECT * FROM heatmap.coordenadas") == 0) {
		resultado = mysql_use_result(myData);
		while (row = mysql_fetch_row(resultado)) {
			printf("%s,%s\n", row[1], row[2]);
		}
	}
}

linkedList DB::consultar() {
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	linkedList listaResultadoConsulta;
	if (mysql_query(myData, "SELECT * FROM heatmap.coordenadas") == 0) {
		resultado = mysql_store_result(myData);
		while (row = mysql_fetch_row(resultado)) {
			//printf("%s,%s\n", atoi(row[1]), atoi(row[2]));
			listaResultadoConsulta.addValue(atoi(row[1]), atoi(row[2]));
		}
	}
	return listaResultadoConsulta;
}

linkedList DB::consultarRangoFechas(string fechaInicio, string fechaFin) {
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	linkedList listaResultadoConsulta;
	std::string consulta= "SELECT * FROM heatmap.coordenadas "
			"WHERE CAST(time as DATE)>='"+fechaInicio+"' AND CAST(time as DATE)<='"+fechaFin+"';";
	char *cstr = new char[consulta.length() + 1];
		strcpy(cstr, consulta.c_str());
	if (mysql_query(myData, cstr)==0){
		resultado = mysql_store_result(myData);
		if(!resultado){
			cout<<"No hay datos"<<endl;
		}
		while (row = mysql_fetch_row(resultado)) {
			listaResultadoConsulta.addValue(atoi(row[1]), atoi(row[2]));
		}
	//	cout<<"Resultados recuperados: "<<cont<<endl;
	}
	delete [] cstr;
	return listaResultadoConsulta;
}
linkedList DB::consultarRangoFechasYHoras(string fechaInicio, string fechaFin, string horaInicio, string horaFin) {
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	linkedList listaResultadoConsulta;
	std::string consulta= "SELECT * FROM heatmap.coordenadas "
			"WHERE CAST(time as DATE)>='"+fechaInicio+"' AND CAST(time as DATE)<='"+fechaFin+"' "
					"AND CAST(time as TIME)>='"+horaInicio+"' AND CAST(time as TIME)<='"+horaFin+"';";
	cout<<"consulta: "<<consulta<<endl;
	char *cstr = new char[consulta.length() + 1];
		strcpy(cstr, consulta.c_str());
	if (mysql_query(myData, cstr)==0){
		resultado = mysql_store_result(myData);
		if(!resultado){
			cout<<"No hay datos"<<endl;
		}
		while (row = mysql_fetch_row(resultado)) {
			listaResultadoConsulta.addValue(atoi(row[1]), atoi(row[2]));
		}
	//	cout<<"Resultados recuperados: "<<cont<<endl;
	}
	delete [] cstr;
	return listaResultadoConsulta;
}

void DB::eliminarTabla() {
	mysql_query(myData, "DROP TABLE heatmap.coordenadas");
	cout << "tabla eliminada" << endl;
}

void DB::eliminarBD() {
	mysql_query(myData, "DROP DATABASE heatmap");
	cout << "base de datos eliminada" << endl;
}

DB::~DB(){

}
