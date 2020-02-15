#include <Servo.h>
#include <CapacitiveSensor.h>

CapacitiveSensor   cs_12_13 = CapacitiveSensor(12, 13); //multiple capacitive touch sensors slowed down the arduino too much and caused the face tracking to desync
//output, input

Servo headServo;
Servo tailServo;

boolean faceDetected = false;

int ledRed = 7;//digital
int ledYellow = 6;//analog
int ledGreen = 5;//analog

int speakerPin = 4;

int RGBredPin = A0;
int RGBgreenPin = A4;
int RGBbluePin = A5;

float happiness = 0;
int freq = 750;//speaker frequency
int touch = 0;//capsense reading

int timeSinceFaceDetected = 0;

void setup() {
  headServo.attach(10);
  tailServo.attach(9);

  cs_12_13.set_CS_AutocaL_Millis(0xFFFFFFFF);//autocalibrate

  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);

  pinMode(RGBredPin, OUTPUT);
  pinMode(RGBgreenPin, OUTPUT);
  pinMode(RGBbluePin, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  unsigned long int now = millis();

  if (Serial.available()) {//max only sends data if face is detected
    faceDetected = true;

    RGBColor(0, happiness, 128);//blue eyes = detected face, green is amount of happiness

    int coords = Serial.read();
    //Serial.println(coords);

    coords = constrain (coords, 0, 180);//in case max sends a number out of range
    headServo.write(coords);

    timeSinceFaceDetected = now;
  }
  else//face not found
  {
    if (now - timeSinceFaceDetected >= 1001)//small delay in case face is lost for a fraction of a second
    {
      faceDetected = false;
      RGBColor((millis() / 250 % 2) * 255, 0, 0);//flash red eye leds when no eye contact
    }
  }

  touch = cs_12_13.capacitiveSensor(30);
  touch = constrain(touch, 0, 1000);
  touch = map(touch, 0, 1000, 0, 255);
  //touch was previously numOfTouches when we had multiple capacitive touch sensors

  if (faceDetected == true || touch > 101)//if eye contact or being touched (if given attention)
  {
    analogWrite (ledYellow, touch);//yellow brighter when your hand is closer
    happiness += .1 * (((touch * 0.01) + faceDetected)); //add to happiness when making eye contact
    happiness = constrain(happiness, 0, 255);
  }
  else//no attention received
  {
    happiness -= 0.5;//drains happiness
    happiness = constrain(happiness, 0, 255);
    digitalWrite(ledYellow, millis() / 250 % 2);//flash yellow leds
    //yellow flashes when not being touched or being made eye contact with,
    //pre warning to beeping and red led before happiness runs out
  }

  if (happiness < 1)//if unhappy
  {
    digitalWrite(ledRed, millis() / 250 % 2);//flash red leds

    tone(speakerPin, (millis() / 250 % 2) * 1500);//alarm sound
  }
  else//if happy
  {
    digitalWrite(ledRed, 0);//red off

    freq += random(happiness);
    freq -= random(127);
    if (freq >= 3050)
      freq -= 500;
    if (freq <= 99)
      freq += 500;
    //makeshift drunk object from max, randomly goes up and down, starts off at a hum but rises in pitch to more of a whistle as happiness rises

    tone(speakerPin, freq, 500);

    if (millis() / 333 % 2)
    {
      tailServo.write(map(happiness, 0, 255, 0, 90));
    }
    else
    {
      tailServo.write(-map(happiness, 0, 255, 0, 90));
    }
    //moves tail back and forth, moves more with more happiness
  }

  analogWrite (ledGreen, happiness);//green brighter with more happiness

  delay (10 );
}

void RGBColor(int r, int g, int b) {
  analogWrite(RGBredPin, r);
  analogWrite(RGBgreenPin, g);
  analogWrite(RGBbluePin, b);
}

