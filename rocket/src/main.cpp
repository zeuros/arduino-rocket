#include <Arduino.h>
#include <EEPROM.h>
#include "arduino-timer.h"

const int NOMBRE_FILS_RESERVOIR = 3;
const int PINS_RESERVOIR [NOMBRE_FILS_RESERVOIR] = { A0, A1, A2 };// Hauteur eau

const int NOMBRE_ELEMENTS_FUSEE = 4;
const int PINS_LED_ELEMENTS [NOMBRE_ELEMENTS_FUSEE]  = { 7, 8, 9, 10 };
const int PIN_DETECTION_ETAGES_SUPERIEURS = A3;

const int PIN_DETECTION_ETAGE_INFERIEUR = 6; // à mettre en pull-up
const int PIN_FUMIGENE = 7;

bool niveauAtteint[3] = {0};
int nbElementsConnectes = 0; // tous les elements de la fusee sont connectée entre eux
bool reservoirPlein = false;

int etagesConnectes[4] = {0};

auto timer = timer_create_default();

void montrerEtagesConnectesSurBandeauLed(int etagesConnectes)
{
    // @TODO: do it !
}

// ajoute une valeur au tableau de moyennes et retourne la moyenne des 10 dernières valeurs.
// évite les sautes de valeurs dues à des perturbations
int smooth(int avg, int newValue, int amount = 20)
{
    return (avg * (amount - 1) + newValue) / amount;
}

int sum(int *arr, int size)
{
    int sum = 0;

    for (int i = 0; i < size; i++)
    {
        sum += arr[i];
    }

    return sum;
}


/* ------------------------------------ reservoir ------------------------------------ */

// Lis et affiche la hauteur de carburant
bool hauteurReservoirCarburant() {
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

    Serial.print("Reservoir changed: 1:"+String(hauteurEauValues[0])+" 2:"+String(hauteurEauValues[1])+" 3:"+String(hauteurEauValues[2])+"\n");
    if ( reservoirConf != reservoirConf_prev ) {
        // Serial.print("HAUTEUR RESERVOIR CHANGED !");
        // tuding(800 + (niveauAtteint[2] ? 400 : 0) + (niveauAtteint[1] ? 400 : 0) );
    }

    reservoirConf_prev = reservoirConf;

    reservoirPlein = (niveauAtteint[0] && niveauAtteint[1] && niveauAtteint[2]);

}

/* ------------------------------------ elements fusee ------------------------------------ */

String estConnecte(int etage)
{
    return etage == 1 ? "Y" : "N";
}

// Lis et affiche le nombre de parties de fusée connectées
// Plus il y a d'éléments connectés, plus la résistance baisse (éléments en parallèle)
// donc plus on connecte d'éléments, plus la valeur lue sera élevée
bool connectionElementsFusee() {
    // static int etagesConf = -1;
    // static int etagesConf_prev = -1;
    // static int etagesSuperieurs = -1;

    // etagesSuperieurs = smooth( etagesSuperieurs , analogRead(PIN_DETECTION_ETAGES_SUPERIEURS));

    // etagesConnectes[0] = digitalRead(PIN_DETECTION_ETAGE_INFERIEUR) == LOW ? 1 : 0; // Inter inversé (LOW = connecté)
    Serial.println("Bas étage: " + String(digitalRead(PIN_DETECTION_ETAGE_INFERIEUR) == HIGH ? "HIGH" : "LOW"));

    // // Serial.print("Resistance etages superieurs:"+String(etagesSuperieurs)+"\n");

    // if ( etagesSuperieurs <= 20) {        // etagesSuperieurs == 0  => débranché
    //     etagesConnectes[0] = 0;
    //     etagesConnectes[1] = 0;
    //     etagesConnectes[2] = 0;
    //     etagesConnectes[3] = 0;
    // } else if ( etagesSuperieurs <= 150) {// etagesSuperieurs == 40  => etages 1, 2
    //     etagesConnectes[1] = 1;
    //     etagesConnectes[2] = 1;
    //     etagesConnectes[3] = 0;
    // } else if ( etagesSuperieurs <= 400) {// etagesSuperieurs == 323  => etages 1, 2, 3
    //     etagesConnectes[1] = 1;
    //     etagesConnectes[2] = 1;
    //     etagesConnectes[3] = 1;
    // } else if ( etagesSuperieurs <= 700) {// etagesSuperieurs == 523 => etages 2
    //     etagesConnectes[1] = 1;
    //     etagesConnectes[2] = 0;
    //     etagesConnectes[3] = 0;
    // } else if ( etagesSuperieurs <= 1000) {// etagesSuperieurs == 52 => etages 2
    //     Serial.print("AIE !\n");
    // }

    // // identifiant unique de la configuration des étages (détecte les changes)
    // etagesConf = (etagesConnectes[3] << 3) + (etagesConnectes[2] << 2) + (etagesConnectes[1] << 1) + etagesConnectes[0];

    // if ( etagesConf != etagesConf_prev ) {
    //     Serial.print("etages connectes changed: "+String(etagesSuperieurs)+" 1:"+estConnecte(etagesConnectes[0])+" 2:"+estConnecte(etagesConnectes[1])+" 3:"+estConnecte(etagesConnectes[2])+" 4:"+estConnecte(etagesConnectes[3])+"\n");
    // }

    // etagesConf_prev = etagesConf;

    // montrerEtagesConnectesSurBandeauLed(etagesConf);
}


/* ------------------------------------ Fumigène ------------------------------------ */

void pouf () {
    // bool launch = readyToLaunch && neymanActive;

    // if ( launch && !launched) {
    //     launched = true;
    // }

    digitalWrite(PIN_FUMIGENE, HIGH);
    delay(1000);
    digitalWrite(PIN_FUMIGENE, LOW);
    delay(1000);
}


/* ---------------------------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------ Main ---------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------------------- */
void setup() {
    // Serial can be used in callbacks, make sure to init it before.
    Serial.begin(9600);

    /**** fusée ****/
    pinMode(PIN_FUMIGENE, OUTPUT);
    digitalWrite(PIN_FUMIGENE, LOW);

    // Etage inferieur LOW -> HIGH (because of pullup) when plugged
    pinMode(PIN_DETECTION_ETAGE_INFERIEUR, INPUT_PULLUP);

    // timer.every(500, hauteurReservoirCarburant);
    // timer.every(500, connectionElementsFusee);

    // pinMode(PIN_LAUNCH, OUTPUT);

}

void loop() {

    // hauteurReservoirCarburant();
    connectionElementsFusee();

    // allume le fumi
    pouf();

    timer.tick();
}