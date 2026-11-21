#include <Arduino.h>
#include <Mouse.h>

// ---- tuning ----
const uint32_t IDLE_MIN_MS = 15000;   // keep below your lock timeout
const uint32_t IDLE_MAX_MS = 45000;
const int      DIST_MIN    = 8;       // device counts (~px before OS accel)
const int      DIST_MAX    = 60;
const int      STEP_MS     = 8;       // ~125 Hz, typical mouse poll rate
long netX = 0, netY = 0;              // cumulative drift from start

// Minimum-jerk profile: smooth accel/decel, 0..1 -> 0..1
static float minJerk(float t) 
{ 

  return t * t * t * (10 + t * (-15 + 6 * t)); 

}

void glide(int dx, int dy)
{

  float dist  = sqrtf((float)dx * dx + (float)dy * dy);

  int   durMs = 150 + (int)(dist * random(4, 9)) + random(0, 120);

  int   steps = max(3, durMs / STEP_MS);

  // Quadratic Bezier; control point offset perpendicular to the path = arc
  float bend = random(-30, 31) / 100.0f;

  float cx = dx / 2.0f - dy * bend;

  float cy = dy / 2.0f + dx * bend;

  int sentX = 0, sentY = 0;

  for (int i = 1; i < steps; i++) 
  {

    float s = minJerk((float)i / steps), u = 1 - s;

    float px = 2 * u * s * cx + s * s * dx + random(-5, 6) / 10.0f; // + tremor

    float py = 2 * u * s * cy + s * s * dy + random(-5, 6) / 10.0f;

    int mx = (int)lroundf(px) - sentX;

    int my = (int)lroundf(py) - sentY;

    if (mx || my) 
    { 
      Mouse.move(mx, my, 0); sentX += mx; sentY += my; 
    }

    delay(STEP_MS + random(-2, 3));

  }

  Mouse.move(dx - sentX, dy - sentY, 0);  // land exactly on target

  netX += dx; netY += dy;

}

void pickTarget(int &dx, int &dy)
{

  float d = random(DIST_MIN, DIST_MAX + 1);

  float a = random(0, 360) * DEG_TO_RAD;

  // Random vector, biased back toward the starting point
  dx = constrain((int)(d * cosf(a) - netX * 0.6f), -100, 100);

  dy = constrain((int)(d * sinf(a) - netY * 0.6f), -100, 100);

}

uint32_t makeSeed()
{
  uint32_t seed = 0;

  for (int i = 0; i < 32; i++)
  {

    for (uint8_t p = A0; p <= A5; p++)
    {
      
      seed = (seed << 1) | (seed >> 31);      // rotate
      seed ^= analogRead(p) & 1;              // LSB only

    }

    seed ^= micros(); // timing jitter
  }

  return seed;

}

void setup()
{

  Serial.begin(115200);

  randomSeed(makeSeed());

  Mouse.begin();

}

void loop()
{

  int burst = random(1, 4);

  for (int i = 0; i < burst; i++) 
  {

    int dx, dy;

    pickTarget(dx, dy);

    glide(dx, dy);

    Serial.print("Moved "); 
    Serial.print(dx); 
    Serial.print(", "); 
    Serial.println(dy);

    int hold = random(80, 600); // hold for 80-600 ms

    Serial.print("Holding for ");
    Serial.println(hold);

    delay(hold);                // short hesitation between moves

  }

  delay(random(IDLE_MIN_MS, IDLE_MAX_MS));

}