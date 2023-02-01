#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h>
// ********//
// decalarations des instance de servvo moteur
Servo verti_servo;
Servo hori_servo;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

EthernetClient client;

int    HTTP_PORT   = 80 ;
String HTTP_METHOD = "GET";
char   HOST_NAME[] = "192.168.56.242"; // change to your PC's IP address

// replacer des nemro des PINs par des variables siginifique
#define ldrtopr A0    // top-right LDR
#define ldrtopl A1    // top-left LDR
#define ldrbotr A2    // bottom-right LDR
#define ldrbotl A3    // bottom-left LDR
#define pot A4        // bottom-left LDR
#define modeButton 12 // push button pour switcher entre le  mode automatic et le mode manuel
#define axeButton 11
#define manueLed 2
#define autoLed 3 // push button pour switcher le controle entre l'axe horizontal et vertical

// les PIN qu'on va brancher les sevo motor
#define horiPin 7
#define vertiPin 9
// Sensibility Horizontal et verticale
int Hsensibility = 30;      // measurement sensitivity
int Vsensibility = 30;      // measurement sensitivity
int manuelSensibility = 60; // measurement sensitivity

int mode=1, axe, modeState, axeState, potValue, HprevpotValue, VprevpotValue = 0;
// declaration des variable pour recevoie des valeur des chaque LDR
int topl, topr, botl, botr, avgtop, avgbot, avgleft, avgright = 0;
// variable pour stocker la position acctuelle du chaque servo
int leftRightPosition, upDownPosition = 0;
//variable pour lire les valeurs des servo
int ch,cv;
String cy="";
// decalration des antetette des fontions utiliser
void manualMode();
void automaticMode();
void modeSwicher();
void sendVal();
void getVal();
// ------------------------------------------------------------------------------------------------------------//
void setup()
{
  pinMode(38,OUTPUT);
  digitalWrite(38,HIGH);
  // Serial.println("setup"); // serial connection setup  //opens serial port, sets data rate to 9600 bps
  // Serial.println("CLEARDATA");                       //clear all data that’s been place in already
  // Serial.println("LABEL,t,voltage,current,power,Mode");
  // **************//
  // initialisation des Serial et les E/S direction
  Serial.begin(9600);
  pinMode(modeButton, INPUT_PULLUP); // Mode switch Button
  pinMode(axeButton, INPUT_PULLUP);  // Axis switch
  pinMode(manueLed, OUTPUT);
  pinMode(autoLed, OUTPUT);

  verti_servo.attach(vertiPin); // Servo motor up-down movement
  hori_servo.attach(horiPin);   // Servo motor right-left movement
  verti_servo.write(90);
  hori_servo.write(40);

  // obtenir l'adresse IP du machine
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to obtaining an IP address using DHCP");
    while(true);
  }
  Serial.println("initialise");
  delay(2000);
}

void loop()
{
  
  // fonction pour detecter la click sur push button pour changer le mode (A/M)
  // modeSwicher();
  // pour notifier la mode actuelle et activer le mode convonable
  if (mode == 0)
  {
    Serial.println("Manual");
    manualMode();
    digitalWrite(manueLed, HIGH);
    digitalWrite(autoLed, LOW);
  }
  else
  {
    Serial.println("Automatic");
    digitalWrite(manueLed, LOW);
    digitalWrite(autoLed, HIGH);
    automaticMode();
  }
}

void automaticMode()
{
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

  // commande de moteur horizontal ******//
  // commande les moteur : si la valeur absulue différent entre top et bottom > de la sensibilite
  if (abs(Hdifferent) >= Hsensibility)
  {

    leftRightPosition = hori_servo.read();
    Serial.print("left right Position : ");
    Serial.println(leftRightPosition);
    if (Hdifferent > 0)
    {
      if (leftRightPosition < 180)
      {
        hori_servo.write(leftRightPosition + 2);
      }
    }
    if (Hdifferent < 0)
    {
      if (leftRightPosition > 0)
      {
        hori_servo.write(leftRightPosition - 2);
      }
    }
    Serial.println("--------- horizontal Servo Control -------------");
  }
  // modeSwicher();

  // commande de moteur vertical ******//
  // commande les moteur : si la valeur absulue différent entre right et left > de la sensibilite
  if (abs(Vdifferent) >= Vsensibility)
  {
    upDownPosition = verti_servo.read();
    Serial.print("up down Position : ");
    Serial.println(upDownPosition);
    if (Vdifferent > 0)
    {
      if (upDownPosition < 180)
      {
        verti_servo.write(upDownPosition + 2);
      }
    }
    if (Vdifferent < 0)
    {
      if (upDownPosition > 0)
      {
        verti_servo.write(upDownPosition - 2);
      }
    }
    Serial.println("--------- vertical Servo Control -------------");
  }
  modeSwicher();

}

void modeSwicher()
{
  
  // modeState = digitalRead(modeButton);
  // if (modeState == 0)
  // {
  //   //     condition   True False
  //   mode = (mode == 0) ? 1 : 0;
  // }
}
void manualMode()
{
  axeState = digitalRead(axeButton);
  //   condition   True False
  axe = (axeState == 0) ? 1 : 0;
  delay(50);
  if (axe == 0)
  {
    potValue = analogRead(pot);
    int potDifferent = potValue - HprevpotValue;
    if (abs(potDifferent) > manuelSensibility)
    {
      hori_servo.write(map(potValue, 0, 1023, 0, 180));
      HprevpotValue = potValue;
    }
  }
  else
  {
    potValue = analogRead(pot);
    int potDifferent = potValue - VprevpotValue;
    if (abs(potDifferent) > manuelSensibility)
    {
      verti_servo.write(map(potValue, 0, 1023, 0, 180));
      VprevpotValue = potValue;
    }
  }
}
void sendVal(){
  
  String PATH_NAME   = "/push";

  // topr = 449; //analogRead(ldrtopr); // top right LDR
  // topl = 233;//analogRead(ldrtopl); // top left LDR
  // botr = 925;//analogRead(ldrbotr); // bot right LDR
  // botl = 123;//analogRead(ldrbotl); // bot left LDR
  // leftRightPosition = 139;//hori_servo.read();
  // upDownPosition = 65;//verti_servo.read();
  // mode =0;
  String qSL1 ="?ldrtr=";
  String qSL2 ="&ldrtl=";
  String qSL3 ="&ldrbr=";
  String qSL4 ="&ldrbl=";
  String qSHp ="&hposi=";
  String qSVp ="&vposi=";
  String qSM ="&mode=";
   // connect to web server on port 80:
  if(client.connect(HOST_NAME, HTTP_PORT)) {
    // if connected:
    Serial.println("Connected to server");
    // make a HTTP request:
    // send HTTP header
    client.println(HTTP_METHOD + " " + PATH_NAME + qSL1 + topr + qSL2 + topl + qSL3 + botr + qSL4 + botl +  qSHp + leftRightPosition + qSVp + upDownPosition + qSM + mode + " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println(); // end HTTP header

    while(client.connected()) {
      if(client.available()){
        // read an incoming byte from the server and print it to serial monitor:
        char c = client.read();
        Serial.print(c);
      }
    }

    // the server's disconnected, stop the client:
    client.stop();
    Serial.println();
    Serial.println("disconnected");
  } else {// if not connected:
    Serial.println("connection failed");
  }
}
void getVal(){
  String PATH_NAME   = "/get";
   if(client.connect(HOST_NAME,HTTP_PORT))
  {
    Serial.println("connected");

    // add the Host here
    client.println(HTTP_METHOD + " " + PATH_NAME + " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println();

    while(client.connected()) {
      if(client.available()){
        // read an incoming byte from the server and print it to serial monitor:
        char c = client.read();
        Serial.print(c);
        cy=cy+c;
      }
    }
        Serial.println();
        mode=cy[179]-'0';
        Serial.println(mode);
        if(cy[182]==','){
        cv=cy[181]-'0';
          if(cy[184]==' '){
          ch=cy[183]-'0';
          }
          if(cy[185]==' '){
          ch=(10*(cy[183]-'0'))+cy[184]-'0';
          }
          if(cy[186]==' '){
            ch=(100*(cy[183]-'0'))+(10*(cy[184]-'0'))+(cy[185]-'0');
          }
        }
        if(cy[183]==','){
        cv=(10*(cy[181]-'0'))+cy[182]-'0';
          if(cy[185]==' '){
          ch=cy[184]-'0';
          }
          if(cy[186]==' '){
          ch=(10*(cy[184]-'0'))+cy[185]-'0';
          }
          if(cy[187]==' '){
            ch=(100*(cy[184]-'0'))+(10*(cy[185]-'0'))+(cy[186]-'0');
          }
        }
        if(cy[184]==','){
          cv=(100*(cy[181]-'0'))+(10*(cy[182]-'0'))+(cy[183]-'0');
          if(cy[186]==' '){
          ch=cy[185]-'0';
          }
          if(cy[187]==' '){
          ch=(10*(cy[185]-'0'))+cy[186]-'0';
          }
          if(cy[188]==' '){
            ch=(100*(cy[185]-'0'))+(10*(cy[186]-'0'))+(cy[187]-'0');
          }
        }
        leftRightPosition=cv;
        Serial.println(leftRightPosition);
        upDownPosition=ch;
        Serial.println(upDownPosition);
        verti_servo.write(leftRightPosition);
        hori_servo.write(upDownPosition);
        cv=0;
        ch=0;
        cy="";
    // the server's disconnected, stop the client:
    client.stop();
    Serial.println();
    Serial.println("disconnected");
  } else {// if not connected:
    Serial.println("connection failed");
  }
}