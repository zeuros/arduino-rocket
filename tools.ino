

int sum (int* values) {
    int res = 0;
    for(int i=0; i<10; i++){
        res += values[i];
    }
    return res;
}

// ajoute une valeur au tableau de moyennes et retourne la moyenne.
int average(int* values, int newValue) {
    static int currentValue = 0;
    
    values[currentValue] = newValue;
    
    currentValue++;

    if ( currentValue > 9 ) {
        currentValue = 0;
    }

    return sum(values) / 10;
}

