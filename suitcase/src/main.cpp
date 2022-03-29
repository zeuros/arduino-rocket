#include <TM1638plus.h>
#include <arduino-timer.h>
#include <EEPROM.h>

// init afficheur
#define STROBE_TM 2     // strobe = GPIO connected to strobe line of module
#define CLOCK_TM 3      // clock = GPIO connected to clock line of module
#define DIO_TM 4        // data = GPIO connected to data line of module
bool high_freq = false; // default false, If using a high freq CPU > ~100 MHZ set to true.

#define NOMBRE_ELEMENTS_FUSEE 4 // strobe = GPIO connected to strobe line of module

TM1638plus tm(STROBE_TM, CLOCK_TM, DIO_TM, high_freq);

auto timer = timer_create_default();

const int PINS_LED_ELEMENTS[NOMBRE_ELEMENTS_FUSEE] = {7, 8, 9, 10};

const int PIN_LAUNCH = A4; // Led affiche possibilité de lancement (=tout est activé)
const int PIN_BUZZER = 11;
const int PIN_BOUTON_SET_CODE = 10; // Pin Lecture analogique seulement !
const int PIN_NEYMAN = 12;// D12

const bool WITH_SOUND = false;

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



/* ------------------------------------ Fumigène ------------------------------------ */

void pouf () {
    bool launch = readyToLaunch && neymanActive;

    if ( launch && !launched) {
        launched = true;
    }

    // digitalWrite(PIN_FUMIGENE, launch ? HIGH : LOW);
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
    tm.displayBegin();

    // blingBling(); // lel
    secretCode = retrieveSecretCode();

    for ( int i = 0 ; i < NOMBRE_ELEMENTS_FUSEE ; i++) {
        pinMode(PINS_LED_ELEMENTS[i], OUTPUT);
    }

    if ( WITH_SOUND )
        pinMode(PIN_BUZZER, OUTPUT);


    pinMode(PIN_LAUNCH, OUTPUT);
    doLEDs(255);

    pinMode(PIN_BOUTON_SET_CODE, INPUT_PULLUP);
    timer.every(500, boutonAdminMemoriserCode);

    pinMode(PIN_NEYMAN, INPUT_PULLUP);
    timer.every(250, neymanCheck);

}

void loop() {

    changeChiffreCode();

    // voyantLaunch();

    // Serial.print(String(nbElementsConnectes == NOMBRE_ELEMENTS_FUSEE) + " " + reservoirPlein + " " + codesMatch + " " +readyToLaunch + " " + neymanActive + " " + (readyToLaunch && neymanActive) + "\n");

    // allume le fumi
    // pouf();

    timer.tick();
}