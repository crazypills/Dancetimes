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

// works but slow
//#define ACC_SIZE 64
//#define FFT_SIZE 512
//#define HANN_SIZE 60

#define LOW_PASS_SIZE 64 // 250Hz sample freq, this means signals above 125Hz are filtered
#define ACC_SIZE 2 // reduce sample rate before doing fft for performance reasons
#define FFT_SIZE 256
#define HANN_SIZE 40
#define FS (16000.0 / ACC_SIZE / LOW_PASS_SIZE)
//#define F2 3.5f
#define F2 6.8f
#define END_INDEX ((uint16_t) (F2 / (FS / FFT_SIZE)))

#define LED_PIN 13

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

float fftBuffer[FFT_SIZE]; // fft buffer holds the half rectfifed differences
float incomingWindow[HANN_SIZE];
float hannWindow[HANN_SIZE];
float prevWindowedVal;

// accumulate incoming values to create low pass filter
extern PDMClass PDM;
float lowPass = 0;
uint16_t numInLowPass = 0;
float acc = 0;
uint16_t numInAcc = 0;

// raw values incoming from PDM
short tempBuffer[256];  // buffer to read samples into, each sample is 16-bits
volatile int samplesRead; // number of samples read

void setup(void) {
  // This allows for higher precision micros().  Without dwt micros updates in increments of about 1000.
  dwt_enable();
  Serial.begin(115200);
  // while (!Serial) delay(10);
  Serial.println("Feather Sense Sensor Demo");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

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
  PDM.setGain(10);

  // init hann window
  for (int i = 0; i < HANN_SIZE ; i++) {
    float sinVal = sin(PI * (i + HANN_SIZE) / (2.0f * HANN_SIZE));
    hannWindow[i] = sinVal * sinVal;
  }

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

// rads is always between [-pi, pi)
float norm_rads(float angle_rad) {
    if (angle_rad < -PI) {
        return angle_rad + 2 * PI;
    } else if (angle_rad >= PI) {
        return angle_rad - 2 * PI;
    }
    return angle_rad;
}

void fft_phase(float fs, float f1, float f2, uint16_t samples, const float data[],
						float mag[], float phase[], uint16_t &startIndex, uint16_t &endIndex)
{
	//changes f1 and f2 to indices
	//fs/samples gives the increments of frequency on the x-axis
	startIndex = f1/(fs/samples);
	endIndex = f2/(fs/samples);
	
	
	for (uint16_t i = startIndex; i < endIndex; i++)
	{
		float real = 0;
		float imag = 0;
		
		
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
		real = real / 1000 / samples;
		imag = imag / 1000 / samples;
		
                float phaseRad = atan2(imag, real);
		
                // mag[i] = log2(sqrt(real * real + imag * imag));
                mag[i] = sqrt(real * real + imag * imag);
		phase[i] = phaseRad;
	}
}

float prevPhase[END_INDEX];
uint16_t winningIndex = 0;
float phaseAvg[END_INDEX];
float phaseRateAverage[END_INDEX];
float magAvg[END_INDEX];

float updatePhase(const float mag[], const float phases[], uint16_t startIndex, uint16_t endIndex) {
    float maxMag = -1000;
    uint16_t maxIndex = startIndex;
    bool ret = 0;
    for (int i = startIndex; i < endIndex; i++) {
        float magnitude = mag[i];
        magAvg[i] = magAvg[i] * 0.9 + magnitude * 0.1;
        // We look for the beat in 2-7, we can also check i*2 to verify
        // usually the overtones are synced up as well
        if (magAvg[i] > maxMag && i > 1 && i < 7) {
            maxMag = magAvg[i];
            maxIndex = i;
        }

        float phase = phases[i];
        float old_phase = prevPhase[i];
	prevPhase[i] = phase;

        // Add the rate to our phase even in the case where we don't update the rate.
        phaseAvg[i] += phaseRateAverage[i];
        phaseAvg[i] = norm_rads(phaseAvg[i]);

        float phaseDiff = norm_rads(phase - old_phase);

        if (winningIndex == i) {
          phaseRateAverage[i] = phaseRateAverage[i] * 0.98 + phaseDiff * 0.02;
        } else {
          phaseRateAverage[i] = phaseRateAverage[i] * 0.9 + phaseDiff * 0.1;
        }

        // Serial.print("PhaseRateAvg: "); Serial.println(phaseRateAverage);
        // Serial.print("PhaseDiff: "); Serial.println(phaseDiff);

        if (phase + PI < phaseAvg[i]) {
            phase += 2*PI;
        } else if (phase - PI > phaseAvg[i]) {
            phase -= 2*PI;
        }

        if (winningIndex == i) {
          phaseAvg[i] = phaseAvg[i] * 0.99 + phase * 0.01;
        } else {
          // aggressively converge for losers
          phaseAvg[i] = phaseAvg[i] * 0.9 + phase * 0.1;
        }
        phaseAvg[i] = norm_rads(phaseAvg[i]);
        // Serial.print("Phase    : "); Serial.println(phase);
        // Serial.print("Phase: "); Serial.println(_phase_avg);
    }

    winningIndex = maxIndex;
    //if (_phase_avg > 0) {
    //  digitalWrite(LED_PIN, LOW);
    //} else {
    //  digitalWrite(LED_PIN, HIGH);
    //}

    return ret;
}



void addToFFT(float val) {
  // Serial.print("Val: "); Serial.println(val);
  val = abs(val); // rectify val
  // val = val * val;
  float windowedVal = 0;
  for (int i = HANN_SIZE - 1; i > 0; i--) {
    incomingWindow[i] = incomingWindow[i - 1];
    windowedVal += incomingWindow[i] * hannWindow[i];
  }
  windowedVal += val;
  incomingWindow[0] = val;
  
  for (int i = FFT_SIZE - 1; i > 0; i--) {
    fftBuffer[i] = fftBuffer[i-1];
  }

  float diff = max(0, windowedVal - prevWindowedVal);
  fftBuffer[0] = diff;
  // Serial.print("Diff: "); Serial.println(diff);

  prevWindowedVal = windowedVal;

  float mag[END_INDEX];
  float phase[END_INDEX];
  uint16_t startIndex, endIndex;
  // KickFFT<int32_t>::fft(fs, 0, 4, FFT_SIZE, fftBuffer, mag, startIndex, endIndex);
  fft_phase(FS, 0, F2, FFT_SIZE, fftBuffer, mag, phase, startIndex, endIndex);
  updatePhase(mag, phase, startIndex, endIndex);
  bool phaseNearZero = false;
  for (int i = startIndex; i < endIndex; i++) {
      if (i == winningIndex && abs(phaseAvg[i]) < 0.05f * i) {
        phaseNearZero = true;
      }
  }

  if (diff > 2 * magAvg[0]) {
    digitalWrite(LED_PIN, LOW);

    Serial.print("Prev :\t");
    for (int i = 0; i < endIndex; i++) {
      float phase = prevPhase[i];
      Serial.print(phase);
      Serial.print("\t");
    }
    Serial.println("");

    // Serial.print("Diff:\t"); Serial.println(diff);
    Serial.print("Phase:\t");
    for (int i = 0; i < endIndex; i++) {
      float phase = phaseAvg[i];
      Serial.print(phase);
      Serial.print("\t");
    }
    Serial.println("");

    Serial.print("BPM:\t");
    for (int i = 0; i < endIndex; i++) {
      float hz = FS * phaseRateAverage[i] / 2 / PI;
      float bpm = hz * 60;
      Serial.print(bpm);
      Serial.print("\t");
    }
    Serial.println("");

    Serial.print("Mag:\t");
    for (int i = 0; i < endIndex; i++) {
      float mag = magAvg[i];
      Serial.print(mag);
      Serial.print("\t");
    }
    Serial.println("");

  }

  if (diff > 1 * magAvg[0] && phaseNearZero) {
    Serial.println("BEAT!");
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }


  // float hz = FS * phaseRateAverage / 2 / PI;
  // Serial.print("endIndex: "); Serial.println(endIndex);
  // Serial.print("END_INDEX: "); Serial.println(END_INDEX);
  // Serial.print("BPM: "); Serial.println(hz * 60);
  //Serial.print("radsPerUpdate: "); Serial.println(phaseRateAverage);


  // each sample changes the phase phaseRateAverage rads
  
 
  // Serial.print("startIndex: ");
  // Serial.print(startIndex);
  // Serial.print("  Mag: ");
  // for (int i = startIndex; i < endIndex; i++) {
  //   Serial.print(mag[i]);
  //   Serial.print(" ");
  // }
  // Serial.println("");

  // Serial.print("Phase: ");
  // for (int i = startIndex; i < endIndex; i++) {
  //   Serial.print(phase[i]);
  //   Serial.print(" ");
  // }
  // Serial.println("");
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
    uint32_t start = micros();
    for (int i = 0; i < samplesRead; i++) {
      float newVal = tempBuffer[i];
      lowPass += newVal;
      if (++numInLowPass >= LOW_PASS_SIZE) {
        lowPass /= LOW_PASS_SIZE;
        acc += lowPass * lowPass;
        lowPass = 0;
        numInLowPass = 0;
        numInAcc++;
      }

      if (numInAcc >= ACC_SIZE) {
          addToFFT(acc / 2048);
          acc = 0;
          numInAcc = 0;
      }

      //samples--;
    }
    // clear the read count
    //uint32_t duration = micros() - start;
    //Serial.print("micros: "); Serial.println(duration);
    //Serial.print("samples: "); Serial.println(samplesRead);

    samplesRead = 0;
  }
  Serial.print("max: ");
  Serial.println(maxwave);
  Serial.print("min: ");
  Serial.println(minwave);
  return maxwave - minwave;
}

void onPDMdata() {
  if (samplesRead) {
    Serial.println("PDM data did not get processed");
  }

  // query the number of bytes available
  int bytesAvailable = PDM.available();

  // read into the sample buffer
  PDM.read(tempBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
}
