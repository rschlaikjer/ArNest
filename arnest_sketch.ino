#include <SPI.h>
#include <Ethernet.h>
#include <SFE_BMP180.h>
#include <Wire.h>

#define ALTITUDE 4.0 // Altitude Medford, MA

int relay_pin = 8;
SFE_BMP180 sensor;

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

EthernetClient client;

char server[] = "nest.rhye.org";

unsigned long lastConnectionTime = 0;           // last time we connected to the server, in millis
boolean lastConnected = false;                  // state of the connection last time through the main loop
const unsigned long postingInterval = 30*1000;  // delay between updates, in millis

float temp, pressure; // C, mbar

void setup() {
    // Wait for ethernet to init
    delay(1000);
    Ethernet.begin(mac);

    // Set up the pin controlling the heater relay as output
    pinMode(relay_pin, OUTPUT);

    // Start the temp/pressure sensor
    if (sensor.begin()){
    } else {
        while(1); // Busy halt
    }
}

/*
 * Main loop:
 * - If there's an HTTP response pending, parse it and update relay state
 * - Close the HTTP conn if we're done with it
 * - If it's been $polltime since the last connection, init a new one
 *     - Get the temp/pressure first
 */
void loop() {
    if (client.available()) {
        parse_resp();
    }

    // If there's no net connection, but there was one last time
    // through the loop, then stop the client:
    if (!client.connected() && lastConnected) {
        client.stop();
    }

    // if you're not connected, and ten seconds have passed since
    // your last connection, then connect again and send data:
    if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
        updateTemp();
        httpRequest();
    }
    // store the state of the connection for next time through
    // the loop:
    lastConnected = client.connected();
}

void parse_resp(){
    // Really cheap and dirty, pretty much just strcmp for burn-y or burn-n
    // in the response body. Not guaranteed to work with other people's httpds
    char msg_payload_len = 6;
    unsigned char post_nl_read = 0;
    char cmd[msg_payload_len];
    while (client.available()){
        char c = client.read();
        if (c == '\n' || c == '\r'){
space:
            post_nl_read = 0;
            while (client.available() && post_nl_read < msg_payload_len){
                cmd[post_nl_read] = client.read();
                if (cmd[post_nl_read] == '\n' || cmd[post_nl_read] == '\r'){
                    goto space;
                }
                post_nl_read++;
            }
            if (post_nl_read == 6){
                if (cmd[0] == 'b'
                        && cmd[1] == 'u'
                        && cmd[2] == 'r'
                        && cmd[3] == 'n'
                        && cmd[4] == '-'){
                    if (cmd[5] == 'y'){
                        // Turn on the heat
                        digitalWrite(relay_pin, HIGH);
                    } else if (cmd[5] == 'n'){
                        // Turn off the heat
                        digitalWrite(relay_pin, LOW);
                    }
                }
            }
        }
    }
}

void httpRequest() {
    // Sends the temp/pressure data to the server
    if (client.connect(server, 80)) {
        client.print("GET /nest.php?temp=");
        client.print(temp, 2);
        client.print("&pressure=");
        client.print(pressure, 2);
        client.println(" HTTP/1.1");
        client.println("Host: nest.rhye.org");
        client.println("User-Agent: arduino-ethernet");
        client.println("Connection: close");
        client.println();

        // note the time that the connection was made:
        lastConnectionTime = millis();
    }
    else {
        // Conn failed
        client.stop();
    }
}

void updateTemp(){
    char status;
    double t_celsius,P,p0,a;
    status = sensor.startTemperature();
    if (status != 0){
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed temperature measurement:
        // Note that the measurement is stored in the variable T.
        // Function returns 1 if successful, 0 if failure.
        status = sensor.getTemperature(t_celsius);
        if (status != 0){
            temp = t_celsius;

            // Start a pressure measurement:
            // The parameter is the oversampling setting, from 0 to 3
            // If request is successful, the number of ms to wait is returned.
            // If request is unsuccessful, 0 is returned.
            status = sensor.startPressure(3);
            if (status != 0){
                // Wait for the measurement to complete:
                delay(status);

                // Get the pressure (dependent on temperature)
                status = sensor.getPressure(P,t_celsius);
                if (status != 0){
                    // The pressure sensor returns abolute pressure, which varies with altitude.
                    // To remove the effects of altitude, use the sealevel function and your current altitude.
                    p0 = sensor.sealevel(P,ALTITUDE);
                    pressure = p0;

                }
            }
        }
    }
}
