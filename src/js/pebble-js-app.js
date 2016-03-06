var openWeatherMapAPIKey = '90973b6bdeeed6bb35d37c628c5a987a';
var configureURL = 'https://countdays-config.bitballoon.com/';

var unsendTemperature = '';
var unsendCondition = '';
var unsendCountdays = '';
var unsendDreamday  = '';

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

  unsendTemperature = temperature + '\xB0C';
  unsendCondition = conditions;

  if(isAllDataArrived()) {
    sendAllAppMessage();
  }

}

function isAllDataArrived() {
  return unsendTemperature !== '' &&
  unsendCondition !== '' &&
  unsendCountdays !== '' &&
  unsendDreamday !== '';
}

function sendAllAppMessage() {

  var data = {
    'KEY_TEMPERATURE': unsendTemperature,
    'KEY_CONDITIONS': unsendCondition,
    'KEY_COUNTDAYS': unsendCountdays,
    'KEY_DREAMDAY': unsendDreamday
  };

  Pebble.sendAppMessage(data,
    function(e) {
      console.log('Data sent to Pebble successfully!');
      clearAllSentData();
    },
    function(e) {
      console.log('Error sending dat to Pebble!');
      clearAllSentData();
    }
  );

}

function clearAllSentData() {
  unsendTemperature = '';
  unsendCondition = '';
  unsendCountdays = '';
  unsendDreamday = '';
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
    'KEY_DREAMDAY': ' '
  });

}


Pebble.addEventListener('ready', function(e) {

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

  syncWithPhone(e.payload.KEY_DREAMDAY);
  console.log('Received message: ' + JSON.stringify(e.payload));

});

function setCountdaysFromWatchPayload(storedDreamdayPayload) {

  var dreamday = new Date(storedDreamdayPayload);
  var countdays = getCountDays(dreamday);
  sendCountdaysToWatch(countdays, storedDreamdayPayload);

}

Pebble.addEventListener('webviewclosed', function (e) {

  var configData = getConfigDataFromResponse(e.response);
  syncWithPhone(configData.dreamday);

  console.log(e.type + ', response= ' + e.response);

});

function syncWithPhone(dreamday) {

  getWeather();
  setCountdaysFromDreamdayRaw(dreamday);

}

function setCountdaysFromDreamdayRaw(dreamdayRaw){
  var dreamday = new Date(dreamdayRaw);
  var countdays = getCountDays(dreamday);

  sendCountdaysToWatch(countdays, dreamdayRaw);
}

function sendCountdaysToWatch(countdays, dreamday) {

  console.log("countdays = " + countdays);
  console.log("dreamday = " + dreamday);

  unsendCountdays = countdays + '';
  unsendDreamday = dreamday;

  if(isAllDataArrived()) {
    sendAllAppMessage();
  }

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
