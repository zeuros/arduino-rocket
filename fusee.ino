#include <TM1638plus.h>
#include <EEPROM.h>
#include <timer.h>

#define  STROBE_TM 2
#define  CLOCK_TM 3
#define  DIO_TM 4

// init afficheur
TM1638plus tm(STROBE_TM, CLOCK_TM, DIO_TM);
auto timer = timer_create_default();

const int PIN_COUNT_RESERVOIR = 3;
const int PINS_LED_RESERVOIR [PIN_COUNT_RESERVOIR]  = { 11, 12, 13 };
const int PINS_RESERVOIR [PIN_COUNT_RESERVOIR] = { A0, A1, A2 };

const int PIN_COUNT_ELEMENTS = 4;
const int PINS_LED_ELEMENTS [PIN_COUNT_ELEMENTS]  = { 7, 8, 9, 10 };
const int PIN_READ_ELEMENTS = A3;

const int PIN_BUZZER = 5;
const int PIN_LAUNCH = A4;
const int PIN_BOUTON_SET_CODE = A6;// Pin Lecture analogique seulement !
const int PIN_NEYMAN = A7;// Pin Lecture analogique seulement !

const int PIN_FUMIGENE = 6;

int neymanValues[10] = {1024};// par défaut, le pin est en l'air (> 0)
int analVal = 0;
bool neymanActive;
bool buttonPressed;
char* code       = "00000000";// code de départ
char* secretCode = retrieveSecretCode();
bool codesMatch  = false;

bool buttonHold[8] = {false};





/* ---------------------------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------ Fusée --------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------------------- */



/* ------------------------------------ reservoir ------------------------------------ */

// Lis et affiche la hauteur de carburant
void hauteurReservoirCarburant() {
    // Lecture et Affichage A0-A2
    for ( int pinNumber = 0 ; pinNumber < PIN_COUNT_RESERVOIR ; pinNumber++) {
        // Lecture et Affichage AX
        analVal = analogRead(PINS_RESERVOIR[pinNumber]);
        digitalWrite(PINS_LED_RESERVOIR[pinNumber], HIGH);
        // Serial.print(String(pinNumber) + ":" + analVal);  Serial.print("\t");
    }
    // Serial.print("\n");
}




/* ------------------------------------ elements fusee ------------------------------------ */


void setupPinsElements() {
    for ( int i = 0 ; i < PIN_COUNT_ELEMENTS ; i++) {
        pinMode(PINS_LED_ELEMENTS[i], OUTPUT);
    }
}

// Lis et affiche le nombre de parties de fusée connectées
// Plus il y a d'éléments connectés, plus la résistance baisse (éléments en parallèle)
// donc plus on connecte d'éléments, plus la valeur lue sera faible
void connectionPartiesFusee() {

    analVal = analogRead(PIN_READ_ELEMENTS);

    digitalWrite(PINS_LED_ELEMENTS[0], analVal < 400 ? HIGH : LOW);// seuil: 346
    digitalWrite(PINS_LED_ELEMENTS[1], analVal < 300 ? HIGH : LOW);// seuil: 165
    digitalWrite(PINS_LED_ELEMENTS[2], analVal < 100 ? HIGH : LOW);// seuil: 89
    digitalWrite(PINS_LED_ELEMENTS[3], analVal < 70 ? HIGH : LOW);// seuil: 56
}

/* ------------------------------------ Fumigène ------------------------------------ */

/* PAS DE CODE POUR L'INSTANT */



/* ---------------------------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------ Mallette ------------------------------------------------------------ */
/* ---------------------------------------------------------------------------------------------------------------------------- */



void neyman() {
    neymanActive  = average(neymanValues, analogRead(PIN_NEYMAN)) > 50;// le neyman n'a pas de pull-up/down => moyenne de la sortie

    Serial.print(String("Neyman val: ")+neymanActive);
}

void boutonSetCode() {
    bool setCode = !(analogRead(PIN_BOUTON_SET_CODE) > 500);

    if ( setCode ) {
        doLEDs(255);
        saveSecretCode(code);
        delay(1500);
        doLEDs(0);
    }
}

// gère l'afficheur 7 segments & ses boutons.
void codeSecret() {

}

void successTone() {
    tone(PIN_BUZZER, 3000, 5000);
}

void failTones() {
//  failTone();
    // faire le second bip après 200ms
  timer.in(200, failTone);
}

void failTone(void *) {
    tone(PIN_BUZZER, 1000, 400);
}








/* ---------------------------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------ Main ---------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------------------- */
void setup() {

    /**** fusée ****/
    pinMode(PIN_FUMIGENE, OUTPUT);


    /**** Mallette ****/
    for ( int i = 0 ; i < PIN_COUNT_RESERVOIR ; i++) {
        pinMode(PINS_LED_RESERVOIR[i], OUTPUT);
    }

    for ( int i = 0 ; i < PIN_COUNT_ELEMENTS ; i++) {
        pinMode(PINS_LED_ELEMENTS[i], OUTPUT);
    }

    pinMode(PIN_BUZZER, OUTPUT);

    pinMode(PIN_LAUNCH, OUTPUT);
    pinMode(PIN_BOUTON_SET_CODE, INPUT_PULLUP);
    pinMode(PIN_NEYMAN, INPUT_PULLUP);

    blingBling();

    Serial.begin(9600);

}

void loop() {

    // fusée
    hauteurReservoirCarburant();
    connectionPartiesFusee();
    
    // mallette

    uint8_t buttons = tm.readButtons();
    ajouteChiffre(buttons, code);
    
    
    if ( !codesMatch ) {
        doLEDs(buttons);  
    }

    boutonSetCode();
    neyman();
    
    timer.tick();
}

