# Minimum Viable Smart Watch
 A sample firmware for playing around with the Lilygo T-Watch 2020
 
# using this code
It's in a pretty high state of flux at the moment. I try to make sure that the latest version here always works (at least compiles and runs), but no promises.

In order to compile it, you'll need the board added to your Arduino IDE, official Arduino t-watch library (https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library) installed. Also, ArduinoJSON. I think that's all the requirements

# secrets
You'll need a secrets.h file with your personal secret data (wifi logins and open weather app id at the moment). See ex-secrets.h for details

Below are a bunch of comments I pulled out of the main file. They fit better here, but need formatting / sorting

# Learnings
Things you absolutely have to do to make the battery last
 * reduce clock speed
 * lower the back light brightness
 * not have serial output running
 * have an auto-power off on the display (3 or 4 seconds seems good)
 * Dark colours not readable outside, light ones are (sound's obvious!)
 
I don't think it's possible to use light and deep sleep without vastly complicating proceedings because lightsleep isn't implemented in Arduino and deepsleep wipes variables. There might be a way of writing them to RTCmemory / disk, but it's not something I want to spend time on initially

The BMA class has changed recently and is not versioned. Need to have the latest version in order to build

# Stuff implemented
* tilt to turn on
* multiple screens that you can switch with a touch (note, there's a delay in this, so you have to hold your finger on the screen. not ideal, but also OK and not on my list of things to fix short-term)

# Stuff to be implemented
* double tap detection
* saving data to SPIFFS
* uploading data to online storage
* inactivity detection (& buzz?)
* time spent in different activities

# Structure
Each 'screen' on the phone has two functions, static which is drawn each time the screen is turned on, and dynamic which is drawn each second. This may be expanded to a touch handler and a double click handler. Not implemented these yet. Pointers to the screen functions are held in two arrays that are set up in Setup():

```c
    screen_dynamic[0] = main_clock_dynamic;
    screen_dynamic[1] = main_clock_dynamic;
    screen_dynamic[2] = main_clock_dynamic;

    screen_static[0] = screen1_static;
    screen_static[1] = screen2_static;
    screen_static[2] = screen3_static;
 ```


# comments rippedo out of code

//power useage seems to be up. Things to try
// turning the display fully off: (from https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library/blob/master/examples/BasicUnit/WakeupFormSensor/WakeupFormSensor.ino)
//    watch->powerOff();
    // LDO2 is used to power the display, and LDO2 can be turned off if needed
    // power->setPowerOutPut(AXP202_LDO2, false);


//useful functions of the bma423 that could be of use:
// getActicity (detects walking and running and stationary)
// isDoubleClick

//would be cool to save activity data to a CSV file, then have a web server that runs when the watch is plugged in that serves up that file (and maybe visualises it)

//next thing to do -- detect charging and do wifi (weather) stuff when plugged in.
//if that works, work on online step tracking

//do I want things like detecting periods of inactivity? That might be nice.
