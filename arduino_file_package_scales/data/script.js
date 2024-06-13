const panjang = document.getElementById('length');
const lebar = document.getElementById('width');
const tinggi = document.getElementById('height');
const volume = document.getElementById('volume');
const weight = document.getElementById('weight');

document.addEventListener('DOMContentLoaded', (event) => {
  fetch('/readings')
    .then(response => response.json())
    .then(data => {
      panjang.textContent = data.panjang;
      lebar.textContent = data.lebar;
      tinggi.textContent = data.tinggi;
      volume.textContent = data.volume;
      weight.textContent = data.weight;
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
      panjang.textContent = myObj.panjang;
      lebar.textContent = myObj.lebar;
      tinggi.textContent = myObj.tinggi;
      volume.textContent = myObj.volume;
      weight.textContent = myObj.weight;
    }, false);
  }

  document.getElementById('printButton').addEventListener('click', () => {
    window.print();
  });

  function updateDateTime() {
    const now = new Date();
    const date = now.toLocaleDateString();
    
    const hours = now.getHours().toString().padStart(2, '0');
    const minutes = now.getMinutes().toString().padStart(2, '0');

    const time = `${hours}:${minutes}`;
    
    document.getElementById('date').textContent = date;
    document.getElementById('time').textContent = time;
  }

  // Memperbarui tanggal dan waktu setiap detik
  setInterval(updateDateTime, 1000);

  // Memperbarui tanggal dan waktu saat halaman dimuat
  updateDateTime();
});
