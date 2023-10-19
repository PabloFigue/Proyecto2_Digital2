//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//
//This example creates a bridge between Serial and Classical Bluetooth (SPP)
//and also demonstrate that SerialBT have the same functionalities of a normal Serial

#include "BluetoothSerial.h"

//Pines de salida para activar el juego
#define arriba 23
#define abajo 22
#define derecha 19
#define izquierda 18


//#define USE_PIN // Uncomment this to use PIN during pairing. The pin is specified on the line below
const char *pin = "1234"; // Change this to more secure PIN.

String device_name = "ESP32-BT-Slave";

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;

void setup() {
  pinMode(arriba, OUTPUT);
  pinMode(abajo, OUTPUT);
  pinMode(izquierda, OUTPUT);
  pinMode(derecha, OUTPUT);
  Serial.begin(115200);
  SerialBT.begin(device_name); //Bluetooth device name
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());
  //Serial.printf("The device with name \"%s\" and MAC address %s is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str(), SerialBT.getMacString()); // Use this after the MAC method is implemented
  #ifdef USE_PIN
    SerialBT.setPin(pin);
    Serial.println("Using PIN");
  #endif
}

void loop() {
if (SerialBT.available()) {
    char data = SerialBT.read();
    int opcion = atoi(&data);
    Serial.println(opcion);
    if (opcion == 1){ //arriba
      digitalWrite(arriba, HIGH);
    }else if (opcion == 2){ //abajo
      digitalWrite(abajo, HIGH);
    }else if (opcion == 3){ //derecha
      digitalWrite(derecha, HIGH);
    }else if (opcion == 4){ //izquierda
      digitalWrite(izquierda, HIGH);
    }
  }
  //delay(100);
      digitalWrite(arriba, LOW);
      digitalWrite(abajo, LOW);
      digitalWrite(derecha, LOW);
      digitalWrite(izquierda, LOW);
}
