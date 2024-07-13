function toggleRelay() {
  var xhr = new XMLHttpRequest();
  xhr.open('GET', 'http://192.168.129.223/toggle', true);
  xhr.onreadystatechange = function () {
    if (xhr.readyState === 4) { // Use strict equality
      if (xhr.status === 200) {
        var contentType = xhr.getResponseHeader("Content-Type");
        if (contentType && contentType.includes("text/plain")) {
          var relayState = xhr.responseText.trim(); // Trim the response
          console.log('API Response:', relayState); // Log the API response
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
      } else {
        console.error('Error fetching relay state:', xhr.statusText); // Error handling
      }
    }
  };
  xhr.onerror = function () {
    console.error('Request failed'); // Handle network errors
  };
  xhr.send();
}

function toggleMode() {
  var body = document.body;
  var modeButton = document.getElementById('modeButton');
  if (body.classList.contains('light-mode')) {
    body.classList.remove('light-mode');
    body.classList.add('dark-mode');
    modeButton.innerHTML = 'Switch to Light Mode';
    modeButton.className = 'btn btn-light mb-3';
  } else {
    body.classList.remove('dark-mode');
    body.classList.add('light-mode');
    modeButton.innerHTML = 'Switch to Dark Mode';
    modeButton.className = 'btn btn-secondary mb-3';
  }
}
