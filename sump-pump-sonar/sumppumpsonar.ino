/* HC-SR04 Ping / Range finder wiring:
 * -----------------------------------
 * Particle - HC-SR04
 *      GND - GND
 *      VIN - VCC
 *       D2 - TRIG
 *       D6 - ECHO
 */

// Pins used by the Sonic sensor
pin_t trig_pin = D2;
pin_t echo_pin = D6;

// Depth of pump housing.
float PUMP_DEPTH = 23.5;

// Globally defined variables for Particle to use.
char pv_inches[8];
//char pv_cm[8];

void setup() {
    // Define particle variables for app to scrape via API
    Particle.variable("inch_distance", pv_inches);
    //Particle.variable("cm_distance", pv_cm);
    
    // Opens serial port and sets to fastest bits per second(baud)
    Serial.begin(115200);
    
    // Defines the Trigger and Echo pins for the HC-SR04
    pinMode(trig_pin, OUTPUT);
    pinMode(echo_pin, INPUT);
    
    // Begins with a LOW write to the Trigger pin
    // Eventual HIGH writes will be read for sonic duration
    digitalWriteFast(trig_pin, LOW);
}

void loop() {
    pingrounded();
    delay(20000);
}

void pingrounded()
{
    bool should_publish = true;
    /* Trigger the sensor by sending a HIGH pulse of 10 or more microseconds */
    digitalWriteFast(trig_pin, HIGH);
    delayMicroseconds(10);
    digitalWriteFast(trig_pin, LOW);
  
    float duration = pulseIn(echo_pin, HIGH);

    if (duration > 5500) should_publish = false;
    /* Convert the time into a distance */
    // Sound travels at 1130 ft/s (73.746 us/inch)
    // or 340 m/s (29 us/cm), out and back so divide by 2
    float inches = (duration / 2.0) / 73.746;
    //float cm = (duration / 2.0) / 29.0;
    
    // PUMP_DEPTH Diagram
    //  B----------23.5"----------Sensor would read 22", want 0"
    //  B---5.5"--W-------18"-----Sensor would read 18", want 5.5"
    
    // Subtract distance from PUMP_DEPTH to get depth of actual water
    inches = PUMP_DEPTH - inches;
    
    // Writes distances + duration out to serial monitor
    Serial.printlnf("%.3f in / %.3f us -- Publish: %s", inches, duration, should_publish ? "YES" : "NO");
    
    if (should_publish) {
        // Writes each distance value to the global Char array for Particle Variables
        sprintf(pv_inches, "%.3f", inches);
        //sprintf(pv_cm, "%.3f", cm);
        
        // Publishes each distance value to Google App Engine
        // POST http://home-metrics.appspot.com/webhook
        // https://console.particle.io/integrations/webhooks/5c2ce3a3d666d4734fa30700
        Particle.publish("pump_depth_inches", pv_inches, PRIVATE);
    }
}
