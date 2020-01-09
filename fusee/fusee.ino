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

bool niveauAtteint[3]  = {false};
bool neymanActive      = false;
bool buttonPressed     = false;
bool elementsConnectes = false;// tous les elements de la fusee sont connectée entre eux
bool reservoirPlein    = false;
bool codesMatch        = false;
char* code       = "00000000";// code de départ
char* secretCode = retrieveSecretCode();

bool buttonHold[8] = {false};





/* ---------------------------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------ Fusée --------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------------------- */



/* ------------------------------------ reservoir ------------------------------------ */

// Lis et affiche la hauteur de carburant
void hauteurReservoirCarburant() {
    static int hauteurEauValues[3] = {0};// par défaut, le pin est en l'air (> 0)
    int nombreNiveauAtteint = 0;

    // Lecture et Affichage A0-A2
    for ( int pinNumber = 0 ; pinNumber < PIN_COUNT_RESERVOIR ; pinNumber++) {
        // Lecture et Affichage

        hauteurEauValues[pinNumber] = smooth(hauteurEauValues[pinNumber], analogRead(PINS_RESERVOIR[pinNumber]));
        
        niveauAtteint[pinNumber] = hauteurEauValues[pinNumber] > 50;
        
        digitalWrite(PINS_LED_RESERVOIR[pinNumber], niveauAtteint[pinNumber] ? HIGH : LOW);

        nombreNiveauAtteint++;
        
        // Serial.print(String(pinNumber) + ":" + hauteurEauValues[pinNumber] + "\t");
    }

    reservoirPlein = ( nombreNiveauAtteint == PIN_COUNT_RESERVOIR );

    // Serial.print("\n");
}




/* ------------------------------------ elements fusee ------------------------------------ */



// Lis et affiche le nombre de parties de fusée connectées
// Plus il y a d'éléments connectés, plus la résistance baisse (éléments en parallèle)
// donc plus on connecte d'éléments, plus la valeur lue sera faible
void connectionElementsFusee() {

    elementsSmoothed = smooth( elementsSmoothed , analogRead(PIN_READ_ELEMENTS));

    digitalWrite(PINS_LED_ELEMENTS[0], elementsSmoothed < 400 ? HIGH : LOW);// seuil: 346
    digitalWrite(PINS_LED_ELEMENTS[1], elementsSmoothed < 300 ? HIGH : LOW);// seuil: 165
    digitalWrite(PINS_LED_ELEMENTS[2], elementsSmoothed < 100 ? HIGH : LOW);// seuil: 89
    digitalWrite(PINS_LED_ELEMENTS[3], elementsSmoothed < 70 ? HIGH : LOW);// seuil: 56

    elementsConnectes = analVal < 70; // @TODO: A RETAPER 
}

/* ------------------------------------ Fumigène ------------------------------------ */

void pouf () {
    digitalWrite(PIN_FUMIGENE, HIGH);
}



/* ---------------------------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------ Mallette ------------------------------------------------------------ */
/* ---------------------------------------------------------------------------------------------------------------------------- */



// le neyman n'a pas de pull-up/down => fait la moyenne de la sortie pour lisser les valeurs induites (patte en l'air)
void neyman() {
    static int neymanSmoothed = 0;

    neymanSmoothed = smooth(neymanSmoothed, analogRead(PIN_NEYMAN));

    neymanActive = neymanSmoothed > 50;
}

void boutonAdminMemoriserCode() {
    bool setCode = !(analogRead(PIN_BOUTON_SET_CODE) > 500);

    if ( setCode ) {
        doLEDs(255);
        saveSecretCode(code);
        delay(1500);
        doLEDs(0);
    }
}

// incrément des chiffres du code, affichage de la LED correspondante 
void changeChiffreCode () {
    uint8_t buttons = tm.readButtons();

    ajouteChiffre(buttons, code);
    
    // les leds sont controlées par l'appui bouton ou la validité du code entré (blink)
    if ( !codesMatch ) {
        doLEDs(buttons);  
    }
}

void afficheVoyantLaunch() {
    digitalWrite(PIN_LAUNCH, HIGH);
}


/* ---------------------------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------ Main ---------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------------------- */
void setup() {

    blingBling(); // lel

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


    Serial.begin(9600);

}

void loop() {

    // fusée
    hauteurReservoirCarburant();
    
    connectionElementsFusee();
    
    // mallette

    changeChiffreCode();

    boutonAdminMemoriserCode();

    neyman();
    
    if ( elementsConnectes && reservoirPlein && codesMatch ) {
        afficheVoyantLaunch();
        if ( neymanActive ){
            pouf();
        }
    }

    timer.tick();
}

