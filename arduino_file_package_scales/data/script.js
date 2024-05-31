document.getElementById('ongkirForm').addEventListener('submit', function (event) {
  event.preventDefault();

  const courier = document.getElementById('courier').value;
  const origin = document.getElementById('origin').value;
  const destination = document.getElementById('destination').value;
  const weight = document.getElementById('weight').value;

  const apiKey = '0959044ca5412704d19571e7a909c03f6a37111f1333d33e6aaa2bd3f4c12115';
  const url = `https://api.binderbyte.com/v1/cost?api_key=${apiKey}&courier=${courier}&origin=${origin}&destination=${destination}&weight=${weight}`;

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

    resultsDiv.appendChild(resultDiv);
  });
}

function displayError(error) {
  const resultsDiv = document.getElementById('results');
  resultsDiv.innerHTML = `<p style="color: red;">Error: ${error.message}</p>`;
}
