#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h>
// ****//
// decalarations des instance de servvo moteur
Servo verti_servo;
Servo hori_servo;
EthernetClient client;

int    HTTP_PORT   = 80 ;
String HTTP_METHOD = "GET";
char   HOST_NAME[] = "192.168.187.242"; // change to your PC's IP address
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// replacer des nemro des PINs par des variables siginifique
#define ldrtopr A0    // top-right LDR orange
#define ldrtopl A1    // top-left LDR white Brown
#define ldrbotr A2    // bottom-right LDR white green
#define ldrbotl A3    // bottom-left LDR Blue
// les PIN qu'on va brancher les sevo motor
#define horiPin 7
#define vertiPin 9
// Sensibility Horizontal et verticale
int Hsensibility = 30;      // measurement sensitivity
int Vsensibility = 30;      // measurement sensitivity
int mode = 0;
// min and max of servo mouv
int H_min = 25 , H_max = 173 , V_min = 0 , V_max = 180  ; 
// declaration des variable pour recevoie des valeur des chaque LDR
int topl, topr, botl, botr, avgtop, avgbot, avgleft, avgright = 0;
// variable pour stocker la position acctuelle du chaque servo
int leftRightPosition, upDownPosition = 0;
int step = 5;
//variable pour lire les valeurs des servo
int ch,cv;
String cy="";
// Power variable  
float tension = 1.0  , courant = 0.2;
// pre position get by getControlObject func
int pre_Hposi , pre_Vposi = 0 ;
// decalration des antetette des fontions utiliser
void manualMode();
void automaticMode();
void pushPosition() ;
void pushLDR_Power();
void getControlObject();
// ------------------------------------------------------------------------------------------------------------//
void setup()
{
  // initialisation des Serial et les E/S direction
  Serial.begin(115200);

  verti_servo.attach(vertiPin); // Servo motor up-down movement
  hori_servo.attach(horiPin);   // Servo motor right-left movement
  verti_servo.write(90);
  hori_servo.write(90);

  // obtenir l'adresse IP du machine
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to obtaining an IP address using DHCP");
    while(true);
  }
  Serial.println("Initialisation done .");
  delay(2000);
}

void loop()
{
  // pour notifier la mode actuelle et activer le mode convonable
  pushLDR_Power() ;
  getControlObject();
  Serial.print("mode=");
  Serial.println(mode);
  if (mode == 0)
  {
    Serial.println("Manual");
    manualMode();
  }
  else
  {
    Serial.println("Automatic");
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

  // Serial.println("-------------- Get avr --------------");
  // Serial.print("average of top LDRs : ");
  // Serial.println(avgtop);
  // Serial.print("average of bottom LDRs : ");
  // Serial.println(avgbot);
  // Serial.print("average of left LDRs :");
  // Serial.println(avgleft);
  // Serial.print("average of right LDRs :");
  // Serial.println(avgright);

  // calculer le different entre (top et bottom) et entre (right et left)
  int Vdifferent = avgtop - avgbot;
  int Hdifferent = avgright - avgleft;

  // Serial.println("----------------- Get Differrent -----------------");
  // Serial.print("different LDRs top and LDRs bot : ");
  // Serial.println(Vdifferent);
  // Serial.print("different LDRs right and LDRs left : ");
  // Serial.println(Hdifferent);

  // commande de moteur horizontal **//
  // commande les moteur : si la valeur absulue différent entre top et bottom > de la sensibilite
  if (abs(Hdifferent) >= Hsensibility)
  {
    leftRightPosition = hori_servo.read();
    Serial.print("left right Position : ");
    Serial.println(leftRightPosition);
    if (Hdifferent > 0)
    {
      if(leftRightPosition < H_max)
      {   
        hori_servo.write(leftRightPosition + step);
      }
    }
    if (Hdifferent < 0)
    {
      if (leftRightPosition > H_min)
      {
        hori_servo.write(leftRightPosition - step);
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
    if (Vdifferent > 0)
    {
      if (upDownPosition < V_max)
      {
        verti_servo.write(upDownPosition + step);
      }
    }
    if (Vdifferent < 0)
    {
      if (upDownPosition > V_min)
      {
        verti_servo.write(upDownPosition - step);
      }
    }
    Serial.println("--------- vertical Servo Control -------------");
  }
  pushPosition();
}

void manualMode()
{
  leftRightPosition = pre_Hposi ;
  upDownPosition = pre_Vposi ; 
  verti_servo.write(upDownPosition) ; 
  hori_servo.write(leftRightPosition) ; 
}

void pushPosition(){
  String PATH_NAME   = "/push";
  String qSHp ="?hposi=";
  String qSVp ="&vposi=";
   // connect to web server on port 80:
  if(client.connect(HOST_NAME, HTTP_PORT)) {
    // if connected:
    Serial.println("Connected to server");
    // make a HTTP request:
    // send HTTP header
    client.println(HTTP_METHOD + " " + PATH_NAME + qSHp + leftRightPosition + qSVp + upDownPosition + " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println(); // end HTTP header

    while(client.connected()) {
      if(client.available()){
        // read an incoming byte from the server and print it to serial monitor:
        char c = client.read();
        // Serial.print(c);
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

void pushLDR_Power(){
   // connect to web server on port 80:
  if(client.connect(HOST_NAME, HTTP_PORT)) {
    // if connected:
    Serial.println("Connected to server");
    // make a HTTP request:
    // send HTTP header
    client.println(HTTP_METHOD + " " + "/push" + "?ldrtr=" + topr + "&ldrtl=" + topl + "&ldrbr=" + botr + "&ldrbl=" + botl + "&tension=" + tension + "&courant=" + courant + " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println(); // end HTTP header

    while(client.connected()) {
      if(client.available()){
        // read an incoming byte from the server and print it to serial monitor:
        char c = client.read();
        // Serial.print(c);
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
void getControlObject(){
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
        // Serial.print(c);
        cy=cy+c;
      }
    }
  // *******************************************************
// get Servo Position
        Serial.println();
        // preMode=cy[179]-'0';
        // Serial.println(preMode);
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
        pre_Hposi =cv;
        Serial.println(pre_Hposi);
        // hori_servo.write(h);
        pre_Vposi=ch;
        Serial.println(pre_Vposi);
        // verti_servo.write(v);
  // ********************************************************
// end get Servo Position
// get mode 
        mode=cy[179]-'0';
// end get mode         
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