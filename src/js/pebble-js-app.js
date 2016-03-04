var myAPIKey = "90973b6bdeeed6bb35d37c628c5a987a";


function fetchWeather(latitude, longitude) {

  var url = 'http://api.openweathermap.org/data/2.5/weather?' +
    'lat=' + latitude + '&lon=' + longitude + '&appid=' + myAPIKey;

  var req = new XMLHttpRequest();
  req.open('GET', url, true);
  req.onload = function() {
    if (req.readyState === 4) {
      if (req.status === 200) {
        console.log(req.responseText);
        weatherResponseHandler(req.responseText);
      } else {
        console.log('error');
      }
    }
  };
  req.send(null);

}

function weatherResponseHandler(responseText) {

  var response = JSON.parse(responseText);

  var temperature = Math.round(response.main.temp - 273.15);
  console.log('Temperature is ' + temperature);

  var conditions = response.weather[0].description;
  console.log('Conditions are ' + conditions);

  var dictionary = {
    'KEY_TEMPERATURE': temperature + '\xB0C',
    'KEY_CONDITIONS': conditions
  };

  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log('Weather info sent to Pebble successfully!');
    },
    function(e) {
      console.log('Error sending weather info to Pebble!');
    }
  );

}

function locationSuccess(pos) {

  var coordinates = pos.coords;
  fetchWeather(coordinates.latitude, coordinates.longitude);

}


function locationError(err) {
  console.log('Error requesting location!');
  console.warn('location error (' + err.code + '): ' + err.message);
  Pebble.sendAppMessage({
    'KEY_TEMPERATURE': '-',
    'KEY_CONDITIONS': '-'
  });
}

function getCountDays() {

    var startDay = new Date("5/13/1993"); // TODO: should get from the user
    var today = new Date();
    var timeDiff = Math.abs(today.getTime() - startDay.getTime());
    var diffDays = Math.floor(timeDiff / (1000 * 60 * 60 * 24));

    return diffDays;

}

var locationOptions = {
  'timeout': 15000,
  'maximumAge': 60000
};

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');
  console.log(e.ready);
  window.navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    locationOptions
  );
  console.log(e.type);
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('AppMessage received!');
  console.log(e.type);
  console.log('message!');
});

Pebble.addEventListener('webviewclosed', function (e) {
  console.log('Webview closed!');
  console.log(e.type);
  console.log(e.response);
});
