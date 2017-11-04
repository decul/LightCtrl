const int pins[4] = { 11, 10, 9, 3 };

int values[4] = { 0, 0, 0, 0 };



void updateOutput();



void setup() {
    Serial.begin(9600);
	for (int l = 0; l < 4; l++)
		pinMode(pins[l], OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
}



void loop() {
    
}



void serialEvent() {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    char c = Serial.read();
    Serial.write(c);

    delay(10);

    switch (c) {
        case 'r':
            char rgbw[4];
            for (int l = 0; l < 4; l++) {
                if (Serial.available())
                    values[l] = map(Serial.read(), '0', '9', 0, 255);
                else
                    values[l] = 0;
                Serial.write(map(values[l], 0, 255, '0', '9'));
            }
            Serial.write('\n');
            updateOutput();
            break;
    }

    while (Serial.available())
        Serial.read();
}



void updateOutput() {
	for (int l = 0; l < 4; l++)
		analogWrite(pins[l], values[l]);
}
