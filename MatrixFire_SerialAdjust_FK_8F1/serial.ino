


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
  Serial.print("MatrixFire v"); Serial.println(VERSION);
  Serial.print(FPS); Serial.println(" FPS"); 
  Serial.println("Send 'r', 'R' to decrease, increase height of flareRows");
  Serial.println("Send 'm', 'M' to decrease, increase max number of flares");
  Serial.println("Send 'c', 'C' to decrease, increase the chance of a flare");
  Serial.println("Send 'd', 'D' to decrease, increase the decay of a flare");
  // Serial.println("");
  Serial.println("Send '?' to get this message");
  printFlariables();
}


void printFlariables(){
  Serial.println("Rows\tMax\tChance\tDecay");
  Serial.print(flareRows); Serial.print("\t");
  Serial.print(flareMax); Serial.print("\t");
  Serial.print(flareChance); Serial.print("\t");
  Serial.println(flareDecay);

}


