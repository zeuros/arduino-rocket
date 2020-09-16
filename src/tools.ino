
// ajoute une valeur au tableau de moyennes et retourne la moyenne des 10 dernières valeurs.
// évite les sautes de valeurs dues à des perturbations
int smooth(int avg, int newValue, int amount = 20) {
    return ( avg * (amount - 1) + newValue ) / amount;
}

int sum(int* arr, int size) {
	int sum = 0;
	
	for(int i=0; i<size; i++){
	    sum += arr[i];
	}

	return sum;
}