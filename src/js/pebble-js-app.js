var openWeatherMapAPIKey = '90973b6bdeeed6bb35d37c628c5a987a';
var configureURL = 'https://countdays-config.herokuapp.com/';

function getWeatherFromOpenWeatherMap(coordinates) {

  var url = getOpenWeatherMapRequestURL(coordinates);

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

function getOpenWeatherMapRequestURL(coordinates) {

  var latitude = coordinates.latitude;
  var longitude = coordinates.longitude;

  return 'http://api.openweathermap.org/data/2.5/weather?' +
    'lat=' + latitude + '&lon=' + longitude +
    '&appid=' + openWeatherMapAPIKey;

}

function weatherResponseHandler(responseText) {

  var response = JSON.parse(responseText);

  var temperature = Math.round(response.main.temp - 273.15);
  var conditions = response.weather[0].description;

  sendWeatherToWatch(temperature, conditions);

}

function sendWeatherToWatch(temperature, conditions) {

  console.log("sendWeatherToWatch: " + temperature + ", " + conditions);

  var data = {
    'KEY_TEMPERATURE': temperature + '\xB0C',
    'KEY_CONDITIONS': conditions,
    'KEY_COUNTDAYS': '',
    'KEY_COUNTDAYS_START_DATE': ''
  };

  Pebble.sendAppMessage(data,
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
  getWeatherFromOpenWeatherMap(coordinates);

}

function locationError(err) {

  console.log('Error requesting location!');
  console.warn('location error (' + err.code + '): ' + err.message);
  Pebble.sendAppMessage({
    'KEY_TEMPERATURE': ' ',
    'KEY_CONDITIONS': ' ',
    'KEY_COUNTDAYS': ' ',
    'KEY_COUNTDAYS_START_DATE': ' '
  });

}


Pebble.addEventListener('ready', function(e) {

  getWeather();
  console.log(e.type);

});

function getWeather() {

  window.navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    locationOptions
  );

}

var locationOptions = {
  'timeout': 15000,
  'maximumAge': 60000
};

Pebble.addEventListener('appmessage', function(e) {

  console.log('Received message: ' + JSON.stringify(e.payload));
  getWeather();

});

Pebble.addEventListener('webviewclosed', function (e) {

  setCountdaysFromResponse(e.response);
  console.log(e.type + ', response= ' + e.response);

});

function setCountdaysFromResponse(response) {

  var configData = getConfigDataFromResponse(response);

  var dreamday = new Date(configData.dreamday);
  var countdays = getCountDays(dreamday);

  sendCountdaysToWatch(countdays, configData.dreamday);

}

function sendCountdaysToWatch(countdays, countdays_start_date) {

  console.log("countdays = " + countdays);
  console.log("countdays_start_date = " + countdays_start_date);

  var data = {
    'KEY_TEMPERATURE': '',
    'KEY_CONDITIONS': '',
    'KEY_COUNTDAYS': countdays + '',
    'KEY_COUNTDAYS_START_DATE': countdays_start_date
  };

  Pebble.sendAppMessage(data,
    function(e) {
      console.log('Dream day sent to Pebble successfully!');
    },
    function(e) {
      console.log('Error sending dream day to Pebble!');
    }
  );

}

function getConfigDataFromResponse(response) {
  console.log('response: ' + response);
  return JSON.parse(decodeURIComponent(response));
}

function getCountDays(dreamday) {
    var today = new Date();
    var timeDiff = Math.abs(today.getTime() - dreamday.getTime());
    var diffDays = Math.floor(timeDiff / (1000 * 60 * 60 * 24));

    return diffDays;
}

Pebble.addEventListener('showConfiguration', function(e) {

  Pebble.openURL(configureURL);
  console.log(e.type);

});
