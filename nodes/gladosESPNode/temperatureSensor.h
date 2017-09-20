#ifndef TEMPERATURESENSOR_H
#define TEMPERATURESENSOR_H

#include "gladosMQTTNode.h"

class dhtTemperatureSensor : public nodeComponent
{
public:
  dhtTemperatureSensor(int pin, uint8_t t) : nodeComponent() , dht(pin,t)
  {
	m_id = "dhtSensor";
  }
  
	void setup()
	{
		dht.begin();
	}
	
	void updateComponent()	{return;}

	float temperature()   {return last_data.temperature;}
	float humidity()	  {return last_data.relative_humidity;}
	
	void readComponent()
	{
		dht.temperature().getEvent(&last_data);
		mqttPublication pt;
		pt.path  = m_topicPath;
		pt.topic = "temperature";
		pt.val	= last_data.temperature;
		m_publications.push_back(pt);

		dht.humidity().getEvent(&last_data);
		mqttPublication ph;
		ph.path  = m_topicPath;
		ph.topic = "humidity";
		ph.val	= last_data.relative_humidity;
		m_publications.push_back(ph);
	}
	
protected:
	DHT_Unified      dht;
	sensors_event_t  last_data;
};



class ntc100kTemperatureSensor : public nodeComponent
{
public:
  ntc100kTemperatureSensor(int pin)
  {
		m_pin = pin;
  }

  void setup()
  {

  }

  bool isValid()        {return true;}
  bool     canRead()    {return true;}

  uint16_t readRaw()
  {
      return analogRead(m_pin);
  }

  virtual float      readData()
  {
    return convertRead(readRaw());
  }


  static float convertRead(uint16_t raw)
  {

    #define THERMISTORNOMINAL 80000
    // temp. for nominal resistance (almost always 25 C)
    #define TEMPERATURENOMINAL 25
    // how many samples to take and average, more takes longer
    // but is more 'smooth'
    //#define NUMSAMPLES 5
    // The beta coefficient of the thermistor (usually 3000-4000)
    #define BCOEFFICIENT 3950
    // the value of the 'other' resistor
    #define SERIESRESISTOR 160000

      float average = raw;
      // convert the value to resistance
      average = 1023 / average - 1;
      average = SERIESRESISTOR / average;
      Serial.print("Thermistor resistance ");
      Serial.println(average);

      float steinhart;
      steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
      steinhart = log(steinhart);                  // ln(R/Ro)
      steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
      steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
      steinhart = 1.0 / steinhart;                 // Invert
      steinhart -= 273.15;                         // convert to C

      Serial.print("Temperature ");
      Serial.print(steinhart);
      Serial.println(" *C");
      return steinhart;
  }

protected:
	int m_pin;

};



class tmp36Sensor : public nodeComponent
{
public:
  tmp36Sensor(int pin)
  {
	m_pin = pin;
  }

  void setup()
  {
    pinMode(m_pin, INPUT);
  }

  bool isValid()        {return true;}
  bool     canRead()    {return true;}

  uint16_t readRaw()
  {
      return analogRead(m_pin);
  }

  virtual float      readData()
  {
    uint16_t value = readRaw();
    uint8_t iterations = 10;// 64 es lo maximo que un uint16 puede albergar de forma segura para hacer la media (si todos son 1024...1024*64 = 16bits)
    for(int i = 0 ; i < iterations ; i++)
    {
        value += readRaw();
    }
    return convertRead(value/iterations);
  }


  static float convertRead(uint16_t raw)
  {
      //Serial.print(raw); Serial.println(" raw");
      // converting that reading to voltage, for 3.3v arduino use 3.3
      float voltage = raw * 3.3;
      voltage /= 1024.0;
      float temperatureC = (voltage - 0.5) * 100 ;  //converting from 10 mv per degree wit 500 mV offset
                                                    //to degrees ((voltage - 500mV) times 100)

      //Serial.print(temperatureC); Serial.println(" degrees C");
      // now convert to Fahrenheit
      //float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
      //Serial.print(temperatureF); Serial.println(" degrees F");

      return temperatureC;
  }

protected:
	int m_pin;

};


#endif // TEMPERATURESENSOR_H
