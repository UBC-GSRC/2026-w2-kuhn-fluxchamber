# Work Log Procedure:

- Use this page to describe the nitty gritty of what has been done towards a project on a specific date.  

- Always use the date for each entry and tag your initials (YYYY-MM-DD AF).  

- Always put a new entry at the top of the page so you can see the newest work first.  

- If anything important occurs, note it in the 'Project Updates' in a more concise way than you would in this log.  

- Do not delete entries, this is meant to serve as a rudimentary version control and archive.  

- Leave the Procedure at the top of the page.  

## 2026-01-10 AF

I changed some of the logic to make sure that the ISR's work correctly for the reed switch. Basically, you can't be in a delay() and have an ISR work. So everything that takes a while was convereted to be a non-blocking wait using millis().

## 2026-01-06 AF

Watched some videos about using KiCAD9 and it was super helpful. Mostly just showing me the hidden things about using the software and the quality of life boosts. The more and more I learn about PCB design really freaks me out about making something that doesn't work. I feel like modularizing items benefits this. Arduino has done a great job at mastering this idea. I just kinda hate paying the huge premium to use their MCU hats. But I think I need to stop worrying about making only one PCB. Make a few. Ideally we minimize the amount of flywires in the system. But we benefit from modularizing as we don't need to worry about the complexity of PCB design when you start having mixed signals and all the sorts. Maybe it does get included with MCU hats but it feels better to me for some reason. 

I need to find a good video that explains ground currents better. I can't visualize it in the context of PCB's and nets of connections.

Additionally I think I should try to switch the K33 data transfer to UART as it is better designed for further cable length. 

## 2026-01-05 AF
Working mostly on putting this PCB together. I'm learning more about routing, using via's correctly, ground planes, etc. I wasn't convinced that I wanted to put the LM2596's on the board but I figure why not. This is just going to take some cute routing to make it work but that's fine. Just make sure that sensitive signals aren't via'd if possible. 

## 2026-01-29 AF

Did some INA226 testing and it seems that it's better to just get a more expensive module that can read higher amperage. Such as INA228 which can read up to 10A. This means we can get way more accurate energy consumption compared to the modelling method I proposed before. I cleaned up some schematics and tested the INA226 as well. 

## 2026-01-26 AF

I added some redesigns to the DIN clips. They are now in one configurable file. I made them less thick at the through holes, a little less tall, and some other things that I forget. 

I also looked into the INA226 which is a DC current measurement IC which can be used to determine the battery life of a battery. So that's coming in a few days. 

Also looked finally tried out LoRa which was fun. I was able to get readings going through the lab and elevator and up into the main part of Pond. It should be pretty easy to make a LoRa sniffer situation for monitoring the device from the shore. 

## 2026-01-23 AF

I've missed a few of these bad boys. But alas, a lot of things have changed. Now the data logger prototype is on a proper DIN rail test stand. I also made a bunch of clips that need some reworking. 

### Clip Reworks
    - I used the band saw to make the 51mm clips a little wider 
    - Some of the adapter plates need to be further away from the clips as the standoffs interfere 
    - Could reduce the width of the clips because it's pretty annoying to mount them only with gigantic screws
    - Also the clip insertion angle shouldn't need to be rotated. It's really annoying when the wires are pretty tight length clearance

I added a switch to the logger which puts it into either 'calibration' or 'log' mode. The calibration mode is what we do when we need to calibrate the sensors. You can also send serial commands in this mode. 'log' is for ultra low power and turns off peripherals and stuff so you can't listen to it. It just writes to an SD card. 

## 2026-01-16 DF
Installed temporary actuator and created test aparatus gantry. Will fill garbage bucket wiht water and test floating capability.
