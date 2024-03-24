## Wiring hardware / Collegamento hardware:
![Wiring hardware](arduino.png)

## Hardware list / Lista dell'hardware
* Servomotore a rotazione continua / Continuous Rotation Servo Motor
* Condensatore ceramico da 100 nF / 100 nF ceramic capacitor
* Condensatore elettrolitico da 1 uF / 1 uF electrolytic capacitor
* Regolatore di tensione LDO a 5 V / 5V LDO voltage regulator
* Adattatore di alimentazione DC / DC Power Adapter [(like this one)](https://www.digikey.com/en/products/detail/368/1528-1386-ND/5629434).
* Adattatore di alimentazione a parete da 12 V 1A / 12V Wall Power Adapter 
* Arduino UNO (r3 o r4)

## Arduino Pins
* GNU Servo -> GNU Arduino; 
* Control PIN Servo -> Digital PIN #9 Arduino .

## The project / Il progetto
Il dispositivo fa parte dell'installazione artistica "Pneumos", di Oriana Persico e installata nella città di Ravenna. Il dispositivo riceve tramite comunicazione seriale i dati sulla qualità dell'aria di Ravenna, e muove una "sacca polmonare aliena" con un ritmo in linea con la qualità dell'aria. Il software che invia dati ad Arduino, oltre a comandare un'installazione sonora analogica, è disponibile su [(questo link)](https://github.com/Blumealc/PNEUMOS_SW/tree/main/python).

The device is part of the art installation "Pneumos," by Oriana Persico and installed in the city of Ravenna. The device receives via serial communication data on Ravenna's air quality, and moves an "alien lung bag" at a pace in line with air quality. The software that sends data to Arduino, as well as controls an analog sound installation, is available at [(this link)](https://github.com/Blumealc/PNEUMOS_SW/tree/main/python)