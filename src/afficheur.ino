
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

// 
void blingBling() {

    tm.reset();
    tm.displayText("HOUAHAHA");
    
    int speed = 3;// + = +lent
    int steps = 18;// + = +fluide & +lent

    for ( int i = 0 ; i < 7 ; i++ ) {
        // decroissant
        for (uint8_t brightness = 5; brightness-- > 1 ;) {
            //sub steps
            for(int j=0; j<steps; j++){
                tm.brightness(brightness);
                delay((speed * (steps-j)) / steps);
                tm.brightness(brightness-1);
                delay((speed * j) / steps);
            }
        }
        for (uint8_t brightness = 0; brightness < 5; brightness++) {
            //sub steps
            for(int j=0; j<steps; j++){
                tm.brightness(brightness);
                delay((speed * (steps-j)) / steps);
                tm.brightness(brightness+1);
                delay((speed * j) / steps);
            }
        }
    }

    tm.brightness(3);
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
