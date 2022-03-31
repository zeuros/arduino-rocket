#include <Arduino.h>
#include <EEPROM.h>
#include "arduino-timer.h"

const int NOMBRE_FILS_RESERVOIR = 3;
const int PINS_RESERVOIR [NOMBRE_FILS_RESERVOIR] = { A0, A1, A2 };// Hauteur eau


const int PIN_DETECTION_ETAGE_INFERIEUR = 6; // à mettre en pull-up
const int PIN_DETECTION_ETAGES_SUPERIEURS = A0;
const int PIN_FUMIGENE = 7;

bool niveauAtteint[3] = {0};
int nbElementsConnectes = 0; // tous les elements de la fusee sont connectée entre eux
bool reservoirPlein = false;

bool etagesConnectes[4] = {0};

auto timer = timer_create_default();

void montrerEtagesConnectesSurBandeauLed(int etagesConnectes)
{
    // @TODO: do it !
}

// ajoute une valeur au tableau de moyennes et retourne la moyenne des 10 dernières valeurs.
// évite les sautes de valeurs dues à des perturbations
int smooth(int avg, int newValue, int amount = 5)
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

// Lis et affiche le nombre de parties de fusée connectées
// Plus il y a d'éléments connectés, plus la résistance baisse (éléments en parallèle)
// donc plus on connecte d'éléments, plus la valeur lue sera élevée
bool connectionElementsFusee() {
    static int etagesConf = -1;
    static int etagesConf_prev = -1;
    static int etagesSuperieurs = -1;

    etagesSuperieurs = smooth( etagesSuperieurs , analogRead(PIN_DETECTION_ETAGES_SUPERIEURS));

    // Serial.print("Resistance etages superieurs:" + String(etagesSuperieurs)+"\n");

    // Valeurs de PIN_DETECTION_ETAGES_SUPERIEURS: 4091 => pas d'etage superieur connecté // 1080 => 1er etage seulement // 3740 => etages 1 & 2

    // Paliers: 0 - 2410 (2e etage seulement) // 2410 - 3915 (etages 2 & 3) // 3915+ ( pas d'etage superieur connecté)

    etagesConnectes[0] = !!digitalRead(PIN_DETECTION_ETAGE_INFERIEUR); // Inter inversé (LOW = connecté)
    etagesConnectes[1] = true;

    if (etagesSuperieurs < 2410) { // 1er etage seulement
        etagesConnectes[2] = true;
        etagesConnectes[3] = false;
    } else if (etagesSuperieurs < 3915) {
        etagesConnectes[2] = true;
        etagesConnectes[3] = true;
    } else {
        etagesConnectes[2] = false;
        etagesConnectes[3] = false;
    }
    // Serial.println(String(etagesSuperieurs));

    // identifiant unique de la configuration des étages (détecte les changes)
    // etagesConsCf = (etagesConnectes[3] << 3) + (etagesConnectes[2] << 2) + (etagesConnectes[1] << 1) + etagesConnectes[0];

    // if ( etagesConf != etagesConf_prev ) {
    //     for (auto &etage : etagesConnectes)
    //         Serial.print(etage ? "=" : " ");
    //      Serial.print(">\n");
    // }

    // etagesConf_prev = etageonf;

    // montrerEtagesConnectesSurBandeauLed(etagesConf);
}


/* ------------------------------------ Fumigène ------------------------------------ */

void pouf () {
    // bool launch = readyToLaunch && neymanActive;

    // if ( launch && !launched) {
    //     launched = true;
    // }

    digitalWrite(PIN_FUMIGENE, HIGH);
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
    pinMode(PIN_DETECTION_ETAGES_SUPERIEURS, INPUT_PULLUP);

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