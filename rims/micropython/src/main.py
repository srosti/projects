import time
from max31865 import MAX31865

rtd = MAX31865()
while True:
    temp = rtd.read()
    print("Temperatur: ", temp)
    time.sleep(5)