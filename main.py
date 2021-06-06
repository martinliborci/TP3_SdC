#! /usr/bin/python3
# sudo apt-get install python3-matplotlib
# pip3 install pynput

from pynput import keyboard as kb
from pylive import live_plotter
import numpy as np

def detectarTecla(tecla):
    print('Se ha pulsado la tecla ' + str(tecla))
    f = open("/proc/miCatangaF1Proc", "w")
    f.write("a")
    f.close()


title = "TP3: Liborci - Reyes\nJuan Manuel - The Best Driver"
xlabel = "Tiempo"
ylabel = "Señal"
sampleTime = 1 # en segundos
size = 100  # cantidad de muestras
x_vec = np.linspace(0,1,size+1)[0:-1]  # espaciadoMuestras, stop=cantidadMuestras
y_vec = [0] * len(x_vec)
line1 = []  # almacenar los arcos entre puntos

escuchador = kb.Listener(detectarTecla)
escuchador.start()

while True:
    f = open("/proc/miCatangaF1Proc", "r")
    stringLeido = f.read()
    f.close()
    print(stringLeido)
    valorLeido = int(stringLeido,2)
    print(valorLeido)
    y_vec[-1] = valorLeido
    line1 = live_plotter(x_vec,y_vec,line1,title,xlabel,ylabel,sampleTime)
    y_vec = np.append(y_vec[1:],0.0)


