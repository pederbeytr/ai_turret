#include <Servo.h>

Servo servoX; // X axis servo
Servo servoY; // Y axis servo
Servo servoZ; // Z axis servo (Trigger)

int x;
int y;
int prevX;
int prevY;
bool faceDetected = false;
bool scanning = true; // Tarama modu kontrolü

unsigned long lastFaceCheckTime = 0;
const unsigned long faceCheckInterval = 500; // Yüz kontrolü aralığı (yarım saniye)

void setup()
{
  Serial.begin(9600);
  servoX.attach(10); // Attach X axis servo to pin 10
  servoY.attach(9);  // Attach Y axis servo to pin 9
  servoZ.attach(8);  // Attach Z axis servo to pin 8 (Trigger)
  servoX.write(90);
  servoY.write(90);
  servoZ.write(90);
}

void Pos()
{
  if (prevX != x || prevY != y)
  {
    int servoXVal = map(x, 600, 0, 70, 179);
    int servoYVal = map(y, 450, 0, 179, 95);

    servoXVal = min(servoXVal, 179);
    servoXVal = max(servoXVal, 70);
    servoYVal = min(servoYVal, 179);
    servoYVal = max(servoYVal, 95);

    servoX.write(servoXVal);
    servoY.write(servoYVal);
  }
}

void trackFace()
{
  int prevFaceX = -1;
  int prevFaceY = -1;
  bool faceLost = false;

  while (!faceLost)
  {
    if (Serial.available() > 0)
    {
      if (Serial.read() == 'X')
      {
        x = Serial.parseInt() + 50;
        if (Serial.read() == 'Y')
        {
          y = Serial.parseInt() - 100;
          Pos();
          prevFaceX = x;
          prevFaceY = y;
          if (!faceDetected)
          {
            servoZ.write(180); // Trigger on
            faceDetected = true;
          }
        }
        if (Serial.read() == 'Z')
        {
          if (x > 0 && y > 0)
          {
            if (!faceDetected)
            {
              servoZ.write(180); // Trigger on
              faceDetected = true;
              scanning = false; // Tarama modundan çık
            }
          }
          else
          {
            if (faceDetected)
            {
              servoZ.write(90); // Trigger off
              faceDetected = false;
              faceLost = true;
              servoX.write(90); // Reset X servo to center position
              scanning = true; // Tarama moduna geç
            }
          }
        }
      }
      while (Serial.available() > 0)
      {
        Serial.read();
      }
    }

    // Check if face has moved significantly in X axis
    if (prevFaceX != -1 && abs(x - prevFaceX) > 20)
    {
      servoX.write(90); // Reset X servo to center position
      break;
    }

    prevFaceX = x;
    prevFaceY = y;
  }
}

void scanMode()
{
  int angle = 0;
  int increment = 5;

  while (scanning)
  {
    if (Serial.available() > 0)
    {
      if (Serial.read() == 'X')
      {
        x = Serial.parseInt() + 50;
        if (Serial.read() == 'Y')
        {
          y = Serial.parseInt() - 100;
          Pos();
          if (x > 0 && y > 0)
          {
            scanning = false; // Tarama modundan çık
            servoZ.write(180); // Trigger on
            faceDetected = true;
            trackFace();
          }
        }
        if (Serial.read() == 'Z')
        {
          if (x > 0 && y > 0)
          {
            scanning = false; // Tarama modundan çık
            servoZ.write(180); // Trigger on
            faceDetected = true;
            trackFace();
          }
        }
      }
      while (Serial.available() > 0)
      {
        Serial.read();
      }
    }

    angle += increment;
    if (angle > 180 || angle < 0)
    {
      increment *= -1;
      angle += increment;
    }

    servoX.write(angle);
    delay(100);
  }
}

void loop()
{
  if (Serial.available() > 0)
  {
    if (Serial.read() == 'X')
    {
      x = Serial.parseInt() + 50;
      if (Serial.read() == 'Y')
      {
        y = Serial.parseInt() - 100;
        Pos();
        if (!faceDetected && scanning)
        {
          servoZ.write(180); // Trigger on
          faceDetected = true;
          scanning = false; // Tarama modundan çık
        }
      }
      if (Serial.read() == 'Z')
      {
        if (x > 0 && y > 0)
        {
          if (!faceDetected && scanning)
          {
            servoZ.write(180); // Trigger on
            faceDetected = true;
            scanning = false; // Tarama modundan çık
          }
        }
        else
        {
          if (faceDetected)
          {
            servoZ.write(90); // Trigger off
            faceDetected = false;
            servoX.write(90); // Reset X servo to center position
            if (!scanning)
            {
              scanning = true; // Tarama moduna geç
              scanMode();
            }
          }
        }
      }
    }
    while (Serial.available() > 0)
    {
      Serial.read();
    }
  }

  // Yüz kontrolü aralığını kontrol et
  unsigned long currentMillis = millis();
  if (currentMillis - lastFaceCheckTime >= faceCheckInterval)
  {
    lastFaceCheckTime = currentMillis;
    if (!faceDetected && scanning)
    {
      servoZ.write(90); // Trigger off
      faceDetected = false;
      servoX.write(90); // Reset X servo to center position
      scanMode();
    }
  }
}
