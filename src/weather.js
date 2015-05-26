// Send XMLHttpRequest object to request for weather
// url: API, type: GET/POST, callback: for when response is received
var xhrRequest = function(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

// Use as callbacks for success/failure after requesting user location
function locationSuccess(pos) {
  // Construct URL from API
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
      pos.coords.latitude + '&lon=' + pos.coords.longitude;
  
  // Send request to API
  xhrRequest(url, 'GET',
    function(responseText) {
      // responseText contains a JSON object with weather information
      var json = JSON.parse(responseText);
      
      // LEARNT: Possible to discover structure of JSON object by using
      // console.log(JSON.parse(responseText)) nice thing to try
      
      // Temperature in Kelvin requires adjustment
      var temperature = Math.round(json.main.temp - 273.15);
      console.log('Temperature is ' + temperature);
      
      // Conditions
      var conditions = json.weather[0].main;
      console.log('Conditions are ' + conditions);
      
      // Assmeble dictionary using keys from main.c file
      var dictionary = {
        'KEY_TEMPERATURE': temperature,
        'KEY_CONDITIONS': conditions
      };

      // Send data to Pebble watch
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log('Weather info sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending weather info to Pebble!');
        }
      );
    }
  );
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when JS is ready after watchface is opened
Pebble.addEventListener('ready',
  function(e) {
    console.log('PebbleKit JS ready!');
    
    // When ready, calls to get current position 
    getWeather();
  }
);

// Listen for when AppMessage is sent, used for updates later
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
  }
);