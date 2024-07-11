function toggleRelay() {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', 'http://192.168.129.223/toggle', true);  // Replace <ESP32_IP_ADDRESS> with the actual IP address of your ESP32
    xhr.onreadystatechange = function () {
      if (xhr.readyState == 4 && xhr.status == 200) {
        var relayState = xhr.responseText;
        document.getElementById('relayState').innerHTML = relayState;
        var toggleButton = document.getElementById('toggleButton');
        if (relayState === 'ON') {
          toggleButton.innerHTML = 'Turn OFF';
          toggleButton.className = 'btn btn-danger';
        } else {
          toggleButton.innerHTML = 'Turn ON';
          toggleButton.className = 'btn btn-primary';
        }
      }
    }
    xhr.send();
  }
  