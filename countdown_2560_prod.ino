/* Affichage Compte à reboursh
*
*
* 2016 Julien Auberson Theâtre de Vidy Lausanne
* 
* 13.09 Blanc et rouge se suivent, ajout de 0, ajout de info, enlever les trucs inutiles door show etc..
* ajout des 0 dans le decompte blanc, inversion !!!!!! de M se S dans la saisie !!!! :)***--_ooo
* V0.4.1 cleaN des des merdes inuiles... et toutes les variables
* V0.4  ajout enregistrement variables sur eeprom
* V0.3: ajout saisie variables par port série
* V0.3 SC : Utiliisation de la librairie Serial Command.
*
*/

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#include <SerialCommand.h>
#include <EEPROMex.h>

SerialCommand SCmd;   // Initialisation objet Scmd
#define PIN 6  // PIN pour NeoPixel
#define Button_show 2
#define Button_door 3
#define Button_test 4


int i = 0;
char run = 0; // shows if timer is runnig
int door = 0; // Door countdown
int show = 0; // Show countdown
char END = 0; // variable fin décompte Show.
boolean looping = false;


// user setted and "turn on" values

int default_Sec;
int default_Min;
// current values
int Sec;
int Min;
int Min_S;
int Sec_S;
int Sec_countdown;
int Min_countdown;
int brightness;

//timer interrupt

ISR(TIMER1_OVF_vect) {
  Sec--; // timer values decreases
  TCNT1 = 0x0BDC;

}


Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, PIN,
                            NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
                            NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
                            NEO_GRB            + NEO_KHZ800);


const uint16_t colors[] = {
  matrix.Color(200, 0, 0), matrix.Color(180, 180, 180 ), matrix.Color(176, 23, 31), matrix.Color(0, 200, 0), matrix.Color(225, 100, 255)
};



void setup()

{
  // Default timer value definition

  Min_S = EEPROM.readInt(1);
  Sec_S = EEPROM.readInt(3);
  brightness = 12;
  Min_countdown = EEPROM.readInt(7);
  Sec_countdown = EEPROM.readInt(9);

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(12);
  //matrix.setTextColor(colors[0]);
  matrix.fillScreen(0);

  pinMode(Button_door, INPUT_PULLUP);
  //digitalWrite(Button_door, HIGH); //pull up resistor
  pinMode(Button_show, INPUT_PULLUP);
  //digitalWrite(Button_show, HIGH); //pull up resistor
  pinMode(Button_test, INPUT_PULLUP);

  TIMSK1 = 0x01; // enabled global and timer overflow interrupt;
  TCCR1A = 0x00; // normal operation 148 page (mode0);
  TCNT1 =  0x0BDC;
  TCCR1B = 0x00; // stop hardware timer



  SCmd.addDefaultHandler(unrecognized); // Handler for command that isn't matched  (says "What?")
  SCmd.addCommand("T", process_time); // Saisie du temps
  SCmd.addCommand("I", info); //

  SCmd.addCommand("D", start_door); //
  SCmd.addCommand("B", set_brightness); //

  SCmd.addCommand("R", start_reset); //
  SCmd.addCommand("U", process_countdown_time); //



  Serial.begin(9600);

  Serial.print(("READY :)"));
  Serial.println("");


};




void loop ()
{
  //brightnesscontrol();


  SCmd.readSerial();

  default_Sec = Min_S;
  default_Min = Sec_S;


  if (!digitalRead(Button_door)) {
    start_door() ;
  }
  if (!digitalRead(Button_show)) {
    start_show() ;
  }
  if (!digitalRead(Button_test)) {
    start_reset() ;
  }



  if (Sec == -1) {
    Min--;
    Sec = 59;
  }

  ///ajout de 0 dans décompte série/////

  if (run) {
    if (Min <= 9) Serial.print("0");  // adjust for 0-9
    Serial.print(Min);
    Serial.print(".");
    if (Sec <= 9) Serial.print("0");  // adjust for 0-9
    Serial.println (Sec);
  }

  /// Conversion en format MM:SS    ////

  byte Tc = (Min / 10);
  byte Td = (Min % 10);
  byte Te = (Sec / 10);
  byte Tf = (Sec % 10);
  char num[11] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
  char Tm[1] = {num[Tc]};
  char Tm2[1] = {num[Td]};
  char Ts[1] = {num[Te]};
  char Ts2[1] = {num[Tf]};
  char showhour[9] = {Tm[0], Tm2[0], ':', Ts[0], Ts2[0]};




  if (run) {  // to do
    matrix.fillScreen(0);
    matrix.setCursor(1, 1);
    matrix.print(showhour);
    matrix.show();
  }

  else {
    Min = Min_countdown;
    Sec = Sec_countdown;
    matrix.fillScreen(0);
    matrix.setCursor(1, 1);
    if (Min <= 9) matrix.print("0");  // adjust for 0-9
    matrix.print(Min);
    matrix.print(":");
    if (Sec <= 9) matrix.print("0");  // adjust for 0-9
    matrix.print (Sec);

   // matrix.print(Min_countdown);matrix.print(":"); matrix.print(Sec_countdown); // Chiffres statiques
    matrix.show();
    //delay(150);
  }



  ///Fin du compte à rebours "Porte"
  if ((Min == 0) && (Sec == 0) && run && door) {
   // run = 0;
    //TCCR1B = 0x00; //stop timer
    door = 0;
  Serial.println("Decompte Rouge");
  Sec = Min_S;
  Min = Sec_S;
  //TCCR1B = 0x04;
  run = 1;
  show = 1;
  matrix.setTextColor(colors[0]);



    END = 1;
  }

  ///Fin du compte à rebours "Show" ///
  if ((Min == 0) && (Sec == 0) && run && show) {
    run = 0;
    show = 0;
    Serial.println("RED Countdown END");

    matrix.setTextColor(colors[1]);
    TCCR1B = 0x00; //stop timer
    END = 1;

  }


  // END
  if (END) {
    END = 0;
    Sec = default_Sec ;
    Min = default_Min ;

  }

};


void unrecognized()
{
  Serial.println("What?");
}




void process_time()
{

  char *arg;

  Serial.println("RED Countdown Sec : Min");
  arg = SCmd.next();
  if (arg != NULL)
  {
    Min_S = atoi(arg);  // Converts a char string to an integer

    Serial.print("Seconde: ");
    Serial.println(Min_S);
    EEPROM.writeInt(1, Min_S);
  }
  else {
    Serial.println("No arguments");
  }

  arg = SCmd.next();
  if (arg != NULL)
  {
    Sec_S = atol(arg);
    Serial.print("Minute: ");
    Serial.println(Sec_S);
    EEPROM.writeInt(3, Sec_S);
  }
  else {
    Serial.println("No second argument");
  }

}



void process_countdown_time()
{
 

  char *arg;

  Serial.println("Decompte BLANC");
  arg = SCmd.next();
  if (arg != NULL)
  {
    Min_countdown = atoi(arg);  // Converts a char string to an integer

    Serial.print("Minute: ");
    Serial.println(Min_countdown);
    EEPROM.writeInt(7, Min_countdown);
  }
  else {
    Serial.println("No arguments");
  }

  arg = SCmd.next();
  if (arg != NULL)
  {
    Sec_countdown = atol(arg);
    Serial.print("Seconde: ");
    Serial.println(Sec_countdown);
    EEPROM.writeInt(9, Sec_countdown);
  }
  else {
    Serial.println("No second argument");
  }
}






void start_show()
{
  Serial.println("RED Countdown Starting...");
 // Min = Min_S;
 // Sec = Sec_S;
  TCCR1B = 0x04;
  run = 1;
  show = 1;
  matrix.setTextColor(colors[0]);
}

void start_door() {
  Serial.println("WHITE Countdown Starting...");
  TCCR1B = 0x04;
  run = 1;
  door = 1;
  Min = Min_countdown;
  Sec = Sec_countdown ;
  matrix.setTextColor(colors[1]);
  


}

void set_brightness() {


  char *arg;

  Serial.println("We're in process_command");
  arg = SCmd.next();
  if (arg != NULL)
  {
    int  brights = atoi(arg);  // Converts a char string to an integer

    Serial.print("Brightness: ");
    Serial.println(brights);
    matrix.setBrightness(brights);
    EEPROM.writeInt(5, brights);

  }
  else {
    Serial.println("No arguments");
  }



}


void start_reset() {


  Serial.println("Command received: RESET");
  matrix.fillScreen(0);
  Min = Min_S;
  Sec = Sec_S;
  TCCR1B = 0x00;
  run = 0;
  show = 0;
  door = 0;
  //matrix.print("00:00");
  matrix.setTextColor(colors[1]); // defaullt color !!
 
}
void info() {
     Serial.print("Total LED time : ");
     Serial.print("WHITE: ");Serial.print(EEPROM.readInt(7)); Serial.print(" Min");  Serial.print(" + ");   Serial.print(EEPROM.readInt(9)); Serial.print(" Sec");
     
     Serial.print(" RED: ");Serial.print(EEPROM.readInt(3)); Serial.print(" Min "); Serial.print(" + ");  Serial.print(EEPROM.readInt(1)); Serial.print(" Sec");
     
    ;Serial.print(" = "); Serial.print(((EEPROM.readInt(7)*60)+EEPROM.readInt(9))+(EEPROM.readInt(3)*60)+EEPROM.readInt(1));Serial.print(" Sec");
    Serial.println("");

          
     
  
}



