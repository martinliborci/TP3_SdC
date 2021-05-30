#! /usr/bin/python3

# sudo apt-get install python3-matplotlib
# https://makersportal.com/blog/2018/8/14/real-time-graphing-in-python

from pylive import live_plotter
import numpy as np

title = "TP3: Liborci - Reyes\nJuan Manuel - The Best Driver"
xlabel = "Tiempo [seg]"
ylabel = "Temperatura [C]"
sampleTime = 1
size = 100  # cantidad de muestras
x_vec = np.linspace(0,1,size+1)[0:-1]  # espaciadoMuestras, stop=cantidadMuestras
y_vec = np.random.randn(len(x_vec))    # generar 100 muestras aleatorias con distribución normal
line1 = []
while True:
    rand_val = np.random.randn(1)
    y_vec[-1] = rand_val
    line1 = live_plotter(x_vec,y_vec,line1,title,xlabel,ylabel,sampleTime)
    y_vec = np.append(y_vec[1:],0.0)



