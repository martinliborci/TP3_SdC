import matplotlib.pyplot as plt
import numpy as np

plt.style.use('ggplot')  # Mostar los gráficos como en el lenguaje R

# x_vec: vector con las coordenas x de los puntos a graficar
# y1_data: vector con las coordenas y de los puntos a graficar
# line1: almacenar los arcos entre puntos
# title,xlabel,ylabel: título y leyendas de los ejes 
# pause_time: tiempo en que se grafican dos puntos consecutivos
def live_plotter(x_vec, y1_data, line1, title, xlabel, ylabel, pause_time=0.1):
    if line1==[]: # Si no está creado y configurado el gráfico, se lo hace
        plt.ion()  # Interactive ON: el gráfico se muestra inmediatamente
        fig = plt.figure(figsize=(13,6)) # figsize está en pulgadas
        ax = fig.add_subplot(111)   # Agrega los ejes
        line1, = ax.plot(x_vec,y1_data,'-o',alpha=0.8) # Crea la línea para actualizarla después
        plt.xlabel(xlabel)
        plt.ylabel(ylabel)                                                      
        plt.title(title)                                                        
        plt.show()
    
    line1.set_ydata(y1_data) # Se actualiza el valor de las muestras
    # Ajustar los límites si los nuevos datos van más allá de los mismos
    if np.min(y1_data)<=line1.axes.get_ylim()[0] or np.max(y1_data)>=line1.axes.get_ylim()[1]:
        plt.ylim([np.min(y1_data)-np.std(y1_data),np.max(y1_data)+np.std(y1_data)])

    plt.pause(pause_time)  # pausa para actualizar los datos
    
    return line1  # Se retorna la línea para poder actualizarla en la siguiente iteración