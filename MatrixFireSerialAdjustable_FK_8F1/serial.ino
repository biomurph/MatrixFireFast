


void serialCheck(){
  if(Serial.available()){
    char inChar = Serial.read();
    switch(inChar){
      case 'r':

      break;
      case 'R':

      break;
      case 'm':

      break;
      case 'M':

      break;
      case 'c`':

      break;
      case 'C':

      break;
      case 'd':

      break;
      case 'D':

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

void printContro(){
  Serial.println("MakeFire Serial Control");
  Serial.println("Send 'r', 'R' to decrease, increase height of flareRows");
  Serial.println("Send 'm', 'M' to decrease, increase max number of flares");
  Serial.println("Send 'c', 'C' to decrease, increase the chance of a flare");
  Serial.println("Send 'd', 'D' to decrease, increase the decay of a flare");
  // Serial.println("");
  // Serial.println("");
  // Serial.println("");
  Serial.println("Send '?' to get this message");
}





