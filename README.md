# SunAlarm
SunAlarm is a simple alarm clock that simulates sunrise so all of us narcoleptics can somehow get out of the bed.  
And, yeah, it is nothing new - you can buy that at Amazon for 50$ and it will also play you a music at wake-up time. This project is for those who have some spare STM32 dev boards and other electronic material (like I do) and/or want to have some customization (like special light source, settable light intensity, nigh-lamp, ...).
  
Currently 12V LED strip is controlled via usual BJT transistor, but any kind (within transistor/wiring limits) light source can be used.

Setup button:
* Single press: toggle alarm on/off; Rot. encoder: changes alarm time
* Long press: enter/exit setup menu; Rot. encoder: changes current setup value

Light button:
* Single press: toggle night light on/off; Rot. encoder: / OR changes alarm time if alarm is enabled
* Holding button and rotating rot. encoder: changes current indensity value, if light is enabled
