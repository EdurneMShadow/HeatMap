# HeatMap
Para hacer funcionar el programa, hace falta tener instalado el IDE Eclipse, disponible desde su página web https://eclipse.org/. También es necesario instalar OpenCV
2.4.9 y MySQL. Una vez que se cumplen esos requisitos, solamente hay que importar a eclipse el proyecto que se ha facilitado, compilarlo y ejecutarlo.
## Configurar el programa
Dentro de la carpeta del proyecto hay un fichero llamado config.json. Es el fichero de configuración del programa. Su contenido es el que sigue:<br>
```
{"isVideo " : true,
"source " : " rusia.mp4",
"mapRTime " : true,
"drawMapFromBD" : false,
"initDate ":"16-05-18",
"endDate ":"20-05-18",
"initHour" : " 12:00:00",
"endHour" : " 12:20:00"
}
```
isVideo es un parámetro que puede tomar los valores true o false. Si tiene el valor true significa que el segundo parámetro (source) contiene el
nombre de un vídeo, mientras que si tiene el valor false tiene una dirección IP de una cámara.<br>

mapRTime también es un parámetro que puede tomar los valores true o false y que indica si el mapa se quiere ver en tiempo real o no respectivamente. Ver la generación del mapa en tiempo real hace referencia
a ver cómo se va pintando el mapa sobre la imagen que se estaba obteniendo en ese momento, bien sea del flujo de vídeo de un archivo o de una cámara
IP.<br>

En el caso de que mapRTime tenga el valor true, el parámetro drawMapFromBD, que indica que se quiere crear un mapa a partir de la información almacenada en la base de datos, tiene que estar obligatoriamente a false.
En caso de que se quiera obtener un mapa de calor de un momento determinado del tiempo hay que seguir los siguientes pasos:<br>

1. Poner el parámetro mapRTime a false.
2. Poner el parámetro drawMapFromBD a true.
3. Indicar un periodo de tiempo. En initDate iría la fecha de comienzo, en endDate la fecha de fin, en initHour la hora de comienzo y en endHour la hora de fin. <br>

Hay dos posibilidades a la hora de indicar un periodo de tiempo. El primero es no indicar un rango de horas concretas e introducir solo las fechas. En este caso, el programa entendería que se desea conocer toda la
información sobre las 24 horas de los días indicados. Sin embargo, si se introducen las horas, además de las fechas, se obtendrá un mapa de calor de ese rango de fechas de cada uno de los días.

##Ejemplos
