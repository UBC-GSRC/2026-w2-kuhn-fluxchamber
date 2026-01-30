# Work Log Procedure:

- Use this page to describe the nitty gritty of what has been done towards a project on a specific date.  

- Always use the date for each entry and tag your initials (YYYY-MM-DD AF).  

- Always put a new entry at the top of the page so you can see the newest work first.  

- If anything important occurs, note it in the 'Project Updates' in a more concise way than you would in this log.  

- Do not delete entries, this is meant to serve as a rudimentary version control and archive.  

- Leave the Procedure at the top of the page.  

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
