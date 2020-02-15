

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
    tone(PIN_BUZZER, 1200, 40); 
}

