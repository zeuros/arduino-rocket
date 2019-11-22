
bool clearLEDs() {
    doLEDs(0);
    return false; // no repeat
}

void doLEDs(uint8_t value) {
    for (uint8_t position = 0; position < 8; position++) {
        tm.setLED(position, value & 1);
        value = value >> 1;
    }
}

void blingBling() {

    tm.reset();
    tm.displayText("UUALLLAH");
    
    for ( int i = 0 ; i < 7 ; i++ ) {
        for (uint8_t brightness = 5; brightness-- > 0 ;) {
            tm.brightness(brightness);
            delay(20);
        }
        for (uint8_t brightness = 0; brightness < 5; brightness++) {
            tm.brightness(brightness);
            delay(20);
        }
    }
}

// répète blinkLeds tq les codes matchent
void blinkLeds () {
    timer.every(500, [](void*) -> bool {

        timer.in(250, [](void*) -> bool {
            doLEDs(0);
            return false;
        });

        doLEDs(codesMatch ? 255 : 0);

        return codesMatch; 
    });
}
