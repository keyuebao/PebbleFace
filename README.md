# PebbleFace

This is an educational tutorial I followed to get myself started with Pebble development for projects to come.
There is a lot of commenting as I went along, and I logged down what I learnt from compilation errors, such as scope of local variables and order of function declarations. I have also put alternative thoughts in some comment blocks for future reference on how to do things better.

The end product is a Pebble watchface with customized font and bitmap background, there is also an additional text layer which periodically pulls current temperature and weather condition from an outside API, by providing the user's current location. Below is a screenshot of how the watch face looks like:

![Pebble Watchface Screenshot](https://cloud.githubusercontent.com/assets/5943807/7824646/6d105d04-03b9-11e5-8151-f700912e9378.png)

Layout
---------
###main.c

* Where we display the visual layers (Text and Weather, watch screen/window)
* Update time/weather periodically on the watchface
* Place callbacks for when weather/location information is received
* Additional callbacks for any possible errors to occur
* Allocate/deallocate memory space after objects are created/destroyed

###weather.js

* Set up PebbleKitJS for interaction with the weather API using XMLHttpRequest, pushing use location
* Extract information such as temperature and weather condition from the pull request

###Resources

* True type font file for customized font, two different sizes
* Bitmap image for watchface background

License
-------
Pebble 
