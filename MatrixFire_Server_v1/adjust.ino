

void adjustFlareRows(int direction){
	flareRows += direction;
	flareRows = constrain(flareRows,MinFlareRows,MaxFlareRows);
	// Serial.print(F("flareRows: "); Serial.println(flareRows));
}

void adjustFlareMax(int direction){
	flareMax += direction;
	flareMax = constrain(flareMax,MinFlares,MaxFlares);
	// Serial.print(F("flareMax: "); Serial.println(flareMax));
}

void adjustFlareChance(int direction){
	flareChance += direction;
	flareChance = constrain(flareChance,MinFlareChance,MaxFlareChance);
	// Serial.print(F("flareChance: "); Serial.println(flareChance));
}

void adjustFlareDecay(int direction){
	flareDecay += direction;
	flareDecay = constrain(flareDecay,MinFlareDecay,MaxFlareDecay);
	// Serial.print(F("flareDecay: "); Serial.println(flareDecay));
}


