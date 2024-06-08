// Create Weight Gauge
var gaugeWeight = new RadialGauge({
  renderTo: 'gauge-weight',
  width: 150,
  height: 150,
  units: "Weight (Kg)",
  minValue: 0,
  maxValue: 20,
  colorValueBoxRect: "#007bff",
  colorValueBoxRectEnd: "#007bff",
  colorValueBoxBackground: "#cee6ff",
  valueInt: 2,
  majorTicks: [
      "0",
      "4",
      "8",
      "12",
      "16",
      "20"

  ],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
      {
          "from": 80,
          "to": 100,
          "color": "#4a9cf3"
      }
  ],
  colorPlate: "#fff",
  borderShadowWidth: 0,
  borders: false,
  needleType: "line",
  colorNeedle: "#0d57a7",
  colorNeedleEnd: "#0d57a7",
  needleWidth: 2,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#0d57a7",
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear"
}).draw();

// Create Volume Gauge
var gaugeVolume = new RadialGauge({
  renderTo: 'gauge-volume',
  width: 150,
  height: 150,
  units: "Volume (cm³)",
  minValue: 0,
  maxValue: 125000,
  colorValueBoxRect: "#007bff",
  colorValueBoxRectEnd: "#007bff",
  colorValueBoxBackground: "#cee6ff",
  valueInt: 2,
  majorTicks: [
      "0",
      "25000",
      "50000",
      "75000",
      "100000",
      "125000"

  ],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
      {
          "from": 80,
          "to": 100,
          "color": "#4a9cf3"
      }
  ],
  colorPlate: "#fff",
  borderShadowWidth: 0,
  borders: false,
  needleType: "line",
  colorNeedle: "#0d57a7",
  colorNeedleEnd: "#0d57a7",
  needleWidth: 2,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#0d57a7",
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear"
}).draw();

document.addEventListener('DOMContentLoaded', (event) => {
  fetch('/readings')
    .then(response => response.json())
    .then(data => {
      gaugeWeight.value = data.weight;
      gaugeVolume.value = data.volume;
    });

  if (!!window.EventSource) {
    var source = new EventSource('/events');
    
    source.addEventListener('open', function(e) {
      console.log("Events Connected");
    }, false);
  
    source.addEventListener('error', function(e) {
      if (e.target.readyState != EventSource.OPEN) {
        console.log("Events Disconnected");
      }
    }, false);
    
    source.addEventListener('message', function(e) {
      console.log("message", e.data);
    }, false);
    
    source.addEventListener('new_readings', function(e) {
      console.log("new_readings", e.data);
      var myObj = JSON.parse(e.data);
      console.log(myObj);
      gaugeWeight.value = myObj.weight;
      gaugeVolume.value = myObj.volume;
    }, false);
  }

  document.getElementById('printButton').addEventListener('click', () => {
    window.print();
  });
});
