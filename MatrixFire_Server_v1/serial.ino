


void serialCheck(){
  if(Serial.available()){
    char inChar = Serial.read();
    switch(inChar){
      case 'r':
        adjustFlareRows(-1);
        printFlariables();
      break;
      case 'R':
        adjustFlareRows(1);
        printFlariables();
      break;
      case 'm':
        adjustFlareMax(-1);
        printFlariables();
      break;
      case 'M':
        adjustFlareMax(1);
        printFlariables();
      break;
      case 'c':
        adjustFlareChance(-1);
        printFlariables();
      break;
      case 'C':
        adjustFlareChance(1);
        printFlariables();
      break;
      case 'd':
        adjustFlareDecay(-1);
        printFlariables();
      break;
      case 'D':
        adjustFlareDecay(1);
        printFlariables();
      break;
      case '?':
        printControl();
        printFlariables();
      break;
      default:
        Serial.print("i got: "); Serial.println(inChar);
      break;
    }
  }
}

void printControl(){
  Serial.println(versionString);
  Serial.print(FPS); Serial.println(" FPS"); 
  Serial.println(F("Send 'r', 'R' to decrease, increase height of flareRows"));
  Serial.println(F("Send 'm', 'M' to decrease, increase max number of flares"));
  Serial.println(F("Send 'c', 'C' to decrease, increase the chance of a flare"));
  Serial.println(F("Send 'd', 'D' to decrease, increase the decay of a flare"));
  // Serial.println("");
  
  Serial.println(F("Send '?' to get this message"));
  printFlariables();
}


void printFlariables(){
  Serial.println(F("Rows\tMax\tChance\tDecay"));
  Serial.print(flareRows); Serial.write('\t');
  Serial.print(flareMax); Serial.write('\t');
  Serial.print(flareChance); Serial.write('\t');
  Serial.println(flareDecay);

}


