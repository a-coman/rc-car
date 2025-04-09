# Script que obtiene los inputs de un mando conectado al PC,
# y los transmite a una ESP8266 mediante conexion UDP

# Requerimientos:
# - Mando PS4 o equivalente
# - Pythone ver 3.10.9
# - pip install pygame

import os
import pygame
import socket

# Offset para evitar zonas muertas del joystick
OFFSET = 0.1

# Inicializacion
pygame.init()
os.system('cls')

# Configuramos la conexion UDP
client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
print("IP: ", end="")
IP = input()
print("PUERTO: ", end="") 
PORT = int(input())

# Obtenemos un array con todos los joysticks disponibles detectados por pygame
joysticks = [pygame.joystick.Joystick(x) for x in range(pygame.joystick.get_count())]

print("\nJoysticks disponibles: ")
print(joysticks)
print("\nCapturando Joysticks. Presione el boton de xbox para salir...")

leftTrigger = "0"       # Valores entre [0,20]
rightTrigger = "0"      # Valores entre [0,20]
leftJoystick = "0"      # Valores entre [-10...0...10] || IZQ: < 0 || DCH > 0 ||
upperLeftButton = "0"  # Valor 0 o 1
upperRightButton = "0" # Valor 0 o 1

while True:

    for event in pygame.event.get():

        if event.type == pygame.JOYBUTTONDOWN:
            if event.button == 0:
                pygame.joystick.quit()
                exit()

        if event.type == pygame.JOYAXISMOTION:
            if(event.axis == 5):
                value = int(round(event.value, 1) * 10 + 10)    # Multiplicamos y sumamos contantes para obtener un valor entero positivo
                leftTrigger = str(value)
            if(event.axis == 4):
                value = int(round(event.value, 1) * 10 + 10)
                rightTrigger = str(value)
            if(event.axis == 0):
                if(round(event.value,1) < -OFFSET or round(event.value,1) > OFFSET):
                    value = int(round(event.value, 1) * 10)
                    leftJoystick = str(value)
                else:
                    leftJoystick = "0"
        
        if event.type == pygame.JOYBUTTONDOWN:
            if event.button == 10:
                upperLeftButton = "1"
            elif event.button == 9:
                upperRightButton = "1"
                
        if event.type == pygame.JOYBUTTONUP:
            if event.button == 10:
                upperLeftButton = "0"
            elif event.button == 9:
                upperRightButton = "0"


    msg = leftTrigger + "," + rightTrigger + "," + leftJoystick + "," + upperLeftButton + "," + upperRightButton
    print(msg)

    # Enviamos paquetes cada 100 ms
    client.sendto(msg.encode(),(IP,PORT))
    pygame.time.wait(100)
