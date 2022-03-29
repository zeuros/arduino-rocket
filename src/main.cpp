#include <TM1638plus.h>
#include <arduino-timer.h>
#include <EEPROM.h>

#include "pitches.hpp"

// init afficheur
#define STROBE_TM 2     // strobe = GPIO connected to strobe line of module
#define CLOCK_TM 3      // clock = GPIO connected to clock line of module
#define DIO_TM 4        // data = GPIO connected to data line of module
bool high_freq = false; // default false, If using a high freq CPU > ~100 MHZ set to true.

TM1638plus tm(STROBE_TM, CLOCK_TM, DIO_TM, high_freq);

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

const int PIN_LAUNCH = A4; // Led affiche possibilité de lancement (=tout est activé)
const int PIN_BUZZER = 11;
const int PIN_BOUTON_SET_CODE = 10; // Pin Lecture analogique seulement !
const int PIN_NEYMAN = 12;// D12

const int PIN_FUMIGENE = 6;

const bool WITH_SOUND = false;

bool niveauAtteint[3] = {0};
bool readyToLaunch = false;
bool neymanActive = false;
bool buttonPressed = false;
int nbElementsConnectes = 0; // tous les elements de la fusee sont connectée entre eux
bool reservoirPlein = false;
bool codesMatch = false;
bool buzzing = false;
bool launched = false;
char *code = "00000000"; // code de départ
char *secretCode = "00000001";

bool buttonHold[8] = {false};
int etagesConnectes[4] = {0};

// MOVE TO afficheur.cpp

void doLEDs(uint8_t value)
{
    for (uint8_t position = 0; position < 8; position++)
    {
        tm.setLED(position, value & 1);
        value = value >> 1;
    }
}


bool clearLEDs()
{
    doLEDs(0);
    return false; // no repeat
}

void blingBling()
{

    tm.reset();
    tm.displayText("YOURCODE");

    int speed = 3;  // + = +lent
    int steps = 18; // + = +fluide & +lent

    for (int i = 0; i < 4; i++)
    {
        // decroissant
        for (uint8_t brightness = 5; brightness-- > 1;)
        {
            // sub steps
            for (int j = 0; j < steps; j++)
            {
                tm.brightness(brightness);
                delay((speed * (steps - j)) / steps);
                tm.brightness(brightness - 1);
                delay((speed * j) / steps);
            }
        }
        for (uint8_t brightness = 0; brightness < 5; brightness++)
        {
            // sub steps
            for (int j = 0; j < steps; j++)
            {
                tm.brightness(brightness);
                delay((speed * (steps - j)) / steps);
                tm.brightness(brightness + 1);
                delay((speed * j) / steps);
            }
        }
    }

    tm.brightness(3);
}

// répète blinkLeds tq les codes matchent
void blinkLeds()
{
    timer.every(500, [](void *) -> bool {

        timer.in(250, [](void*) -> bool {
            doLEDs(0);
            return false;
        });

        doLEDs(codesMatch ? 255 : 0);

        return codesMatch;
    });
}

void makeABuzz(bool condifion, int freq, int duration)
{
    if (condifion)
    {
        if (!buzzing)
        {
            buzzing = true;
            tone(PIN_BUZZER, freq, duration);
        }
    }
    else if (buzzing)
    {
        noTone(PIN_BUZZER);
        buzzing = false;
    }
}

void successTone()
{
    tone(PIN_BUZZER, 4000, 600);
}

void bouitBouit()
{
    for (int i = 0; i < 5; i++)
    {
        tone(PIN_BUZZER, 1200 + i * 100, 40);
        delay(10);
    }
}

void tuding(int min)
{
    tone(PIN_BUZZER, min, 40);
    delay(50);
    tone(PIN_BUZZER, min + 800, 40);
}

void tudiiWindows()
{
    tone(PIN_BUZZER, 300);
    delay(250);
    tone(PIN_BUZZER, 550);
    delay(150);
    noTone(PIN_BUZZER);
}

void notificationTone()
{
    tone(PIN_BUZZER, 1200, 40);
}

void boom()
{
    for (int i = 0; i < 100; i++)
    {
        tone(PIN_BUZZER, 1600 + i * 100, 40);
        delay(10);
    }
}

void neverGiveYouUp()
{
    int tempo = 130;

    // change this to whichever pin you want to use
    int buzzer = PIN_BUZZER;

    int melody[] = {
        NOTE_A4, 16, NOTE_B4, 16, NOTE_D5, 16, NOTE_B4, 16,
        NOTE_FS5, -8, NOTE_FS5, -8, NOTE_E5, -4, NOTE_A4, 16, NOTE_B4, 16, NOTE_D5, 16, NOTE_B4, 16,

        NOTE_E5, -8, NOTE_E5, -8, NOTE_D5, -8, NOTE_CS5, 16, NOTE_B4, -8, NOTE_A4, 16, NOTE_B4, 16, NOTE_D5, 16, NOTE_B4, 16, // 18
        NOTE_D5, 4, NOTE_E5, 8, NOTE_CS5, -8, NOTE_B4, 16, NOTE_A4, 8, NOTE_A4, 8, NOTE_A4, 8,
        NOTE_E5, 4, NOTE_D5, 2, NOTE_A4, 16, NOTE_B4, 16, NOTE_D5, 16, NOTE_B4, 16,
        NOTE_FS5, -8, NOTE_FS5, -8, NOTE_E5, -4, NOTE_A4, 16, NOTE_B4, 16, NOTE_D5, 16, NOTE_B4, 16,
        NOTE_A5, 4, NOTE_CS5, 8, NOTE_D5, -8, NOTE_CS5, 16, NOTE_B4, 8, NOTE_A4, 16, NOTE_B4, 16, NOTE_D5, 16, NOTE_B4, 16,

        NOTE_D5, 4, NOTE_E5, 8, NOTE_CS5, -8, NOTE_B4, 16, NOTE_A4, 4, NOTE_A4, 8, // 23
        NOTE_E5, 4, NOTE_D5, 2, REST, 4};

    // sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
    // there are two values per note (pitch and duration), so for each note there are four bytes
    int notes = sizeof(melody) / sizeof(melody[0]) / 2;

    // this calculates the duration of a whole note in ms
    int wholenote = (60000 * 4) / tempo;

    int divider = 0, noteDuration = 0;
    for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2)
    {

        // calculates the duration of each note
        divider = melody[thisNote + 1];
        if (divider > 0)
        {
            // regular note, just proceed
            noteDuration = (wholenote) / divider;
        }
        else if (divider < 0)
        {
            // dotted notes are represented with negative durations!!
            noteDuration = (wholenote) / abs(divider);
            noteDuration *= 1.5; // increases the duration in half for dotted notes
        }

        // we only play the note for 90% of the duration, leaving 10% as a pause
        tone(buzzer, melody[thisNote], noteDuration * 0.9);

        // Wait for the specief duration before playing the next note.
        delay(noteDuration);

        // stop the waveform generation before the next note.
        noTone(buzzer);
    }
}

// MOVE TO: secret_code.cpp

// affiche le code, vérifie s'il est juste
void weHaveANewCode(char *newCode)
{
    tm.displayText(newCode);

    codesMatch = String(code).equals(secretCode);

    codesMatch
        ? successTone()
        : notificationTone();

    if (codesMatch)
    {
        blinkLeds();
    }
}

void saveSecretCode(char *theCode)
{
    char toStore[9];
    strcpy(toStore, theCode);

    EEPROM.put(100, toStore);

    strcpy(secretCode, theCode);

    weHaveANewCode(theCode);
}

char *retrieveSecretCode()
{
    static char stored[9];

    EEPROM.get(100, stored);

    return stored;
}

void ajouteChiffre(uint8_t value, char *theCode)
{
    for (uint8_t position = 0; position < 8; position++)
    {
        buttonPressed = (value & 1);

        if (buttonPressed && !buttonHold[position])
        {

            // éteindre la led
            theCode[position]++;

            if (theCode[position] > ('9'))
            {
                theCode[position] = '0';
            }

            buttonHold[position] = true;

            // affiche le bouton pressé
            doLEDs(1 << position);

            weHaveANewCode(theCode);
        }

        buttonHold[position] = buttonPressed;

        value = value >> 1;
    }
}

// TODO: MOVE TO led_stuff.cpp

void montrerEtagesConnectesSurBandeauLed(int etagesConnectes)
{
    // @TODO: do it !
}

// TODO: MOVE TO tools.cpp
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

String estConnecte(int etage)
{
    return etage == 1 ? "Y" : "N";
}

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

bool boutonAdminMemoriserCode(void *argument)
{
    if (digitalRead(PIN_BOUTON_SET_CODE))
    {
        Serial.println("Saving secret code: " + String(code));
        doLEDs(255);
        saveSecretCode(code);
        delay(1500);
        doLEDs(0);
    }

    return true;
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

// Checks for neyman status change
bool neymanCheck(void *argument)
{
    if (neymanActive != digitalRead(PIN_NEYMAN))
    {
        Serial.println("Neyman changed: " + String(neymanActive));
        neymanActive = !neymanActive;
    }

    return true; // to repeat the action - false to stop
}

/* ---------------------------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------ Main ---------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------------------- */
void setup() {
    // Serial can be used in callbacks, make sure to init it before.
    Serial.begin(9600);

    // blingBling(); // lel
    secretCode = retrieveSecretCode();

    /**** fusée ****/
    pinMode(PIN_FUMIGENE, OUTPUT);
    digitalWrite(PIN_FUMIGENE, LOW);

    /**** Mallette ****/
    for ( int i = 0 ; i < NOMBRE_ELEMENTS_FUSEE ; i++) {
        pinMode(PINS_LED_ELEMENTS[i], OUTPUT);
    }

    if ( WITH_SOUND )
        pinMode(PIN_BUZZER, OUTPUT);


    pinMode(PIN_DETECTION_ETAGE_INFERIEUR, INPUT);

    pinMode(PIN_LAUNCH, OUTPUT);
    doLEDs(255);

    pinMode(PIN_BOUTON_SET_CODE, INPUT_PULLUP);
    timer.every(500, boutonAdminMemoriserCode);

    pinMode(PIN_NEYMAN, INPUT_PULLUP);
    timer.every(250, neymanCheck);

}

void loop() {

    // ------- fusée -------
    // hauteurReservoirCarburant();

    // connectionElementsFusee();

    // ------- mallette -------
    // changeChiffreCode();

    // voyantLaunch();

    // Serial.print(String(nbElementsConnectes == NOMBRE_ELEMENTS_FUSEE) + " " + reservoirPlein + " " + codesMatch + " " +readyToLaunch + " " + neymanActive + " " + (readyToLaunch && neymanActive) + "\n");

    // allume le fumi
    // pouf();

    timer.tick();
}