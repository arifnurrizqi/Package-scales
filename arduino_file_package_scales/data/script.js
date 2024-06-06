const weight = document.getElementById('weight');
const volume = document.getElementById('volume');

// Create Weight Gauge
var gaugeWeight = new RadialGauge({
  renderTo: 'gauge-weight',
  width: 300,
  height: 300,
  units: "Weight (Kg)",
  minValue: 0,
  maxValue: 20,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
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
          "color": "#03C0C1"
      }
  ],
  colorPlate: "#fff",
  borderShadowWidth: 0,
  borders: false,
  needleType: "line",
  colorNeedle: "#007F80",
  colorNeedleEnd: "#007F80",
  needleWidth: 2,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#007F80",
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear"
}).draw();

// Create Volume Gauge
var gaugeVolume = new RadialGauge({
  renderTo: 'gauge-volume',
  width: 300,
  height: 300,
  units: "Volume (cm³)",
  minValue: 0,
  maxValue: 125000,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
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
          "color": "#03C0C1"
      }
  ],
  colorPlate: "#fff",
  borderShadowWidth: 0,
  borders: false,
  needleType: "line",
  colorNeedle: "#007F80",
  colorNeedleEnd: "#007F80",
  needleWidth: 2,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#007F80",
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear"
}).draw();

document.addEventListener('DOMContentLoaded', (event) => {
  fetch('/readings')
    .then(response => response.json())
    .then(data => {
      weight.value = data.weight;
      volume.value = data.volume;

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
      weight.value = myObj.weight;
      volume.value = myObj.volume;

      gaugeWeight.value = myObj.weight;
      gaugeVolume.value = myObj.volume;
    }, false);
  }

  document.getElementById('ongkirForm').addEventListener('submit', function(e) {
    e.preventDefault();

    const courier = document.getElementById('courier').value;
    const origin = document.getElementById('origin').value;
    const destination = document.getElementById('destination').value;
    const weight = document.getElementById('weight').value;
    let last_weight;

    if(weight < weight_volume) {
      last_weight = weight_volume;
    } else {
      last_weight = weight;
    }

    const apiKey = '0959044ca5412704d19571e7a909c03f6a37111f1333d33e6aaa2bd3f4c12115';
    const url = `https://api.binderbyte.com/v1/cost?api_key=${apiKey}&courier=${courier}&origin=${origin}&destination=${destination}&weight=${last_weight}`;

    console.log(url);

    fetch(url)
      .then(response => {
        if (!response.ok) {
          throw new Error(`Server responded with status: ${response.status}`);
        }
        return response.json();
      })
      .then(data => {
        if (data.status === 200) {
          displayResults(data);
        } else {
          throw new Error(`API error: ${data.message}`);
        }
      })
      .catch(error => {
        displayError(error);
      });
  });
});

function displayResults(data) {
  const resultsDiv = document.getElementById('results');
  resultsDiv.innerHTML = '';

  const costs = data.data.costs;

  costs.forEach(cost => {
    const resultDiv = document.createElement('div');
    resultDiv.classList.add('result');

    const service = document.createElement('p');
    service.textContent = `Service: ${cost.service}`;
    resultDiv.appendChild(service);

    const price = document.createElement('p');
    price.textContent = `Price: ${cost.price}`;
    resultDiv.appendChild(price);

    const line = document.createElement('hr');
    resultDiv.appendChild(line);

    resultsDiv.appendChild(resultDiv);
  });
}

function displayError(error) {
  const resultsDiv = document.getElementById('results');
  resultsDiv.innerHTML = `<p style="color: red;">Error: ${error.message}</p>`;
}
