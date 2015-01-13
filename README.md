# ArNest
Client implementation for a simple Arduino nest workalike

[Server Implementation](https://github.com/rschlaikjer/GoNest)

## What is this
This is the client code for an Arduino that I'm using as a makeshift Nest for my
apartement. Most of the logic happens in the server, to avoid having to reflash
the arduino ever time I change something.

Essentially, all this code does is:
- Grab a pair of temperature and pressure values
- Make a get request to the server with the temperature and pressure as args
- Parse the response for burn-y or burn-n
- Turn on/off the heater

## Parts
- [Arduino with ethernet/PoE](http://amzn.com/B005EJMQ9U)
- [Adafruit BMP180](http://amzn.com/B00JHJB2V6) pressure & temperature sensor
- [Decently sized relay](http://amzn.com/B00C8R6HWS)
- [I2C Display](http://amzn.com/B0080DYTZQ)
- Acrylic (for mounting)

![Finished Device](/arnest.jpg?raw=true "Device")
