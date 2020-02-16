#include <TM1638plus.h>
#include <timer.h>
#include <EEPROM.h>

// init afficheur
#define  STROBE_TM 2
#define  CLOCK_TM 3
#define  DIO_TM 4

TM1638plus tm(STROBE_TM, CLOCK_TM, DIO_TM);

auto timer = timer_create_default();

const int PIN_COUNT_RESERVOIR = 3;
const int PINS_LED_RESERVOIR [PIN_COUNT_RESERVOIR]  = { 5, 12, 13 };// le 11 est pour bipbop
const int PINS_RESERVOIR [PIN_COUNT_RESERVOIR] = { A0, A1, A2 };

const int PIN_COUNT_ELEMENTS = 4;
const int PINS_LED_ELEMENTS [PIN_COUNT_ELEMENTS]  = { 7, 8, 9, 10 };
const int PIN_READ_ELEMENTS = A3;

const int PIN_BUZZER = 11;
const int PIN_LAUNCH = A4;
const int PIN_BOUTON_SET_CODE = A6;// Pin Lecture analogique seulement !
const int PIN_NEYMAN = A7;// Pin Lecture analogique seulement !

const int PIN_FUMIGENE = 6;

const int NB_CONF_STAGES = 7;
const int NOMBRE_TOTAL_ELEMENTS = 4;

int niveauAtteint[3]     = {0};
bool readyToLaunch       = false;
bool neymanActive        = false;
bool buttonPressed       = false;
int nbElementsConnectes  = 0;// tous les elements de la fusee sont connectée entre eux
bool reservoirPlein      = false;
bool codesMatch          = false;
bool buzzing             = false;
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

    // Lecture et Affichage A0-A2
    for ( int pinNumber = 0 ; pinNumber < PIN_COUNT_RESERVOIR ; pinNumber++) {
        // Lecture et Affichage

        hauteurEauValues[pinNumber] = smooth(hauteurEauValues[pinNumber], analogRead(PINS_RESERVOIR[pinNumber]));
        
        niveauAtteint[pinNumber] = hauteurEauValues[pinNumber] > 50;
        
        digitalWrite(PINS_LED_RESERVOIR[pinNumber], niveauAtteint[pinNumber] ? HIGH : LOW);

    }

    reservoirPlein = ( sum(niveauAtteint, PIN_COUNT_RESERVOIR) == PIN_COUNT_RESERVOIR );
}




/* ------------------------------------ elements fusee ------------------------------------ */



// Lis et affiche le nombre de parties de fusée connectées
// Plus il y a d'éléments connectés, plus la résistance baisse (éléments en parallèle)
// donc plus on connecte d'éléments, plus la valeur lue sera élevée
void connectionElementsFusee() {
    static int elementsSmoothed = 1024;

    // tension liée au nombre d'éléments connectés
    elementsSmoothed = smooth( elementsSmoothed , analogRead(PIN_READ_ELEMENTS));

    static int thresholds[NB_CONF_STAGES] = {/* valeurs_de_palier */
        /*843 => */834,
        /*826 => */726,
        /*625 => */581,
        /*537 => */475, // = (537+412) / 2
        /*412 => */296,
        /*180 => */90,
        /*0 => */0
    };
    static int stages[NB_CONF_STAGES][4] = {// {etage 0, etage 1, etage 2, etage 3}}
        {1, 1, 1, 1},
        {0, 1, 1, 1},
        {1, 1, 1, 0},
        {0, 1, 1, 0},
        {1, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 0, 0}
    };
    
    for(int i = 0; i < NB_CONF_STAGES; i++){
        // si on trouve un palier proche de la mesure, on en déduit les éléments connectés (voir tableau stages)
        if ( elementsSmoothed > thresholds[i] ) {

            nbElementsConnectes = sum(stages[i], 4);

            for(int j=0 ; j < NOMBRE_TOTAL_ELEMENTS ; j++){// cherche les éléments dans la liste d'éléments à activer.
                digitalWrite(PINS_LED_ELEMENTS[j], stages[i][j] ? HIGH : LOW);
            }

            break;
        }
    }
}

/* ------------------------------------ Fumigène ------------------------------------ */

void pouf () {
    digitalWrite(PIN_FUMIGENE, (readyToLaunch && neymanActive) ? HIGH : LOW);
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

void voyantLaunch() {

    readyToLaunch = (nbElementsConnectes == NOMBRE_TOTAL_ELEMENTS) 
        && reservoirPlein 
        && codesMatch;

    digitalWrite(PIN_LAUNCH, readyToLaunch ? HIGH : LOW);
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


    // Serial.begin(9600);
}

void loop() {

    // ------- fusée -------
    hauteurReservoirCarburant();

    connectionElementsFusee();

    // ------- mallette -------
    changeChiffreCode();

    boutonAdminMemoriserCode();

    neyman();

    voyantLaunch();

    // Serial.print(String(nbElementsConnectes == NOMBRE_TOTAL_ELEMENTS) + " " + reservoirPlein + " " + codesMatch + " " +readyToLaunch + " " + neymanActive + " " + (readyToLaunch && neymanActive) + "\n");

    // allume le fumi
    pouf();

    timer.tick();
}

