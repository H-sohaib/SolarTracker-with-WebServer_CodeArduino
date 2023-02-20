#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h>
// ****//
// decalarations des instance de servvo moteur
Servo verti_servo;
Servo hori_servo;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

EthernetClient client;

int    HTTP_PORT   = 80 ;
String HTTP_METHOD = "GET";
char   HOST_NAME[] = "192.168.187.242"; // change to your PC's IP address

// replacer des nemro des PINs par des variables siginifique
#define ldrtopr A0    // top-right LDR
#define ldrtopl A1    // top-left LDR
#define ldrbotr A2    // bottom-right LDR
#define ldrbotl A3    // bottom-left LDR

// les PIN qu'on va brancher les sevo motor
#define horiPin 7
#define vertiPin 9
// Signalisation LEDs
#define manueLed 2
#define autoLed 3 // push button pour switcher le controle entre l'axe horizontal et vertical

// Sensibility Horizontal et verticale
int Hsensibility = 50;      // measurement sensitivity
int Vsensibility = 50;      // measurement sensitivity

int mode = 0;
// declaration des variable pour recevoie des valeur des chaque LDR
int topl, topr, botl, botr, avgtop, avgbot, avgleft, avgright = 0;
// variable pour stocker la position acctuelle du chaque servo
int leftRightPosition, upDownPosition = 0;
int step = 1;


void setup(){
    // ******//
  // initialisation des Serial et les E/S direction
  Serial.begin(9600);
  pinMode(manueLed, OUTPUT);
  pinMode(autoLed, OUTPUT);

  verti_servo.attach(vertiPin); // Servo motor up-down movement
  hori_servo.attach(horiPin);   // Servo motor right-left movement
  verti_servo.write(40);
  delay(500) ;
  verti_servo.write(70);
  hori_servo.write(90);
  
  Serial.println("initialisation done .");
  delay(4000);
}

void loop(){
  
  // Lire les valeur des 4 LDRs
  topr = analogRead(ldrtopr); // top right LDR
  topl = analogRead(ldrtopl); // top left LDR
  botr = analogRead(ldrbotr); // bot right LDR
  botl = analogRead(ldrbotl); // bot left LDR

  // calculer les moyenne les 4 coté (right left top bottom)
  int avgtop = (topr + topl) / 2;   // average of top LDRs
  int avgbot = (botr + botl) / 2;   // average of bottom LDRs
  int avgleft = (topl + botl) / 2;  // average of left LDRs
  int avgright = (topr + botr) / 2; // average of right LDRs

  Serial.println("-------------- Get avr --------------");
  Serial.print("average of top LDRs : ");
  Serial.println(avgtop);
  Serial.print("average of bottom LDRs : ");
  Serial.println(avgbot);
  Serial.print("average of left LDRs :");
  Serial.println(avgleft);
  Serial.print("average of right LDRs :");
  Serial.println(avgright);

  // calculer le different entre (top et bottom) et entre (right et left)
  int Vdifferent = avgtop - avgbot;
  int Hdifferent = avgright - avgleft;

  Serial.println("----------------- Get Differrent -----------------");
  Serial.print("different LDRs top and LDRs bot : ");
  Serial.println(Vdifferent);
  Serial.print("different LDRs right and LDRs left : ");
  Serial.println(Hdifferent);

  // commande de moteur horizontal **//
  // commande les moteur : si la valeur absulue différent entre top et bottom > de la sensibilite
  if (abs(Hdifferent) >= Hsensibility)
  {
    leftRightPosition = hori_servo.read();
    Serial.print("left right Position : ");
    Serial.println(leftRightPosition);
    if (Hdifferent > 0)
    {
      if(leftRightPosition < 173)
      {   
        leftRightPosition = leftRightPosition + step ;
        hori_servo.write(leftRightPosition);
      }
    }
    if (Hdifferent < 0)
    {
      if (leftRightPosition > 7)
      {
        leftRightPosition = leftRightPosition - step ;
        hori_servo.write(leftRightPosition);
      }
    }
    Serial.println("--------- horizontal Servo Control -------------");
  }

  // commande de moteur vertical **//
  // commande les moteur : si la valeur absulue différent entre right et left > de la sensibilite
  if (abs(Vdifferent) >= Vsensibility)
  {
    upDownPosition = verti_servo.read();
    Serial.print("up down Position : ");
    Serial.println(upDownPosition);
    if (Vdifferent < 0)
    {
      if (upDownPosition < 173)
      {
        verti_servo.write(upDownPosition + step);
      }
    }
    if (Vdifferent > 0)
    {
      if (upDownPosition > 7)
      {
        verti_servo.write(upDownPosition - step);
      }
    }
    Serial.println("--------- vertical Servo Control -------------");
  }
  
  // delay(10) ;
}