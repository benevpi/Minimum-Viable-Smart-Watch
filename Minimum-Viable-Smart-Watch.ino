#include "config.h"
#include "secrets.h"

#include "esp_wifi.h"
#include <WiFi.h>
#include <ArduinoJson.h>

// Color definitions stolen from Adafruit
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

//Learnings
//Things you absolutely have to do to make the battery last
// * reduce clock speed
// * lower the back light brightness
// * not have serial output running
// * have an auto-power off on the display (3 or 4 seconds seems good)
// * Dark colours not readable outside, light ones are


// ToDO

//tilt to open and activity detect now working
// note that tilt to start expects the watch to be on the left wrist. Not sure if it's possible to re-map the axies to make it work on the right wrist.
// Also, there's a delay in the tilt that is a little annoying. Reducing the 300 ms wait maybe?
//Anyway, let's see how it affects battery consumption before worrying too much.

//looks like there's been some changes to the BMA class need to fetch the latest version to use the activity sensor

//useful functions of the bma423 that could be of use:
// getActicity (detects walking and running and stationary)
// isDoubleClick

//this one doesn't seem to be working? needs further investigation
// isTilt -- could use this to turn on the watch screen. see example here: https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library/blob/8f95c5ffaf5bdc208fe10783e686d937df62591c/examples/BasicUnit/BMA423_Feature/BMA423_Feature.ino

//would be cool to save activity data to a CSV file, then have a web server that runs when the watch is plugged in that serves up that file (and maybe visualises it)

//weather info coming through -- could do something more fine grained like the weather colour lines on the weather display

//some descriptions of weather too long -- overcast clouds, for example.
// not blanking old text
//would be nice to do a rain line

//if battery holds on current format, it'd be nice to include a tap function to switch to different data to display -- perhaps longer (or momre detailed) wether forecast & more info on step counter. Let's see how this holds up over the weekend
//would be nice to have some sort of data structure to hold screens that can be switched easily. Perhaps just an array of pointers to functions that draw the bit under the clock?
//need to sort out secrets file so i can github properly
//it'll need wifi and openweather app id

//put serial bits in ifdef debug tags to save power when not being used

//there's some interesting things in the wakeup from short press sketch in the examples folder

//note -- you can adjust the level of the backlight. Not too sure what the default is Tyring 50

// there's been a fairly dramatic drop in battery time since enabling wifi. May not be powering it off properly
// need to learn more about how power management works

//no error checking in the networking. No idea what will happen if it fails? Crash?
//need to use the date to work out when to re-fetch the weather
// -- this can be as simple as store the date when it's fetched and if the current date doesn't match this, then it needs re-fetching
//some sensible colours on the screen
// debugging?
// how do the changes affect battery life?

// Save steps per day between power downs
// upload steps somewhere? Google Sheets?
// no need to do processing (such as time updates) if the screen is off.
// could just get steps when the screen is turned on (maybe battery as well). 

//try light sleep rather than delay? something like:
// let's not over complicate things. The battery lasts all day anyway. Can look at this if/when needed

//    esp_sleep_enable_timer_wakeup(1000000); //1 seconds
//    int ret = esp_light_sleep_start();

//next thing to do -- detect charging and do wifi (weather) stuff when plugged in.
//if that works, work on online step tracking

//do I want things like detecting periods of inactivity? That might be nice.

// first version (no auto screen off and delay 200) had about 40% battery left at the end of the day

TTGOClass *ttgo;

//config opitions
int sleep_time = 4000; // 4 or five seconds needed for weather
int brightness = 50; // see how big an effect this has on both battery life and visibility outdoors

//loop delay


//weather option
String url= "/data/2.5/onecall?lat=51.45452&lon=-2.5879&exclude=current,minutely,hourly&units=metric&appid="; // note, you'll need to put your lat and long in there
const char * host = "api.openweathermap.org";
const int port = 80;

//set up different screens
const int num_screens = 3;
int screen = 0;
int current_screen = 0;

//each screen has two functions -- static and dynamic.
//static is drawn when the screen is shown, dynamic updates with each clock update
//for my use case, the dynamic screen will probably be the same for every screen, but that's not necessarily the case for all.
int (*screen_static[num_screens])();
int (*screen_dynamic[num_screens])();

char buf[128];
char battery_chars[128];
char time_chars[128];

//globals to hold data for display
String time_string = "";
String battery_string = "";

String current_activity = "";

//weather lines
String weather1 = "";
String weather2 = "";
String weather3 = "";
String weather4 = "";
String weather5 = "";

String weather6 = "";
String weather7 = "";
String weather8 = "";
String weather9 = "";
String weather10 = "";


bool irq = false;
bool toggle_screen = true;
bool charging = false;
bool forecast = false;

int last_on = 0;

//weather forecast -- we've got five lines.
//one day forecast from openweathermap

//date / max / min temp
//description
//chance of rain
//wind speed
//uv index

// Let's do the  screens here
int main_clock_dynamic() {
  ttgo->tft->setTextColor(GREEN, TFT_BLACK);
  ttgo->tft->drawString(battery_chars, 22, 80, 4);
  ttgo->tft->setTextColor(WHITE, TFT_BLACK);
  ttgo->tft->drawString(time_chars, 1, 10, 7);
}

int screen1_static() {
        ttgo->tft->fillScreen(TFT_BLACK);
          //update step counter only when the screen is refreshed
        ttgo->tft->setTextColor(GREEN, TFT_BLACK);
        snprintf(buf, sizeof(buf), "Steps: %u", ttgo->bma->getCounter());
        ttgo->tft->drawString(buf, 22, 60, 4);
        ttgo->tft->setTextColor(CYAN, TFT_BLACK);
        ttgo->tft->drawString(weather1, 22, 110, 4); // description
        ttgo->tft->drawString(weather2, 22, 135, 4); // temp
        ttgo->tft->drawString(weather3, 22, 160, 4); // wind
        ttgo->tft->setTextColor(MAGENTA, TFT_BLACK);       
        ttgo->tft->drawString(weather4, 22, 185, 4); // tomorrow
        ttgo->tft->drawString(weather5, 22, 210, 4); // tomorrow
}

int screen2_static() {
         ttgo->tft->fillScreen(TFT_BLACK);
          //update step counter only when the screen is refreshed
        ttgo->tft->setTextColor(GREEN, TFT_BLACK);
        snprintf(buf, sizeof(buf), "Steps: %u", ttgo->bma->getCounter());
        ttgo->tft->drawString(buf, 22, 60, 4);
        ttgo->tft->setTextColor(CYAN, TFT_BLACK);
        ttgo->tft->drawString(weather6, 22, 110, 4); // description
        ttgo->tft->drawString(weather7, 22, 135, 4); // temp

        ttgo->tft->setTextColor(MAGENTA, TFT_BLACK);       
        ttgo->tft->drawString(weather8, 22, 185, 4); // tomorrow
        ttgo->tft->drawString(weather9, 22, 210, 4); // tomorrow
}

int screen3_static() {
         ttgo->tft->fillScreen(TFT_BLACK);
          //update step counter only when the screen is refreshed
        ttgo->tft->setTextColor(GREEN, TFT_BLACK);
        snprintf(buf, sizeof(buf), "Steps: %u", ttgo->bma->getCounter());
        ttgo->tft->drawString(buf, 22, 60, 4);
        ttgo->tft->setTextColor(CYAN, TFT_BLACK);
        ttgo->tft->drawString("Debug data", 22, 110, 4); 
        snprintf(buf, sizeof(buf), "activity: %s", ttgo->bma->getActivity());
        ttgo->tft->drawString(buf, 22, 135, 4); // current activity

        ttgo->tft->setTextColor(MAGENTA, TFT_BLACK);       
        ttgo->tft->drawString("", 22, 185, 4); // tomorrow
        ttgo->tft->drawString("", 22, 210, 4); // tomorrow
}

//end screens

int connectToWiFi(const char * ssid, const char * pwd)
{

#ifdef debug
  Serial.println("Connecting to WiFi network: " + String(ssid));
#endif

  WiFi.begin(ssid, pwd);

  int counter = 0;

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
#ifdef debug
    Serial.print(".");
#endif
    counter++;
    if (counter > 120) { return 2;}
  }
#ifdef debug  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#endif
  return 0;
}

void get_forecast() {
  setCpuFrequencyMhz(80);
  connectToWiFi(networkName, networkPswd);
     // printLine();
#ifdef debug
  Serial.println("Connecting to domain: " + String(host));
#endif
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port))
  {
#ifdef debug
    Serial.println("connection failed");
#endif
    return;
  }
#ifdef debug
  Serial.println("Connected!");
#endif

  // This will send the request to the server
  client.print((String)"GET " + url + "  HTTP/1.1\r\n" +
               "Host: " + String(host) + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) 
  {
    if (millis() - timeout > 5000) 
    {
#ifdef debug
      Serial.println(">>> Client Timeout !");
#endif
      client.stop();
      setCpuFrequencyMhz(10);
      return;
    }
  }

   String line = ""; 
 while (client.available()) { 
   line = client.readStringUntil('\r');
   if (line.indexOf('{')>=0) {
#ifdef debug
   Serial.println(line); 
   Serial.println("parsingValues"); 
#endif
   //create a json buffer where to store the json data 

   DynamicJsonDocument json_doc(60000);

   DeserializationError json_error = deserializeJson(json_doc, line);

   if (json_error) { 
#ifdef debug
     Serial.println("parseing failed");
     Serial.println(json_error.c_str());
#endif
     
     //return; 
   } 
   else {
#ifdef debug
    Serial.println("success");
#endif

    //description - 
    weather1 = (const char*)json_doc["daily"][0]["weather"][0]["description"];
    weather2 = json_doc["daily"][0]["temp"]["min"].as<String>() + " - " + json_doc["daily"][0]["temp"]["max"].as<String>() + "C";
    weather3 = "wind: " + json_doc["daily"][0]["wind_speed"].as<String>();
    weather4 = "tmrw: " + json_doc["daily"][1]["weather"][0]["description"].as<String>().substring(0,12);
    weather5 = json_doc["daily"][1]["temp"]["min"].as<String>() + " - " + json_doc["daily"][1]["temp"]["max"].as<String>() + "C";

    weather6 = "day 2: " + json_doc["daily"][2]["weather"][0]["description"].as<String>().substring(0,12);
    weather7 = json_doc["daily"][2]["temp"]["min"].as<String>() + " - " + json_doc["daily"][1]["temp"]["max"].as<String>() + "C";
    weather8 = "day 3: " + json_doc["daily"][3]["weather"][0]["description"].as<String>().substring(0,12);
    weather9 = json_doc["daily"][3]["temp"]["min"].as<String>() + " - " + json_doc["daily"][1]["temp"]["max"].as<String>() + "C";

#ifdef debug
    Serial.println(weather1);
    Serial.println(weather2);
    Serial.println(weather3);
    Serial.println(weather4);
    Serial.println(weather5);
#endif
    //turn off wifi
    WiFi.mode(WIFI_OFF);
    
    forecast = true; // note, need a way to cancel this when the day ticks over
   }
   }
  
}
setCpuFrequencyMhz(10);
}


void low_energy() {
    if (toggle_screen) {
        ttgo->closeBL();
        ttgo->bma->enableStepCountInterrupt(false);
        
        ttgo->displaySleep();
        toggle_screen = false;
        
    } else {
        ttgo->displayWakeup();
        ttgo->openBL();
        ttgo->bma->enableStepCountInterrupt();
        toggle_screen=true;
        last_on = millis();

        //update step counter only when the screen is refreshed
        /**
        ttgo->tft->setTextColor(GREEN, TFT_BLACK);
        snprintf(buf, sizeof(buf), "Steps: %u", ttgo->bma->getCounter());
        ttgo->tft->drawString(buf, 22, 60, 4);
        ttgo->tft->setTextColor(BLUE, TFT_BLACK);
        ttgo->tft->drawString(weather1, 22, 110, 4); // description
        ttgo->tft->drawString(weather2, 22, 135, 4); // temp
        ttgo->tft->drawString(weather3, 22, 160, 4); // wind
        ttgo->tft->setTextColor(MAGENTA, TFT_BLACK);       
        ttgo->tft->drawString(weather4, 22, 185, 4); // tomorrow
        ttgo->tft->drawString(weather5, 22, 210, 4); // tomorrow
        **/
        (*screen_static[current_screen])();
        
    }
}

void setup()
{
    //may need to cut this to save power later
#ifdef debug
    Serial.begin(115200);
#endif

    //create the proper openweather URL
    url=url+appid;
    
    //set up screens
    screen_dynamic[0] = main_clock_dynamic;
    screen_dynamic[1] = main_clock_dynamic;
    screen_dynamic[2] = main_clock_dynamic;

    screen_static[0] = screen1_static;
    screen_static[1] = screen2_static;
    screen_static[2] = screen3_static;
    //end set up screens
    
    ttgo = TTGOClass::getWatch();
    ttgo->begin();
    //remove this for testing. May want to re-instate it at a future point
    //it's causing the wifi to glitch out, but might just need to be turned up and down depending on where we are with wifi.
    setCpuFrequencyMhz(10);
    ttgo->openBL();
    ttgo->bl->adjust(brightness);

    pinMode(BMA423_INT1, INPUT);

    //do we need interrupts on BMA now? yes, step counter doesn't work unless we process all the interrupts
    
    attachInterrupt(BMA423_INT1, [] {
        irq = 1;
    }, RISING);

    
    ttgo->bma->begin();
    ttgo->bma->attachInterrupt(); // not sure about this? looks like this does all the config as well.

     ttgo->bma->enableFeature(BMA423_ACTIVITY, true);
     ttgo->bma->enableActivityInterrupt(true);


    //ttgo->bma->enableTiltInterrupt(true); // nont too sure what the true and false refer to. Taken from sample code
    //ttgo->bma->enableAccel(); // not sure if this is needed
    


    pinMode(AXP202_INT, INPUT_PULLUP);
    attachInterrupt(AXP202_INT, [] {
        irq = true;
    }, FALLING);

    //!Clear IRQ unprocessed  first
    ttgo->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ | AXP202_VBUS_REMOVED_IRQ | AXP202_VBUS_CONNECT_IRQ | AXP202_CHARGING_IRQ, true);
    ttgo->power->clearIRQ();

    ttgo->power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);

    ttgo->tft->fillScreen(TFT_BLACK);
    // a really clumsy way of setting time -- uncoment this and put in the correct time, then comment it and re-flash
    //ttgo->rtc->setDateTime(2020, 7, 7, 10, 01, 50);

    last_on = millis();

    //let's just put this here for testing
    //should maybe be wrapped in something for checking battery level / 
    get_forecast();

}

void loop()
{
    int per = ttgo->power->getBattPercentage();
    if (irq) {
        irq = 0;

        // don't fully understand why this is necessary to get the step counter, but copied from the example and doesn't work if it's removed
        // could this be moved to low-energy, or only done if the screen is on? Feel like it might be sapping power.
        bool  rlst;
        do {
            rlst =  ttgo->bma->readInterrupt();
        } while (!rlst);

        //nnot working, don't know whyy?
        if (!toggle_screen && ttgo->bma->isTilt()) {
            low_energy();
        }
        
        ttgo->power->readIRQ();

        if (ttgo->power->isPEKShortPressIRQ()) {
            low_energy();
        }

        if (ttgo->power->isVbusPlugInIRQ()) {
            charging = true;
        }
        if (ttgo->power->isVbusRemoveIRQ()) {
            charging = false;            
        }
        
        ttgo->power->clearIRQ();
    }

    //let's factor the power bit out and only di it if the screen is on
    //ttgo->tft->setTextColor(GREEN, TFT_BLACK);
    if (charging) {
      snprintf(battery_chars, sizeof(battery_chars), "Battery: %u power", ttgo->power->getBattPercentage());
    }
    else {
      snprintf(battery_chars, sizeof(battery_chars), "Battery: %u                ", ttgo->power->getBattPercentage());
    }
    //ttgo->tft->drawString(buf, 22, 80, 4); moved to dynamic function

    //ttgo->tft->setTextColor(WHITE, TFT_BLACK);
    snprintf(time_chars, sizeof(time_chars), "%s", ttgo->rtc->formatDateTime());
    //ttgo->tft->drawString(buf, 1, 10, 7);
    // end bit to cut if screen not on here

    //draw dynamic part of the screen
    (*screen_dynamic[current_screen])();

    if(toggle_screen & millis() > (last_on + sleep_time)) { low_energy(); } // turn screen off after set number of seconds

    //get weather if both charging and we don't already have it today
    if (charging and forecast == false) {
      get_forecast();
    }

    //touch sensing
    //doesn't seem to need a debounce, the loop delay is enough
    // may want it to reset the screen timeout
    int16_t x;
    int16_t y;
    //I think this && is lazy, so we shouldn't be doing the getTouch if the screen is off?
    //There is also the touched function
     if (toggle_screen && ttgo->getTouch(x, y)) {
        current_screen++;
        last_on = millis();
        if (current_screen >= num_screens) { current_screen=0;}
        (*screen_static[current_screen])();
    }

    //could shorten the delay when the display is active to make the touch interface more responsive.
    delay(300);
}
