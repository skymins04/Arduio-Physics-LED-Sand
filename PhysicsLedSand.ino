/*
 Name:		PhysicsLedSand.ino
 Created:	2018-09-01 P.M. 8:39:53
 Author:	BetaMan
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303.h>
#include <Adafruit_LSM303_U.h>
#include <LedControl.h>

Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);
LedControl matrix = LedControl(11, 9, 10, 2);
sensors_event_t event;

#define N_GRAINS 40
#define WIDTH 8
#define HEIGHT 16
#define MAX_X (WIDTH - 1) * 256
#define MAX_Y (HEIGHT - 1) * 256

struct Grains {
	int16_t x, y;
	int16_t vx, vy;
} grain[N_GRAINS];

boolean screen[HEIGHT][WIDTH];

void setup() {
	matrix.shutdown(0, false); matrix.shutdown(1, false);
	matrix.setIntensity(0, 8); matrix.setIntensity(1, 8);
	matrix.clearDisplay(0); matrix.clearDisplay(1);

	Serial.begin(115200);

	accel.begin();

	randomSeed(analogRead(0));
	memset(screen, false, sizeof(screen));
	for (int i = 0; i < N_GRAINS; i++) {
		int j;
		do {
			grain[i].x = random(MAX_X);
			grain[i].y = random(MAX_Y);
			for (j = 0; (j<i) && (((grain[i].x / 256) != (grain[j].x / 256)) || ((grain[i].y / 256) != (grain[j].y / 256))); j++);
		} while (j < i);
		
		screen[(int16_t)(grain[i].y / 256)][(int16_t)(grain[i].x / 256)] = true;
		
		grain[i].vx = grain[i].vy = 0;
	}
}

void loop() {
	// print matrix
	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			if (i > 7) matrix.setLed(1, i - 8, j, screen[i][j]);
			else matrix.setLed(0, i, j, screen[i][j]);
		}
	}

	accel.getEvent(&event);
	int8_t ax = event.acceleration.x, ay = event.acceleration.y, az = event.acceleration.z;
	
	for (int i = 0; i < N_GRAINS; i++) {
		grain[i].vx = ax * 200.0;
		grain[i].vy = ay * 200.0;

		(grain[i].vx > 0) ? grain[i].vx += random(az*1.0) : grain[i].vx += random(az, 0);
		(grain[i].vy > 0) ? grain[i].vy += random(az*1.0) : grain[i].vy += random(az, 0);

		int16_t newx = grain[i].x + grain[i].vx, newy = grain[i].y + grain[i].vy;

		if (newx > MAX_X) newx = MAX_X;
		else if (newx < 0) newx = 0;
		if (newy > MAX_Y) newy = MAX_Y;
		else if (newy < 0) newy = 0;

		if (screen[(int)grain[i].y / 256][(int)newx / 256]) newx = grain[i].x;
		if (screen[(int)newy / 256][(int)grain[i].x / 256]) newy = grain[i].y;

		//memset(screen, false, sizeof(screen));
		screen[(int)grain[i].y / 256][(int)grain[i].x / 256] = false;
		screen[(int)newy / 256][(int)newx / 256] = true;

		Serial.println(String((int)newx / 256) + " , " + String((int)newy / 256) + " | | | " + String(newx) + " , " + String(newy));
		grain[i].x = newx; grain[i].y = newy;
	}
	memset(screen, false, sizeof(screen));
	for (int i = 0; i < N_GRAINS; i++) {
		screen[(int)grain[i].y / 256][(int)grain[i].x / 256] = true;
	}
}