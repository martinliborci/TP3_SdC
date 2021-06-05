#! /usr/bin/python3
# https://docs.python.org/3/tutorial/inputoutput.html

# sudo apt-get install python3-matplotlib
# https://makersportal.com/blog/2018/8/14/real-time-graphing-in-python

from pylive import live_plotter
import numpy as np

title = "TP3: Liborci - Reyes\nJuan Manuel - The Best Driver"
xlabel = "Tiempo [seg]"
ylabel = "Temperatura [C]"
sampleTime = 0.1
size = 100  # cantidad de muestras
x_vec = np.linspace(0,1,size+1)[0:-1]  # espaciadoMuestras, stop=cantidadMuestras
#y_vec = np.random.randn(len(x_vec))    # generar 100 muestras aleatorias con distribución normal
y_vec = [0] * len(x_vec)
line1 = []

# r:Read
# b:Modo Binario
# 0: No utilizar buffer para leer los datos en tiempo real



while True:
    #rand_val = np.random.randn(1)
    #rand_val = f.read()
    f = open("/proc/gpio_proc", "r")
    stringLeido = f.read()
    ##rand_val = int.from_bytes(f.read(), "little")  # lscpu | grep Endian
    f.close()
    print(stringLeido)
    valorLeido = int(stringLeido,2)
    print(valorLeido)
    y_vec[-1] = valorLeido
    line1 = live_plotter(x_vec,y_vec,line1,title,xlabel,ylabel,sampleTime)
    y_vec = np.append(y_vec[1:],0.0)