

void makeABuzz(bool condifion, int freq, int duration) {
    if ( condifion ) {
        if ( !buzzing ) {
            buzzing = true;
            tone(PIN_BUZZER, freq, duration);
        }
    } else if ( buzzing ) {
        noTone(PIN_BUZZER);
        buzzing = false;
    }
}

void successTone() {
    tone(PIN_BUZZER, 4000, 600);
}

void notificationTone() {
    for(int i=0; i<5; i++){
        tone(PIN_BUZZER, 1200 + i * 100, 40);
        delay(10);
    }
}

void boom() {
    for(int i=0; i<100; i++){
        tone(PIN_BUZZER, 1600 + i * 100, 40);
        delay(10);
    }
}

