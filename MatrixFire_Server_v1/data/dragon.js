var gateway = `ws://${window.location.hostname}/ws`; var websocket;
window.addEventListener('load', onload);
function  onload(event) {
  initWebSocket();
  initButton();
  // getCurrentValue();
}
function initWebSocket() {
  console.log('Trying to open a WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen = onOpen;
  websocket.onclose = onClose;
  websocket.onmessage = onMessage;
}
function onOpen(event) {
  console.log('Connection opened');
  websocket.send("getFlariables");
}
function onClose(event) {
  console.log('Connection closed');
  setTimeout(initWebSocket, 2000);
}
function onMessage(event) {
  var kindling = JSON.parse(event.data);
  console.log(kindling);
  var fireState = kindling.isBurning;
  var numFlareRows = kindling.flareRows;
  var numFlareMax = kindling.flareMax;
  var chanceOfFlare = kindling.flareChance;
  var decayOfFlare = kindling.flareDecay;
  var versionString = kindling.meta;
  console.log(fireState);
  console.log(numFlareRows);
  console.log(numFlareMax);
  console.log(chanceOfFlare);
  console.log(decayOfFlare);
  console.log(versionString);
  if (fireState == "1"){
    document.getElementById("STATEs").innerHTML = "Alive";
  } else {
    document.getElementById("STATEs").innerHTML = "Dead";
  }
  document.getElementById("flareRows").innerHTML = numFlareRows;
  document.getElementById("flareRowSlider").value = numFlareRows;
  document.getElementById("flareMax").innerHTML = numFlareMax;
  document.getElementById("flareMaxSlider").value = numFlareMax;
  document.getElementById("flareChance").innerHTML = chanceOfFlare;
  document.getElementById("flareChanceSlider").value = chanceOfFlare;
  document.getElementById("flareDecay").innerHTML = decayOfFlare;
  document.getElementById("flareDecaySlider").value = decayOfFlare;
  // document.getElementById("meta").innerHTML = versionString;
  console.log(event.data);
}

// function getCurrentValue() {
// var xhr = new XMLHttpRequest(); xhr.onreadystatechange = function() {
//   if (this.readyState == 4 && this.status == 200) {
//     document.getElementById("STATEs").innerHTML = "Dead";
//     document.getElementById("flareRowSlider").value = this.responseText;
//     document.getElementById("flareRows").innerHTML = this.responseText;

//     document.getElementById("flareMaxSlider").value = this.responseText;
//     document.getElementById("flareMax").innerHTML = this.responseText;

//     document.getElementById("flareChanceSlider").value = this.responseText;
//     document.getElementById("flareChance").innerHTML = this.responseText;

//     document.getElementById("flareDelaySlider").value = this.responseText;
//     document.getElementById("flareDecay").innerHTML = this.responseText;
//   } 
// };
// xhr.open("GET", "/getFlariables", true);
//   xhr.send();
// }

function updateSliderFlareRows(element) {
  var flareRowValue = document.getElementById("flareRowSlider").value;
  document.getElementById("flareRows").innerHTML = flareRowValue;
  console.log(flareRowValue);
  websocket.send(getOurNewValues());
}

function updateSliderFlareMax(element) {
  var flareMaxValue = document.getElementById("flareMaxSlider").value;
  document.getElementById("flareMax").innerHTML = flareMaxValue;
  console.log(flareMaxValue);
  websocket.send(getOurNewValues());
}

function updateSliderFlareChance(element) {
  var flareChanceValue = document.getElementById("flareChanceSlider").value;
  document.getElementById("flareChance").innerHTML = flareChanceValue;
  console.log(flareChanceValue);
  websocket.send(getOurNewValues());
}

function updateSliderFlareDecay(element) {
  var flareDecayValue = document.getElementById("flareDecaySlider").value;
  document.getElementById("flareDecay").innerHTML = flareDecayValue;
  console.log(flareDecayValue);
  websocket.send(getOurNewValues());
}

function initButton() {
  document.getElementById('bBURN').addEventListener('click', toggleON);
  document.getElementById('bDIE').addEventListener('click', toggleOFF);
}

function toggleON(event){
  console.log("BURN");
  document.getElementById("STATEs").innerHTML = "Alive";
  websocket.send(getOurNewValues());
}

function toggleOFF(event){
  console.log("DIE");
  document.getElementById("STATEs").innerHTML = "Dead";
  websocket.send(getOurNewValues());
}

function getOurNewValues(){
  var fireState = document.getElementById("STATEs").innerHTML;
  var fireRows = document.getElementById("flareRows").innerHTML;
  var fireMax = document.getElementById("flareMax").innerHTML;
  var fireChance = document.getElementById("flareChance").innerHTML;
  var fireDecay = document.getElementById("flareDecay").innerHTML;

  let flariables = {
    isBurning: fireState,
    flareRows: fireRows,
    flareMax: fireMax,
    flareChance: fireChance,
    flareDecay: fireDecay,
  };

  var ourStates = JSON.stringify(flariables);
  console.log(ourStates);
  return ourStates;
}