#include "pitches.h"

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

void bouitBouit() {
    for(int i=0; i<5; i++){
        tone(PIN_BUZZER, 1200 + i * 100, 40);
        delay(10);
    }
}

void tuding(int min) {
    tone(PIN_BUZZER, min, 40);
    delay(50);
    tone(PIN_BUZZER, min+800, 40);
}

void tudiiWindows() {
    tone(PIN_BUZZER, 300);
    delay(250);
    tone(PIN_BUZZER, 550);
    delay(150);
    noTone(PIN_BUZZER);
}


void notificationTone() {
    tone(PIN_BUZZER, 1200, 40);
}

void boom() {
    for(int i=0; i<100; i++){
        tone(PIN_BUZZER, 1600 + i * 100, 40);
        delay(10);
    }
}


void neverGiveYouUp() {
    int tempo = 130;

    // change this to whichever pin you want to use
    int buzzer = PIN_BUZZER;

    int melody[] = {
        NOTE_A4,16, NOTE_B4,16, NOTE_D5,16, NOTE_B4,16,
        NOTE_FS5,-8, NOTE_FS5,-8, NOTE_E5,-4, NOTE_A4,16, NOTE_B4,16, NOTE_D5,16, NOTE_B4,16,

        NOTE_E5,-8, NOTE_E5,-8, NOTE_D5,-8, NOTE_CS5,16, NOTE_B4,-8, NOTE_A4,16, NOTE_B4,16, NOTE_D5,16, NOTE_B4,16, //18
        NOTE_D5,4, NOTE_E5,8, NOTE_CS5,-8, NOTE_B4,16, NOTE_A4,8, NOTE_A4,8, NOTE_A4,8, 
        NOTE_E5,4, NOTE_D5,2, NOTE_A4,16, NOTE_B4,16, NOTE_D5,16, NOTE_B4,16,
        NOTE_FS5,-8, NOTE_FS5,-8, NOTE_E5,-4, NOTE_A4,16, NOTE_B4,16, NOTE_D5,16, NOTE_B4,16,
        NOTE_A5,4, NOTE_CS5,8, NOTE_D5,-8, NOTE_CS5,16, NOTE_B4,8, NOTE_A4,16, NOTE_B4,16, NOTE_D5,16, NOTE_B4,16,

        NOTE_D5,4, NOTE_E5,8, NOTE_CS5,-8, NOTE_B4,16, NOTE_A4,4, NOTE_A4,8,  //23
        NOTE_E5,4, NOTE_D5,2, REST,4
    };

    // sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
    // there are two values per note (pitch and duration), so for each note there are four bytes
    int notes = sizeof(melody) / sizeof(melody[0]) / 2;

    // this calculates the duration of a whole note in ms
    int wholenote = (60000 * 4) / tempo;

    int divider = 0, noteDuration = 0;
    for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

        // calculates the duration of each note
        divider = melody[thisNote + 1];
        if (divider > 0) {
          // regular note, just proceed
          noteDuration = (wholenote) / divider;
        } else if (divider < 0) {
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
