
/*******************************************************************
*                   ==SmartGreenHouse==
*   Sensors Class
*   Created: 17/02/2013
*   Author:  Ax
 *   License: CC BY-SA 3.0
 *            http://creativecommons.org/licenses/by-sa/3.0/
 *=====================================================================
Sensors Class
Contains all sensor procedure to update state variable
********************************************************************/


#ifndef SENSORS_H
#define SENSORS_H

#include "State.h"
#include <DHT.h>

#define DHTPIN 8
#define DHTTYPE DHT22

#define LEVELPIN 5
#define LIGHTAPIN 0

#define LIGHTTAU 10
#define HUMIDITYTAU 120

#define GOODLEVEL HIGH
#define DHTMAXERRN 5
class Sensors
{
//To be used with noisy input
    class LowPassFilter
    {
    public:
//Pole tau and dt setting. dt_s is LOOPT and tau_s must be tuned for the
        void setup(float tau_s)
        {
            this->a=tau_s/(tau_s+LOOPT);
        }
        float update(float value)
        {
            this->sum*=1-a;
            this->sum+=a*value;

            return this->sum;
        }
        float a;
        float sum;
    };
public:
    int setup();
    //Update status with measured data
    int update(State *state);
protected:
private:
    DHT dht;
    unsigned short dhterrN;
    LowPassFilter lplight,lphumidity;

};
/** @brief update
 *
 * @todo: document this function
 */
int Sensors::update(State *state)
{
    //Read temperature and humudity and store it in state variable-----------------
    float h = dht.readHumidity();
    float t = dht.readTemperature();

//If some error occurred
    if (isnan(t) || isnan(h))
    {
        //If dht11 failed for too many times consecutively
        if((++this->dhterrN)>DHTMAXERRN)
        {
            state->esensors|=State::ESENS_DHTERR;
            state->log(State::CRITICAL,"Dht11 major error");
        }
        else
            state->log(State::ERROR,"Dht11 minor error");
    }
    else
    {
        //Clear DHT11 Error (If present)
        state->esensors&=!(State::ESENS_DHTERR);
        //Update temperature & humidity
        state->temp=t;
        state->humidity=h;

        //Set error count to 0
        this->dhterrN=0;
    }
    //Set level according to level sensor----------------------------------
    if(!(digitalRead(LEVELPIN)==GOODLEVEL) && state->level)
    {
        //Level just went low
        state->log(State::ERROR,"Low Water level");
    }
    state->level=(digitalRead(LEVELPIN)==GOODLEVEL);
    //Read Light sensor----------------------------------------------------
    state->light=analogRead(LIGHTAPIN);
    //Filtered variables
    state->flight=this->lplight.update(state->light);
    state->fhumidity=this->lphumidity.update(state->humidity);
}

/** @brief setup
 *
 * @todo: document this function
 */
int Sensors::setup()
{
    pinMode(LEVELPIN,INPUT);

    //Set error number to 0
    this->dhterrN=0;
    this->dht=DHT(DHTPIN,DHTTYPE);
    this->dht.begin();
    //Filter definition
    this->lplight.setup(LIGHTTAU);
    this->lphumidity.setup(HUMIDITYTAU);

}




#endif // SENSORS_H

