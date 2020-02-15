
void saveSecretCode(char* theCode) {
    char toStore[9];
    strcpy(toStore, theCode);

    EEPROM.put(100, toStore);

    strcpy(secretCode, theCode);

    weHaveANewCode(theCode);
}

char* retrieveSecretCode() {
    static char stored[9];

    EEPROM.get(100, stored);
    
    return stored;
}

// affiche le code, vérifie s'il est juste
void weHaveANewCode(char* newCode) {
    tm.displayText(newCode);

    codesMatch = String(code).equals(secretCode);

    codesMatch 
        ? successTone() 
        : notificationTone();

    if ( codesMatch ) {
        blinkLeds();
    }
}

void ajouteChiffre(uint8_t value, char* theCode) {
    for (uint8_t position = 0; position < 8; position++) {
        buttonPressed = (value & 1);

        if ( buttonPressed && !buttonHold[position] ) {

            // éteindre la led 
            theCode[position]++;

            if ( theCode[position] > ('9') ) {
                theCode[position] = '0';
            }

            buttonHold[position] = true;

            // affiche le bouton pressé
            doLEDs(1<<position);

            weHaveANewCode(theCode);
        }

        buttonHold[position] = buttonPressed;

        value = value >> 1;
    }
}

