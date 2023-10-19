//***************************************************************************************************************************************
/* Librería para el uso de la pantalla ILI9341 en modo SPI PROYECTO2
   Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
   Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
   Con ayuda de: José Guerra
   IE3027: Electrónica Digital 2 - 2019
*/
//***************************************************************************************************************************************
#include <stdint.h>
#include <Energia.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include <SPI.h>
#include <SD.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"

//LED va a 3.3V
//El SPI es el 0
//MOSI va a PA_5
//MISO va a PA_4
//SCK va a PA_2
File myFile;

#define LCD_RST PD_0
#define LCD_DC PD_1
#define LCD_CS PA_3
#define SW1 PF_4
#define SW2 PF_0

//pin para la sd
#define scard PA_7

//pines para la musica
#define sound PC_6
#define TX2 PD_7

//Pines para los controles Ralph
#define Rizquierda PB_5
#define Rataque PE_1
#define Rderecha PA_6

//Pines para los controles Felix
#define Farriba PE_3
#define Fabajo PE_2
#define Fderecha PD_2
#define Fizquierda PD_3



//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset);

//PROTOTIPOS NUEVOS
void movfelix(int x, int y, int sprintedelay, unsigned char pos1[], unsigned char pos2[], unsigned char pos3[]);
void movralph(int x, int sprintedelay, unsigned char pos1[], unsigned char pos2[], unsigned char pos3[]);
void felixmenu(int sprintedelay);
void movselector(int y);
void reiniciovar(void);

//Prototipos Funciones Interrupcion Movimiento Jugadores.
void Felix_arriba(void);
void Felix_abajo(void);
void Felix_derecha(void);
void Felix_izquierda(void);
void Ralph_ataque(void);
void Ralph_derecha(void);
void Ralph_izquierda(void);
void conteoventana(void);

// NUEVAS VARIABLES

extern uint8_t inicio[];
//extern uint8_t fondo[];
extern uint8_t fondoinicio[];

extern uint8_t ralph1nvl3[];
extern uint8_t ralph2nvl3[];
extern uint8_t ralph3nvl3[];
extern uint8_t ralph4nvl3[];
extern uint8_t ralph5dnvl3[];
extern uint8_t ralph5invl3[];
extern uint8_t rstfondo3ralph[];

char conteofelix [19];
unsigned int nivel;
unsigned int disparo;
unsigned int posicionLadrilloX;
unsigned int posicionLadrilloY;
unsigned int vermax;
unsigned int vermin;
unsigned int bandera;

unsigned int posicionralphX;
unsigned int posicionralphY;

unsigned int posicionfelixX;
unsigned int posicionfelixY;
unsigned int orientationfelix;

unsigned int posicionSelector;

//INTERRUPCIONES DE SERVICIO

void Felix_izquierda(void) { //IzquierdaFelix
  if (nivel == 3) { //JUEGO
    orientationfelix = 0;
    if (posicionfelixX == 36) {
      movfelix(0, 0, 100, felix2i, felix3i, felix1i);
    }
    else {
      movfelix(-33, 0, 100, felix2i, felix3i, felix1i);
    }
    conteoventana();
  } else if (nivel == 2) { //MENU
    movfelixmenu(100);
  } else if (nivel == 1) { //INICIO
    nivel = 2;
    bandera = 0;
  } else if (nivel == 4) {
    nivel = 1;
    bandera = 1;
  } else if (nivel == 5) {
    nivel = 1;
    bandera = 1;
  }
}
void Felix_derecha(void) { //DerechaFelix
  if (nivel == 3) {     //JUEGO
    orientationfelix = 1;
    if (posicionfelixX == 168) {
      movfelix(0, 0, 100, felix2d, felix3d, felix1d);
    }
    else {
      movfelix(33, 0, 100, felix2d, felix3d, felix1d);
    }
    conteoventana();
  } else if (nivel == 2) { //MENU
    if (posicionSelector == 168) { //INICIAR
      nivel = 3;
      bandera = 0;
    } else if (posicionSelector == 198) { //MUTEAR
      FillRect(0, 0, 240, 320, 0x0000);
      Serial2.write('6');
      digitalWrite(sound, HIGH);
      digitalWrite(sound, LOW);
      nivel = 2;
      bandera = 0;
    } else if (posicionSelector == 228) { // CREDITOS
      Serial2.write('7');
      digitalWrite(sound, HIGH);
      digitalWrite(sound, LOW);
      FillRect(0, 0, 240, 320, 0x0000);
      if (!SD.begin(scard)) {
        Serial.println("initialization failed!");
        return;
      }
      Serial.println("initialization done.");
      myFile = SD.open("creditos.txt");
      if (myFile) {
        Serial.println("creditos.txt:");

        // read from the file until there's nothing else in it:
        int yc = 10;
        while (myFile.available()) {
          String datasd = myFile.readStringUntil(' ');
          Serial.println(datasd);
          LCD_Print(datasd, 50, yc, 1, 0xffff, 0x0000);
          delay(1500);
          yc = yc + 20;
          if (yc == 330){
            break;
          }
        }
        
        // close the file:
        myFile.close();
      } else {
        // if the file didn't open, print an error:
        Serial.println("error opening creditos.txt");
      }
      nivel = 2;
      bandera = 0;
    }
  } else if (nivel == 1) { //INICIO
    nivel = 2;
    bandera = 0;
  } else if (nivel == 4) {
    nivel = 1;
    bandera = 1;
  } else if (nivel == 5) {
    nivel = 1;
    bandera = 1;
  }
}
void Felix_abajo(void) { //AbajoFelix
  if (nivel == 3) { //JUEGO
    if (orientationfelix == 1) {
      if (posicionfelixY == 264) {
        movfelix(0, 0, 100, felix2d, felix3d, felix1d);
      }
      else {
        movfelix(0, 50, 100, felix2d, felix3d, felix1d);
      }
    } else {
      if (posicionfelixY == 264) {
        movfelix(0, 0, 100, felix2i, felix3i, felix1i);
      }
      else {
        movfelix(0, 50, 100, felix2i, felix3i, felix1i);
      }
    }
    conteoventana();
  } else if (nivel == 2) { //MENU
    if (posicionSelector == 228) {
      movselector(0);
    }
    else {
      movselector(30);
    }
    movfelixmenu(100);
  } else if (nivel == 1) { // INICIO
    nivel = 2;
    bandera = 0;
  } else if (nivel == 4) {
    nivel = 1;
    bandera = 1;
  } else if (nivel == 5) {
    nivel = 1;
    bandera = 1;
  }
}
void Felix_arriba(void) {  //ArribaFelix
  if (nivel == 3) { //JUEGO
    if (orientationfelix == 1) {
      if (posicionfelixY == 114) {
        movfelix(0, 0, 100, felix2d, felix3d, felix1d);
      }
      else {
        movfelix(0, -50, 100, felix2d, felix3d, felix1d);
      }
    } else {
      if (posicionfelixY == 114) {
        movfelix(0, 0, 100, felix2i, felix3i, felix1i);
      }
      else {
        movfelix(0, -50, 100, felix2i, felix3i, felix1i);
      }
    }
    conteoventana();
  } else if (nivel == 2) { //MENU
    if (posicionSelector == 168) {
      movselector(0);
    }
    else {
      movselector(-30);
    }
    movfelixmenu(100);
  } else if (nivel == 1) { //INICIO
    nivel = 2;
    bandera = 0;
  } else if (nivel == 4) {
    nivel = 1;
    bandera = 1;
  } else if (nivel == 5) {
    nivel = 1;
    bandera = 1;
  }
}
void Ralph_derecha(void) { // DerechaRalph
  if (nivel == 3) {  //JUEGO
    if (posicionralphX == 136) {
      movralph(0, 100, ralph5dnvl3, ralph2nvl3, ralph1nvl3);
    }
    else {
      movralph(33, 100, ralph5dnvl3, ralph2nvl3, ralph1nvl3);
    }
  } else if (nivel == 2) { //MENU
    if (posicionSelector == 168) { //INICIAR
      nivel = 3;
      bandera = 0;
    } else if (posicionSelector == 198) { //MUTEAR
      FillRect(0, 0, 240, 320, 0x0000);
      Serial2.write('6');
      digitalWrite(sound, HIGH);
      digitalWrite(sound, LOW);
      nivel = 2;
      bandera = 0;
    } else if (posicionSelector == 228) { // CREDITOS
      FillRect(0, 0, 240, 320, 0x0000);
      Serial2.write('7');
      digitalWrite(sound, HIGH);
      digitalWrite(sound, LOW);
      if (!SD.begin(scard)) {
        Serial.println("initialization failed!");
        return;
      }
      Serial.println("initialization done.");
      myFile = SD.open("creditos.txt");
      if (myFile) {
        Serial.println("creditos.txt:");

        // read from the file until there's nothing else in it:
        int yc = 10;
        while (myFile.available()) {
          String datasd = myFile.readStringUntil(' ');
          Serial.println(datasd);
          LCD_Print(datasd, 50, yc, 1, 0xffff, 0x0000);
          delay(1500);
          yc = yc + 20;
          if (yc == 330){
            break;
          }
        }
        
        // close the file:
        myFile.close();
      } else {
        // if the file didn't open, print an error:
        Serial.println("error opening creditos.txt");
      }
      nivel = 2;
      bandera = 0;
    }
  } else if (nivel == 1) { //INICIO
    nivel = 2;
    bandera = 0;
  } else if (nivel == 4) {
    nivel = 1;
    bandera = 1;
  } else if (nivel == 5) {
    nivel = 1;
    bandera = 1;
  }
}

void Ralph_izquierda(void) {   //IzquierdaRalph
  if (nivel == 3) {       //JUEGO
    if (posicionralphX == 37) {
      movralph(0, 100, ralph5invl3, ralph2nvl3, ralph1nvl3);
    }
    else {
      movralph(-33, 100, ralph5invl3, ralph2nvl3, ralph1nvl3);
    }
  } else if (nivel == 2) { //MENU
    if (posicionSelector == 168) {
      movselector(0);
    }
    else {
      movselector(-30);
    }
    movralph(0, 100, ralph2nvl3, ralph3nvl3, ralph1nvl3);
  } else if (nivel == 1) { //INICIO
    nivel = 2;
    bandera = 0;
  } else if (nivel == 4) {
    nivel = 1;
    bandera = 1;
  } else if (nivel == 5) {
    nivel = 1;
    bandera = 1;
  }
}

void Ralph_ataque(void) { //AtaqueRalph
  if (nivel == 3) { // JUEGO
    disparo = 1;
    movralph(0, 100, ralph3nvl3, ralph4nvl3, ralph1nvl3);
  } else if (nivel == 2) { //MENU
    if (posicionSelector == 228) {
      movselector(0);
    } else {
      movselector(30);
    }
    movralph(0, 100, ralph2nvl3, ralph3nvl3, ralph1nvl3);
  } else if (nivel == 1) { //INICIO
    nivel = 2;
    bandera = 0;
  } else if (nivel == 4) {
    nivel = 1;
    bandera = 1;
  } else if (nivel == 5) {
    nivel = 1;
    bandera = 1;
  }

}

//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {

  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
  Serial.begin(115200);
  Serial2.begin(115200);
  SPI.setModule(0);

  pinMode(scard, OUTPUT);
  pinMode(sound, OUTPUT);
  pinMode(Farriba, INPUT);
  pinMode(Fabajo, INPUT);
  pinMode(Fderecha, INPUT);
  pinMode(Fizquierda, INPUT);

  pinMode(Rizquierda, INPUT);
  pinMode(Rderecha, INPUT);
  pinMode(Rataque, INPUT);


  //Anclar Interrupciones a los pines
  attachInterrupt(digitalPinToInterrupt(Farriba), Felix_arriba, FALLING);
  attachInterrupt(digitalPinToInterrupt(Fabajo), Felix_abajo, FALLING);
  attachInterrupt(digitalPinToInterrupt(Fderecha), Felix_derecha, FALLING);
  attachInterrupt(digitalPinToInterrupt(Fizquierda), Felix_izquierda, FALLING);

  attachInterrupt(digitalPinToInterrupt(Rizquierda), Ralph_izquierda, FALLING);
  attachInterrupt(digitalPinToInterrupt(Rderecha), Ralph_derecha, FALLING);
  attachInterrupt(digitalPinToInterrupt(Rataque), Ralph_ataque, FALLING);
  interrupts();

  LCD_Init();
  LCD_Clear(0x00);

  //  INICIO
  nivel = 1;
  disparo = 0;
  //LCD_Bitmap(0, 0, 240, 320, fondo);
  //LCD_Bitmap(87, 5, 65, 53, inicio);

  orientationfelix = 1;

}
//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************
void loop() {
  if (nivel == 1) {

    digitalWrite(sound, HIGH);
    digitalWrite(sound, LOW);
    Serial2.write('1');
    reiniciovar();
    bandera = 1;
    LCD_Bitmap(0, 0, 240, 320, fondoinicio);
    while (bandera == 1) {
      String start = "START";
      LCD_Print(start, 73, 180, 2, 0xffff, 0x0000);
      delay(300);
      FillRect(73, 180, 80, 15, 0x0000);
      delay(300);
    }
  }
  else if (nivel == 2) {

    digitalWrite(sound, HIGH);
    digitalWrite(sound, LOW);
    Serial2.write('2');
    bandera = 1;
    FillRect(0, 0, 240, 320, 0x0000);
    posicionfelixX = 140;
    posicionfelixY = 97;
    posicionralphX = 65;
    posicionralphY  = 64;
    posicionSelector = 168;

    String text1 = "Menu";
    LCD_Print(text1, 87, 30, 2, 0xffff, 0x0000);

    String text2 = "Iniciar";
    LCD_Print(text2, 70, 170, 2, 0xffff, 0x0000);
    String text3 = "Mutear";
    LCD_Print(text3, 70, 200, 2, 0xffff, 0x0000);
    String text4 = "Creditos";
    LCD_Print(text4, 70, 230, 2, 0xffff, 0x0000);

    LCD_Bitmap(posicionralphX, posicionralphY, 66, 66, ralph1nvl3);
    LCD_Bitmap(posicionfelixX, posicionfelixY, 30, 33, felixmenu2);
    LCD_Bitmap(50, posicionSelector, 16, 20, selector);

    for (int y = 0; y < 320; y = y + 10) {
      LCD_Bitmap(0, y, 10, 10, tapladrillo);
      LCD_Bitmap(10, y, 10, 10, tapladrillo);
      LCD_Bitmap(230, y, 10, 10, tapladrillo);
      LCD_Bitmap(220, y, 10, 10, tapladrillo);
      delay(50);
    }
    while (bandera == 1) {
      if (posicionSelector == 168) {
        LCD_Print(text3, 70, 200, 2, 0xffff, 0x0000);
        LCD_Print(text4, 70, 230, 2, 0xffff, 0x0000);
        //INICIAR
        LCD_Bitmap(50, posicionSelector, 16, 20, selector);
        LCD_Print(text2, 70, 170, 2, 0xffff, 0x7a7a7a);
        delay(500);
        FillRect(50, posicionSelector, 16, 20, 0x0000);
        LCD_Print(text2, 70, 170, 2, 0xffff, 0x0000);
        delay(500);

      } else if (posicionSelector == 198) {
        LCD_Print(text2, 70, 170, 2, 0xffff, 0x0000);
        LCD_Print(text4, 70, 230, 2, 0xffff, 0x0000);
        //MUTEAR
        LCD_Bitmap(50, posicionSelector, 16, 20, selector);
        LCD_Print(text3, 70, 200, 2, 0xffff, 0x7a7a7a);
        delay(500);
        FillRect(50, posicionSelector, 16, 20, 0x0000);
        LCD_Print(text3, 70, 200, 2, 0xffff, 0x0000);
        delay(500);

      } else if (posicionSelector == 228) {
        LCD_Print(text2, 70, 170, 2, 0xffff, 0x0000);
        LCD_Print(text3, 70, 200, 2, 0xffff, 0x0000);
        // CREDITOS
        LCD_Bitmap(50, posicionSelector, 16, 20, selector);
        LCD_Print(text4, 70, 230, 2, 0xffff, 0x7a7a7a);
        delay(500);
        FillRect(50, posicionSelector, 16, 20, 0x0000);
        LCD_Print(text4, 70, 230, 2, 0xffff, 0x0000);
        delay(500);

      }
    }
  }
  else if (nivel == 3) {

    digitalWrite(sound, HIGH);
    digitalWrite(sound, LOW);
    Serial2.write('3');
    bandera = 1;
    FillRect(0, 0, 240, 320, 0x0000);
    posicionfelixX = 36;
    posicionfelixY = 264;
    posicionralphX = 37;
    posicionralphY  = 3;
    orientationfelix = 1;
    LCD_Bitmap(0, 0, 240, 320, fondoinicio);
    LCD_Bitmap(87, 3, 66, 66, rstfondo3ralph);
    LCD_Bitmap(posicionfelixX, posicionfelixY, 31, 39, felix1d);
    LCD_Bitmap(posicionralphX, posicionralphY, 66, 66, ralph1nvl3);
    while (bandera == 1) {

      if (conteofelix[1] == 1 && conteofelix[2] == 1 && conteofelix[3] == 1 && conteofelix[4] == 1 && conteofelix[5] == 1 && conteofelix[6] == 1 && conteofelix[7] == 1 && conteofelix[8] == 1 && conteofelix[9] == 1 && conteofelix[10] == 1 && conteofelix[11] == 1 && conteofelix[12] == 1 && conteofelix[13] == 1 && conteofelix[14] == 1 && conteofelix[15] == 1 && conteofelix[16] == 1 && conteofelix[17] == 1 && conteofelix[18] == 1 && conteofelix[19] == 1) {
        nivel = 4;
        bandera = 0;
        disparo = 0;
        break;
      }

      if (disparo == 1) {
        posicionLadrilloX = posicionralphX;
        for (int y = 115; y < 300; y = y + 10) {
          LCD_Bitmap(posicionLadrilloX + 26, y, 10, 10, ladrillo);
          LCD_Bitmap(posicionLadrilloX + 26, y - 10, 10, 10, tapladrillo);
          LCD_Bitmap(posicionLadrilloX + 26, 295, 10, 10, tapladrillo);
          posicionLadrilloY = y + 10;
          delay(10);
          if (posicionLadrilloY >= posicionfelixY + 20) {
            if (posicionLadrilloX >= posicionfelixX && posicionLadrilloX <= posicionfelixX + 33) {
              nivel = 5;
              disparo = 0;
              bandera = 0;
              break;
            }
          }
        }
        disparo = 0;
      } else {
        delay(10);
      }
    }
  } else if (nivel == 4) {
    Serial2.write('4');
    digitalWrite(sound, HIGH);
    digitalWrite(sound, LOW);
    FillRect(0, 0, 240, 320, 0x0000);
    String win1 = "J1 WINNER";
    LCD_Print(win1, 50, 180, 2, 0xffff, 0x0000);
    while (bandera == 0) {
      posicionfelixX = 110;
      posicionfelixY = 105;
      LCD_Bitmap(posicionfelixX, posicionfelixY, 30, 33, felixmenu1);
      delay(100);
      LCD_Bitmap(posicionfelixX, posicionfelixY, 30, 33, felixmenu2);
      delay(100);
      LCD_Bitmap(posicionfelixX, posicionfelixY, 30, 33, felixmenu3);
    }
  } else if (nivel == 5) {
    Serial2.write('5');
    digitalWrite(sound, HIGH);
    digitalWrite(sound, LOW);
    FillRect(0, 0, 240, 320, 0x0000);
    String win2 = "J2 WINNER";
    LCD_Print(win2, 50, 180, 2, 0xffff, 0x0000);
    while (bandera == 0) {
      LCD_Bitmap(85, 80, 66, 66, ralph2nvl3);
      delay(100);
      LCD_Bitmap(85, 80, 66, 66, ralph3nvl3);
      delay(100);
      LCD_Bitmap(85, 80, 66, 66, ralph1nvl3);

    }
  }
}

void reiniciovar(void) {
  for (int i = 0; i < 20; i++) {
    conteofelix[i] = 0;
  }
}

void conteoventana(void) {
  if (posicionfelixY == 264) { //sumarle 50 o restarle
    if (posicionfelixX == 36) { //sumarle 33 o restarle
      conteofelix[0] = 1;
    } else if (posicionfelixX == 69) {
      conteofelix[1] = 1;
    } else if (posicionfelixX == 102) {
      conteofelix[2] = 1;
    } else if (posicionfelixX == 135) {
      conteofelix[3] = 1;
    } else if (posicionfelixX == 168) {
      conteofelix[4] = 1;
    }

  } else if (posicionfelixY == 214) {
    if (posicionfelixX == 36) { //sumarle 33 o restarle
      conteofelix[5] = 1;
    } else if (posicionfelixX == 69) {
      conteofelix[6] = 1;
    } else if (posicionfelixX == 102) {
      conteofelix[7] = 1;
    } else if (posicionfelixX == 135) {
      conteofelix[8] = 1;
    } else if (posicionfelixX == 168) {
      conteofelix[9] = 1;
    }

  } else if (posicionfelixY == 164) {
    if (posicionfelixX == 36) { //sumarle 33 o restarle
      conteofelix[10] = 1;
    } else if (posicionfelixX == 69) {
      conteofelix[11] = 1;
    } else if (posicionfelixX == 102) {
      conteofelix[12] = 1;
    } else if (posicionfelixX == 135) {
      conteofelix[13] = 1;
    } else if (posicionfelixX == 168) {
      conteofelix[14] = 1;
    }

  } else if (posicionfelixY == 114) {
    if (posicionfelixX == 36) { //sumarle 33 o restarle
      conteofelix[15] = 1;
    } else if (posicionfelixX == 69) {
      conteofelix[16] = 1;
    } else if (posicionfelixX == 102) {
      conteofelix[17] = 1;
    } else if (posicionfelixX == 135) {
      conteofelix[18] = 1;
    } else if (posicionfelixX == 168) {
      conteofelix[19] = 1;
    }
  }
}


//FUNCIONES NUEVAS
void movfelixmenu(int sprintedelay) {
  LCD_Bitmap(140, 97, 30, 33, felixmenu1);
  delay(sprintedelay);
  LCD_Bitmap(140, 97, 30, 33, felixmenu2);
  delay(sprintedelay);
  LCD_Bitmap(140, 97, 30, 33, felixmenu3);
}
void movfelix(int x, int y, int sprintedelay, unsigned char pos1[], unsigned char pos2[], unsigned char pos3[]) {
  LCD_Bitmap(posicionfelixX, posicionfelixY, 31, 39, rstfondofelix);
  posicionfelixX = posicionfelixX + x;
  posicionfelixY = posicionfelixY + y;
  LCD_Bitmap(posicionfelixX, posicionfelixY, 31, 39, pos1);
  delay(sprintedelay);
  LCD_Bitmap(posicionfelixX, posicionfelixY, 31, 39, pos2);
  delay(sprintedelay);
  LCD_Bitmap(posicionfelixX, posicionfelixY, 31, 39, pos3);
}
void movralph(int x, int sprintedelay, unsigned char pos1[], unsigned char pos2[], unsigned char pos3[]) {
  LCD_Bitmap(posicionralphX, posicionralphY, 66, 66, rstfondo3ralph);
  posicionralphX = posicionralphX + x;
  LCD_Bitmap(posicionralphX, posicionralphY, 66, 66, pos1);
  delay(sprintedelay);
  LCD_Bitmap(posicionralphX, posicionralphY, 66, 66, pos2);
  delay(sprintedelay);
  LCD_Bitmap(posicionralphX, posicionralphY, 66, 66, pos3);
}
void movselector(int y) {
  FillRect(50, posicionSelector, 16, 20, 0x0000);
  posicionSelector = posicionSelector + y;
  LCD_Bitmap(50, posicionSelector, 16, 20, selector);
}
/*void movralph(int x, int sprintedelay, unsigned char pos1[], unsigned char pos2[], unsigned char pos3[], unsigned char rstfondoralph[]){ //VARIOS FONDOS

  LCD_Bitmap(posicionralphX, posicionralphY, 66, 66, rstfondoralph);
  posicionralphX = posicionralphX + x;
  LCD_Bitmap(posicionralphX, posicionralphY, 66, 66, pos1);
  delay(sprintedelay);
  LCD_Bitmap(posicionralphX, posicionralphY, 66, 66, pos2);
  delay(sprintedelay);
  LCD_Bitmap(posicionralphX, posicionralphY, 66, 66, pos3);
  }*/

//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************

void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_DC, OUTPUT);
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_DC, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER)
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x80 | 0x08);
  //LCD_DATA(0x40|0x80|0x20|0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
  //  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_DC, LOW);
  SPI.transfer(cmd);
}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_DC, HIGH);
  SPI.transfer(data);
}
//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);
  LCD_CMD(0x2c); // Write_memory_start
}
//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c) {
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_DC, HIGH);
  digitalWrite(LCD_CS, LOW);
  SetWindows(0, 0, 239, 319); // 479, 319);
  for (x = 0; x < 240; x++)
    for (y = 0; y < 320; y++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);
    }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_DC, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_DC, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y + h, w, c);
  V_line(x  , y  , h, c);
  V_line(x + w, y  , h, c);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  unsigned int i;
  for (i = 0; i < h; i++) {
    H_line(x  , y  , w, c);
    H_line(x  , y + i, w, c);
  }
}
//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background)
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;

  if (fontSize == 1) {
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if (fontSize == 2) {
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }

  char charInput ;
  int cLength = text.length();
  Serial.println(cLength, DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength + 1];
  text.toCharArray(char_array, cLength + 1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1) {
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2) {
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_DC, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + width;
  y2 = y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k + 1]);
      //LCD_DATA(bitmap[k]);
      k = k + 2;
    }
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_DC, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 =   x + width;
  y2 =    y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  int k = 0;
  int ancho = ((width * columns));
  if (flip) {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width - 1 - offset) * 2;
      k = k + width * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k - 2;
      }
    }
  } else {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width + 1 + offset) * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k + 2;
      }
    }
  }
  digitalWrite(LCD_CS, HIGH);
}
