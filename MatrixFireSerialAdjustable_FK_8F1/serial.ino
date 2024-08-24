


void serialCheck(){
  if(Serial.available()){
    char inChar = Serial.read();
    switch(inChar){
      case 'r':
        adjustFlareRows(-1);
      break;
      case 'R':
        adjustFlareRows(1);
      break;
      case 'm':
        adjustFlareMax(-1);
      break;
      case 'M':
        adjustFlareMax(1);
      break;
      case 'c`':
        adjustFlareChance(-1);
      break;
      case 'C':
        adjustFlareChance(1);
      break;
      case 'd':
        adjustFlareDecay(-1);
      break;
      case 'D':
        adjustFlareDecay(1);
      break;
      case '?':
        printControl();
      break;
      default:
        Serial.print("i got, "); Serial.println(inChar);
      break;
    }
  }
}

void printControl(){
  Serial.println("MakeFire Serial Control");
  Serial.println("Send 'r', 'R' to decrease, increase height of flareRows");
  Serial.println("Send 'm', 'M' to decrease, increase max number of flares");
  Serial.println("Send 'c', 'C' to decrease, increase the chance of a flare");
  Serial.println("Send 'd', 'D' to decrease, increase the decay of a flare");
  // Serial.println("");
  Serial.println("Send '?' to get this message");
}





