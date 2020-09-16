#include <TM1638plus.h>
#include <timer.h>
#include <EEPROM.h>

// init afficheur
#define  STROBE_TM 2
#define  CLOCK_TM 3
#define  DIO_TM 4

TM1638plus tm(STROBE_TM, CLOCK_TM, DIO_TM);

auto timer = timer_create_default();

const int NOMBRE_FILS_RESERVOIR = 3;
const int PINS_RESERVOIR [NOMBRE_FILS_RESERVOIR] = { A0, A1, A2 };// Hauteur eau
// const int 
// 
//  
//    [NOMBRE_FILS_RESERVOIR]  = { 5, 12, 13 };// le 11 est pour bipbop

const int NOMBRE_ELEMENTS_FUSEE = 4;
const int PINS_LED_ELEMENTS [NOMBRE_ELEMENTS_FUSEE]  = { 7, 8, 9, 10 };
const int PIN_DETECTION_ETAGES_SUPERIEURS = A3;
const int PIN_DETECTION_ETAGE_INFERIEUR = 7;

const int PIN_BUZZER = 11;
const int PIN_LAUNCH = A4;// Led affiche possibilité de lancement (=tout est activé)
const int PIN_BOUTON_SET_CODE = A6;// Pin Lecture analogique seulement !
const int PIN_NEYMAN = A7;// Pin Lecture analogique seulement !

const int PIN_FUMIGENE = 6;

const bool WITH_SOUND = false;

bool niveauAtteint[3]     = {0};
bool readyToLaunch       = false;
bool neymanActive        = false;
bool buttonPressed       = false;
int nbElementsConnectes  = 0;// tous les elements de la fusee sont connectée entre eux
bool reservoirPlein      = false;
bool codesMatch          = false;
bool buzzing             = false;
bool launched            = false;
char* code       = "00000000";// code de départ
char* secretCode = retrieveSecretCode();

bool buttonHold[8] = {false};
int etagesConnectes[4] = {0};



/* ---------------------------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------ Fusée --------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------------------- */


/* ------------------------------------ reservoir ------------------------------------ */

// Lis et affiche la hauteur de carburant
void hauteurReservoirCarburant() {
    static int hauteurEauValues[3] = {0};// par défaut, le pin est en l'air (> 0)
    static int hauteurEauValues_prev[3] = {0};
    
    static int reservoirConf = -1;
    static int reservoirConf_prev = -1;

    memcpy(hauteurEauValues_prev, hauteurEauValues, sizeof(hauteurEauValues));

    // Lecture A0-A2
    for ( int pinNumber = 0 ; pinNumber < NOMBRE_FILS_RESERVOIR ; pinNumber++ ) {
        hauteurEauValues[pinNumber] = smooth(hauteurEauValues[pinNumber], analogRead(PINS_RESERVOIR[pinNumber]));
        niveauAtteint[pinNumber] = hauteurEauValues[pinNumber] > 300;
    }
    
    reservoirConf_prev = reservoirConf;
    reservoirConf = (niveauAtteint[2] << 2) + (niveauAtteint[1] << 1) + niveauAtteint[0];

    // Serial.print("Reservoir changed: 1:"+String(hauteurEauValues[0])+" 2:"+String(hauteurEauValues[1])+" 3:"+String(hauteurEauValues[2])+"\n");
    if ( reservoirConf != reservoirConf_prev ) {
        // Serial.print("HAUTEUR RESERVOIR CHANGED !");
        tuding(800 + (niveauAtteint[2] ? 400 : 0) + (niveauAtteint[1] ? 400 : 0) );
    }

    reservoirConf_prev = reservoirConf;

    reservoirPlein = (niveauAtteint[0] && niveauAtteint[1] && niveauAtteint[2]);

}

/* ------------------------------------ elements fusee ------------------------------------ */



// Lis et affiche le nombre de parties de fusée connectées
// Plus il y a d'éléments connectés, plus la résistance baisse (éléments en parallèle)
// donc plus on connecte d'éléments, plus la valeur lue sera élevée
void connectionElementsFusee() {
    static int etagesConf = -1;
    static int etagesConf_prev = -1;
    static int etagesSuperieurs = -1;

    etagesSuperieurs = smooth( etagesSuperieurs , analogRead(PIN_DETECTION_ETAGES_SUPERIEURS));

    etagesConnectes[0] = digitalRead(PIN_DETECTION_ETAGE_INFERIEUR) == LOW ? 1 : 0;// Inter inversé (LOW = connecté)
    // Serial.print("Resistance etages superieurs:"+String(etagesSuperieurs)+"\n");

    if ( etagesSuperieurs <= 20) {        // etagesSuperieurs == 0  => débranché
        etagesConnectes[0] = 0;
        etagesConnectes[1] = 0;
        etagesConnectes[2] = 0;
        etagesConnectes[3] = 0;
    } else if ( etagesSuperieurs <= 150) {// etagesSuperieurs == 40  => etages 1, 2
        etagesConnectes[1] = 1;
        etagesConnectes[2] = 1;
        etagesConnectes[3] = 0;
    } else if ( etagesSuperieurs <= 400) {// etagesSuperieurs == 323  => etages 1, 2, 3
        etagesConnectes[1] = 1;
        etagesConnectes[2] = 1;
        etagesConnectes[3] = 1;
    } else if ( etagesSuperieurs <= 700) {// etagesSuperieurs == 523 => etages 2
        etagesConnectes[1] = 1;
        etagesConnectes[2] = 0;
        etagesConnectes[3] = 0;
    } else if ( etagesSuperieurs <= 1000) {// etagesSuperieurs == 52 => etages 2
        Serial.print("AIE !\n");
    }

    // identifiant unique de la configuration des étages (détecte les changes)
    etagesConf = (etagesConnectes[3] << 3) + (etagesConnectes[2] << 2) + (etagesConnectes[1] << 1) + etagesConnectes[0];

    if ( etagesConf != etagesConf_prev ) {
        Serial.print("etages connectes changed: "+String(etagesSuperieurs)+" 1:"+estConnecte(etagesConnectes[0])+" 2:"+estConnecte(etagesConnectes[1])+" 3:"+estConnecte(etagesConnectes[2])+" 4:"+estConnecte(etagesConnectes[3])+"\n");
        tudiiWindows();
    }

    etagesConf_prev = etagesConf;

    montrerEtagesConnectesSurBandeauLed(etagesConf);
}

String estConnecte(int etage) {
    return etage == 1 ? "Y" : "N";
}

/* ------------------------------------ Fumigène ------------------------------------ */

void pouf () {
    bool launch = readyToLaunch && neymanActive;

    if ( launch && !launched) {
        neverGiveYouUp();
        launched = true;
    }

    digitalWrite(PIN_FUMIGENE, launch ? HIGH : LOW);
}



/* ---------------------------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------ Mallette ------------------------------------------------------------ */
/* ---------------------------------------------------------------------------------------------------------------------------- */



// le neyman n'a pas de pull-up/down => fait la moyenne de la sortie pour lisser les valeurs induites (patte en l'air)
void neyman() {
    static int neymanSmoothed = 0;
    int neymanBefore = neymanActive;

    neymanSmoothed = smooth(neymanSmoothed, analogRead(PIN_NEYMAN));

    neymanActive = neymanSmoothed < 50;

    if ( neymanActive != neymanBefore ) 
        Serial.print("Neyman changed: "+String(neymanActive));
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

    readyToLaunch = (nbElementsConnectes == NOMBRE_ELEMENTS_FUSEE) 
    && reservoirPlein 
    && codesMatch;

    digitalWrite(PIN_LAUNCH, readyToLaunch ? HIGH : LOW);
}


/* ---------------------------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------ Main ---------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------------------- */
void setup() {

    // blingBling(); // lel

    /**** fusée ****/
    pinMode(PIN_FUMIGENE, OUTPUT);


    /**** Mallette ****/
    for ( int i = 0 ; i < NOMBRE_ELEMENTS_FUSEE ; i++) {
        pinMode(PINS_LED_ELEMENTS[i], OUTPUT);
    }

    if ( WITH_SOUND )
        pinMode(PIN_BUZZER, OUTPUT);

    pinMode(PIN_DETECTION_ETAGE_INFERIEUR, INPUT);

    pinMode(PIN_LAUNCH, OUTPUT);
    pinMode(PIN_BOUTON_SET_CODE, INPUT_PULLUP);
    pinMode(PIN_NEYMAN, INPUT_PULLUP);


    Serial.begin(9600);
}

void loop() {

    digitalWrite(PIN_FUMIGENE, LOW);
    // ------- fusée -------
    hauteurReservoirCarburant();

    connectionElementsFusee();

    // ------- mallette -------
    changeChiffreCode();

    boutonAdminMemoriserCode();

    neyman();

    voyantLaunch();

    // Serial.print(String(nbElementsConnectes == NOMBRE_ELEMENTS_FUSEE) + " " + reservoirPlein + " " + codesMatch + " " +readyToLaunch + " " + neymanActive + " " + (readyToLaunch && neymanActive) + "\n");

    // allume le fumi
    pouf();

    timer.tick();
}

