# Minimum Viable Smart Watch
 A sample firmware for playing around with the Lilygo T-Watch 2020
 
# using this code
It's in a pretty high state of flux at the moment. I try to make sure that the latest version here always works (at least compiles and runs), but no promises

# secrets
You'll need a secrets.h file with your personal secret data (wifi logins and open weather app id at the moment). See ex-secrets.h for details

Below are a bunch of comments I pulled out of the main file. They fit better here, but need formatting / sorting
//Learnings
//Things you absolutely have to do to make the battery last
// * reduce clock speed
// * lower the back light brightness
// * not have serial output running
// * have an auto-power off on the display (3 or 4 seconds seems good)
// * Dark colours not readable outside, light ones are


// ToDO

//power useage seems to be up. Things to try
// turning the display fully off: (from https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library/blob/master/examples/BasicUnit/WakeupFormSensor/WakeupFormSensor.ino)
//    watch->powerOff();
    // LDO2 is used to power the display, and LDO2 can be turned off if needed
    // power->setPowerOutPut(AXP202_LDO2, false);
//also, configure wakeup from both button and tilt (see same example above)
//note, this wipes the memory, so will need to store the weather and steps in the RTC memory if space
//probably not possible.

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
