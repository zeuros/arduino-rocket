
// ajoute une valeur au tableau de moyennes et retourne la moyenne des 10 dernières valeurs.
// évite les sautes de valeurs dues à des perturbations
int smooth(int avg, int newValue) {
    return ( avg * 9 + newValue ) / 10;
}

int sum(int* arr, int size) {
	int sum = 0;
	
	for(int i=0; i<size; i++){
	    sum += arr[i];
	}

	return sum;
}