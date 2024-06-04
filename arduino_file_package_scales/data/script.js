const weight = document.getElementById('weight').value;
const volume = document.getElementById('volume').value;
const weight_volume =  document.getElementById('weight_volume').value;

document.addEventListener('DOMContentLoaded', (event) => {
  fetch('/readings')
    .then(response => response.json())
    .then(data => {
      weight = data.weight;
      volume = data.volume;
      weight_volume = data.volume/6000;
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
      weight = myObj.weight;
      volume = myObj.volume;
      weight_volume = myObj.volume/6000;
    }, false);
  }

  document.getElementById('ongkirForm').addEventListener('submit', function(e) {
    e.preventDefault();

    const courier = document.getElementById('courier').value;
    const origin = document.getElementById('origin').value;
    const destination = document.getElementById('destination').value;
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
