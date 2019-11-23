
void successTone() {
    tone(PIN_BUZZER, 3000, 5000);
}

void failTones() {
//  failTone();
    // faire le second bip après 200ms
  timer.in(200, failTone);
}

void failTone(void *) {
    tone(PIN_BUZZER, 1000, 400);
}
