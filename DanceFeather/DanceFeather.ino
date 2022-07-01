// SPDX-FileCopyrightText: 2020 Carter Nelson for Adafruit Industries
//
// SPDX-License-Identifier: MIT
//
#include <Adafruit_APDS9960.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_LSM6DS33.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_Sensor.h>
#include <PDM.h>
#include <KickFFT.h>

#define ACC_SIZE 64
#define FFT_SIZE 512

Adafruit_APDS9960 apds9960; // proximity, light, color, gesture
Adafruit_BMP280 bmp280;     // temperautre, barometric pressure
Adafruit_LIS3MDL lis3mdl;   // magnetometer
Adafruit_LSM6DS33 lsm6ds33; // accelerometer, gyroscope
Adafruit_SHT31 sht30;       // humidity

uint8_t proximity;
uint16_t r, g, b, c;
float temperature, pressure, altitude;
float magnetic_x, magnetic_y, magnetic_z;
float accel_x, accel_y, accel_z;
float gyro_x, gyro_y, gyro_z;
float humidity;
int32_t mic;

extern PDMClass PDM;
short tempBuffer[256];  // buffer to read samples into, each sample is 16-bits
float acc = 0;
float fftBuffer[FFT_SIZE];
uint16_t numInAcc = 0;
volatile int samplesRead; // number of samples read

void setup(void) {
  // This allows for higher precision micros().  Without dwt micros updates in increments of about 1000.
  dwt_enable();
  Serial.begin(115200);
  // while (!Serial) delay(10);
  Serial.println("Feather Sense Sensor Demo");

  // initialize the sensors
  apds9960.begin();
  apds9960.enableProximity(true);
  apds9960.enableColor(true);
  bmp280.begin();
  lis3mdl.begin_I2C();
  lsm6ds33.begin_I2C();
  sht30.begin();
  PDM.onReceive(onPDMdata);
  PDM.begin(1, 16000);
}

void loop(void) {
  proximity = apds9960.readProximity();
  while (!apds9960.colorDataReady()) {
    delay(5);
  }
  apds9960.getColorData(&r, &g, &b, &c);

  temperature = bmp280.readTemperature();
  pressure = bmp280.readPressure();
  altitude = bmp280.readAltitude(1013.25);

  lis3mdl.read();
  magnetic_x = lis3mdl.x;
  magnetic_y = lis3mdl.y;
  magnetic_z = lis3mdl.z;

  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  lsm6ds33.getEvent(&accel, &gyro, &temp);
  accel_x = accel.acceleration.x;
  accel_y = accel.acceleration.y;
  accel_z = accel.acceleration.z;
  gyro_x = gyro.gyro.x;
  gyro_y = gyro.gyro.y;
  gyro_z = gyro.gyro.z;

  humidity = sht30.readHumidity();

  samplesRead = 0;
  mic = getPDMwave(4000);

  Serial.println("\nFeather Sense Sensor Demo");
  Serial.println("---------------------------------------------");
  Serial.print("Proximity: ");
  Serial.println(apds9960.readProximity());
  Serial.print("Red: ");
  Serial.print(r);
  Serial.print(" Green: ");
  Serial.print(g);
  Serial.print(" Blue :");
  Serial.print(b);
  Serial.print(" Clear: ");
  Serial.println(c);
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");
  Serial.print("Barometric pressure: ");
  Serial.println(pressure);
  Serial.print("Altitude: ");
  Serial.print(altitude);
  Serial.println(" m");
  Serial.print("Magnetic: ");
  Serial.print(magnetic_x);
  Serial.print(" ");
  Serial.print(magnetic_y);
  Serial.print(" ");
  Serial.print(magnetic_z);
  Serial.println(" uTesla");
  Serial.print("Acceleration: ");
  Serial.print(accel_x);
  Serial.print(" ");
  Serial.print(accel_y);
  Serial.print(" ");
  Serial.print(accel_z);
  Serial.println(" m/s^2");
  Serial.print("Gyro: ");
  Serial.print(gyro_x);
  Serial.print(" ");
  Serial.print(gyro_y);
  Serial.print(" ");
  Serial.print(gyro_z);
  Serial.println(" dps");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.print("Mic: ");
  Serial.println(mic);
  delay(300);
}

void fft_phase(float fs, float f1, float f2, uint16_t samples, const float data[],
						uint32_t mag[], float phase[], uint16_t &startIndex, uint16_t &endIndex)
{
	//changes f1 and f2 to indices
	//fs/samples gives the increments of frequency on the x-axis
	startIndex = f1/(fs/samples);
	endIndex = f2/(fs/samples);
	
	
	for (uint16_t i = startIndex; i < endIndex; i++)
	{
		signed long int real = 0;
		signed long int imag = 0;
		
		
		//Euler's Identity
		for (uint16_t j = 0; j < samples; j++)
		{
			//uses lookup tables for trigonometric
			//functions to save compputing power
			switch(samples)
			{
				case 512:
					real += intcosine512[ (i*j) - (samples*((i*j)/samples)) ] * data[j];
					imag += intsine512[ (i*j) - (samples*((i*j)/samples)) ] * data[j];
					break;
					
				case 256:
					real += intcosine256[ (i*j) - (samples*((i*j)/samples)) ] * data[j];
					imag += intsine256[ (i*j) - (samples*((i*j)/samples)) ] * data[j];
					break;
					
				case 128:
					real += intcosine128[ (i*j) - (samples*((i*j)/samples)) ] * data[j];
					imag += intsine128[ (i*j) - (samples*((i*j)/samples)) ] * data[j];
					break;
					
				case 64:
					real += intcosine64[ (i*j) - (samples*((i*j)/samples)) ] * data[j];
					imag += intsine64[ (i*j) - (samples*((i*j)/samples)) ] * data[j];
					break;
					
				case 32:
					real += intcosine32[ (i*j) - (samples*((i*j)/samples)) ] * data[j];
					imag += intsine32[ (i*j) - (samples*((i*j)/samples)) ] * data[j];
					break;
					
				default:
					break;
			}
		}
		
		
		//dividing each number by 1000 to prevent each number from getting too large
		//Also adjusts for the fact that the trigonometric values were multiplied
		//by 1000 as well to make them integers instead of decimal values
		real = real/1000;
		imag = imag/1000;
		
                float phaseRad = atan2(imag, real) + PI;
		
		//calculating magnitude of the data by taking the square root of the
		//sum of the squares of the real and imaginary component of each signal
		mag[i] = KickMath<signed long int>::calcMagnitude(real, imag);
		phase[i] = phaseRad;
	}
}


void addToFFT(float val) {
  // Serial.println(val);
  for (int i = FFT_SIZE - 1; i > 0; i--) {
    fftBuffer[i] = fftBuffer[i-1];
  }
  fftBuffer[0] = val;
  float fs = 16000/ACC_SIZE; // 250 Hz
  uint32_t mag[FFT_SIZE];
  float phase[FFT_SIZE];
  uint16_t startIndex, endIndex;
  // KickFFT<int32_t>::fft(fs, 0, 4, FFT_SIZE, fftBuffer, mag, startIndex, endIndex);
  fft_phase(fs, .5, 4, FFT_SIZE, fftBuffer, mag, phase, startIndex, endIndex);
  Serial.print("FFT: ");
  for (int i = startIndex; i < endIndex; i++) {
    Serial.print(phase[i]);
    Serial.print(" ");
  }
  Serial.println("");
}

/*****************************************************************/
int32_t getPDMwave(int32_t samples) {
  short minwave = 30000;
  short maxwave = -30000;

  while (samples > 0) {
    if (!samplesRead) {
      yield();
      continue;
    }
    for (int i = 0; i < samplesRead; i++) {
      float newVal = tempBuffer[i];
      acc += newVal;
      numInAcc++;
      if (numInAcc >= ACC_SIZE) {
          float value = acc / ACC_SIZE;
          acc = 0;
          numInAcc = 0;
          minwave = min(value, minwave);
          maxwave = max(value, maxwave);
          addToFFT(value);
      }
      // minwave = min(tempBuffer[i], minwave);
      // maxwave = max(tempBuffer[i], maxwave);

      //samples--;
    }
    // clear the read count
    samplesRead = 0;
  }
  Serial.print("max: ");
  Serial.println(maxwave);
  Serial.print("min: ");
  Serial.println(minwave);
  return maxwave - minwave;
}

void onPDMdata() {
  // query the number of bytes available
  int bytesAvailable = PDM.available();

  // read into the sample buffer
  PDM.read(tempBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
}
