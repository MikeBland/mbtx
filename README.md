# mbtx
er9x/ersky9x radio firmware
   
## IMPORTANT NOTICE - TURNIGY 9XR   
Hobbyking/Turnigy ave just released a radio called 9XR which has a port for a programmer and an installed backlight. This is a very welcome addition since it makes programming the radio very easy for the novice. That being said, they have not given credit to the originators and contributors and they have not released the code as required by the GNU 2.0 software license. 
So while we are very happy to see them make the 9XR radio we do want them to acknowledge the hard work and the effort made by all the contributors. Please let them know how you feel about this. 
We have tried to contact them but have not been able to receive any response. We'll update here if and when we get one.
   
HobbyKing have acknowledged their use of er9x on the 9XR.

The Eurgle/FlySky/Imax/Turnigy 9x r/c Transmitter is a nicely built unit basically copied from a more recognized manufacturer.

The unit itself hosts an Atmel AtMega64 8-bit MCU. The hardware includes 2 sticks, 3 potentiometers and 7 switches. It also supports a 128x64px screen without a back light.

This project aims to replace the software that comes with the standard transmitter with a more powerful and flexible version.

This project is based on he th9x project. http://code.google.com/p/th9x/



**Support Forum**
Thanks to an enterprising member we have a support forum:
http://www.OpenRcforums.com
Please join and use for all your questions, suggestions and discussions regarding the er9x/eepe platform.



**Like er9x? Try the accompanying editor and simulator - eePe!**
homepage: http://code.google.com/p/eepe/
Windows installer: http://eepe.googlecode.com/svn/trunk/eePeInstall.exe 
eePe Quickstart video: http://www.youtube.com/watch?v=ZNzRAPGo8uY



**WARNING! Deleting a model causes the memory to jump to the previous model memory in the list. Do not delete a model memory while you have a model "listening". Always shut down your receiver before deleting a model.**



eePe and er9x are free to use under the GNU v2.0 License. Feel free to use, copy and modify it as you wish!
I have spent a lot of time (and will continue to) to make this software as good as possible. If you feel that this software has been beneficial to you please show your support by donating. This will be greatly appreciated and you'll be added to the "contributors" list in the code (optional of-course).
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=YHX43JR3J7XGW"><img src="https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif" alt="[paypal]" /></a> 

 

## Help! Where do I start?
<ul><li><a href="https://openrcforums.com/forum/viewtopic.php?f=5&t=6473#p90349" >ER9x User guide - WIKI</a> - This is the best place to start. </li><li><a href="http://OpenRcforums.com/wiki/index.php/Flashing_your_9x" >Flashing your transmitter with er9x</a> </li><li><a href="http://OpenRcforums.com/wiki/index.php/Er9x_firmware_information" >What FW do I choose?</a> </li><li><a href="http://www.OpenRcforums.com" >OpenRcforums.com</a> - BEST PLACE TO ASK QUESTIONS! </li></ul>


## Latest Binaries
<a href="http://OpenRcforums.com/wiki/index.php/Er9x_firmware_information" >What FW do I choose?</a>

<p><i>To download, right-click and "save-as"</i><br> <a href="http://er9x.googlecode.com/svn/trunk/er9x.hex" >ER9x</a> <i>&lt;- If you are unsure, choose this one!</i><br> <a href="http://er9x.googlecode.com/svn/trunk/er9x-noht.hex" >ER9x-NOHT</a><br> <a href="http://er9x.googlecode.com/svn/trunk/er9x-frsky.hex" >ER9x-FRSKY</a><br> <a href="http://er9x.googlecode.com/svn/trunk/er9x-frsky-noht.hex" >ER9x-FRSKY-NOHT</a><br> <a href="http://er9x.googlecode.com/svn/trunk/er9x-jeti.hex" >ER9x-JETI</a><br> <a href="http://er9x.googlecode.com/svn/trunk/er9x-ardupilot.hex" >ER9x-ARDUPILOT</a><br> <a href="http://er9x.googlecode.com/svn/trunk/er9x-nmea.hex" >ER9x-NMEA</a> </p>



## ER9x Manual
<a href="https://openrcforums.com/forum/viewtopic.php?f=5&t=6473#p90349" >NEW-WIKI fully updated</a>

<p><i>old versions of the manual - for reference</i><br> <a href="http://er9x.googlecode.com/svn/trunk/doc/ER9x%20Users%20Guide.pdf" >ER9x Manual - English</a><br> <a href="http://er9x.googlecode.com/svn/trunk/doc/ER9x%20Users%20Manual%20Francais.pdf" >ER9x Manual - French</a> <br> <a href="http://er9x.googlecode.com/svn/trunk/doc/ER9x%20Manual%20Aeromodelitis.pdf" >ER9x Manual - Spanish</a> (by Luis Llaberia  <a href="http://www.aeromodelitis.com/" >www.aeromodelitis.com/</a>) <br> <a href="http://er9x.googlecode.com/svn/trunk/doc/ER9x%20Users%20Manual_ptbr.pdf" >ER9x Manual - Portuguese</a> (by Alexandre Magalhaes  <a href="http://www.e-voo.com/" >http://www.e-voo.com/</a>) <br> <a href="http://er9x.googlecode.com/svn/trunk/doc/ER9x%20RUS%20Manual.pdf" >ER9x Manual - Russian</a> (by Dmitry Bugaevsky) <br> <a href="http://er9x.googlecode.com/svn/trunk/doc/ER9x_Users_Manual_Hu_v1.pdf" >ER9x Manual - Hungarian</a> (by Hegyi István) <br> <a href="http://er9x.googlecode.com/svn/trunk/doc/Er9x%20user%20guide%20-%20tc.pdf" >ER9x Manual - Traditional Chinese</a> (by Henry Liao) <br><br>   </p>

## Sources
**LATEST**: svn checkout http://er9x.googlecode.com/svn/trunk/ er9x  
**STABLE**: svn checkout http://er9x.googlecode.com/svn/trunk/ er9x -r 708 

## Documentation and Tutorials
<p>Sample <a href="http://code.google.com/p/eepe/" rel="nofollow">.eepe</a> files can be downloaded at the following link: </p>

<a href="http://code.google.com/p/er9x/downloads/list" rel="nofollow">http://code.google.com/p/er9x/downloads/list</a>

<ul><li>Video tutorial by richardmrazek: </li><ul><li><a href="http://www.youtube.com/watch?v=2Yv2rCxfjSk" rel="nofollow">How to wire radio for flashing</a> </li><li><a href="http://www.youtube.com/watch?v=VVvOLneNinQ" rel="nofollow">How to install a backlight</a> </li><li><a href="http://www.youtube.com/watch?v=q2UT1pFcYhM" rel="nofollow">Part 1</a> </li><li><a href="http://www.youtube.com/watch?v=vpgGnb1ROP8" rel="nofollow">Part 2</a> </li><li><a href="http://www.youtube.com/watch?v=EjJMgR41GKc" rel="nofollow">Part 3</a> </li><li><a href="http://www.youtube.com/watch?v=h-1mGs-vivE" rel="nofollow">Part 4</a> </li></ul></ul>

<ul><li><a href="http://er9x.googlecode.com/svn/trunk/doc/Flashing%20the%209x.pdf" rel="nofollow">Flashing the 9x</a> by Jon Lowe. </li><ul><li><a href="http://er9x.googlecode.com/svn/trunk/doc/Appendix%201.pdf" rel="nofollow">Flashing the 9x - Appendix 1</a> by Jon Lowe. </li><li><a href="http://er9x.googlecode.com/svn/trunk/doc/Alternate%209x%20hookup.pdf" rel="nofollow">Alternate 9x hookup.pdf</a> by Jon Lowe. </li><li><a href="http://er9x.googlecode.com/svn/trunk/doc/9x-addon-instructions.pdf" rel="nofollow">Smartieparts 9x Add-On Board Installation Howto</a> by ghost. </li><li><a href="http://www.rcgroups.com/forums/showthread.php?t=1338412#post16549680" rel="nofollow">Smartieparts 9x Add-On Board Review</a>  By me :) </li><li><a href="http://code.google.com/p/th9x/wiki/installation_de" rel="nofollow">Flysky 9x / AVR pinout</a> courtesy of thus.  </li></ul><li>FrSky Telemetry Mods.
</li><ul><li><a href="http://er9x.googlecode.com/svn/trunk/doc/FrSky%20Telemetry%20%20details.pdf" rel="nofollow">FrSky Telemetry  details</a> by Phil. </li><li><a href="http://code.google.com/p/gruvin9x/wiki/FrskyInterfacing" rel="nofollow">FrSky interfacing tutorial</a> by gruvin (gruvin made an excellent tutorial that's valid for er9x as well as his own flavor). </li><li><a href="https://github.com/MikeBland/er9x/raw/master/doc/TelemetryMods.pdf">FrSky Telemetry mods</a> by Mike Blandford. </li><li><a href="http://www.flickr.com/photos/erezraviv/5830896454/in/photostream" rel="nofollow">Connecting the SF RS232-TTL converter</a>. </li><li><a href="http://er9x.googlecode.com/svn/trunk/doc/FRSKYTelemetry.pdf" rel="nofollow">FRSKY telemetry guide</a> by Rob Thomson. </li></ul><li><a href="http://er9x.googlecode.com/svn/trunk/doc/Er9x450helisetup.pdf" rel="nofollow">450 Heli Setup Tutorial</a> by ghost. </li><ul><li><a href="http://er9x.googlecode.com/svn/trunk/doc/hk450_heli.hex" rel="nofollow">The accompanying HEX file</a> by ghost. </li><li><a href="http://er9x.googlecode.com/svn/trunk/doc/HeliSetup_Template.pdf" rel="nofollow">Newer heli template tutorial</a> by ghost.  </li><li><a href="http://er9x.googlecode.com/svn/trunk/doc/Er9x%20450Helisetup%20Aeromodelitis.pdf" rel="nofollow">450 Heli Setup (Spanish)</a> by Luis Llaberia <a href="http://www.aeromodelitis.com/" rel="nofollow">www.aeromodelitis.com/</a> </li></ul><li><a href="http://er9x.googlecode.com/files/4Channel%20Tutorial.pdf" rel="nofollow">Simple 4 Channel Tutorial</a> by ghost. </li><li><a href="http://er9x.googlecode.com/files/Elevon%20and%20V-Tail.pdf" rel="nofollow">Elevon and V-Tail mixing tutorial</a> by Boxhead. </li><li><a href="http://er9x.googlecode.com/svn/trunk/doc/Compile%20Tutorial.pdf" rel="nofollow">Windows Compile Tutorial</a> by Telemachus. </li><li><a href="http://er9x.googlecode.com/svn/trunk/doc/Backlight%20Installation.pdf" rel="nofollow">EL Backlight Installation</a> by Earl. </li><li><a href="http://www.youtube.com/watch?v=wGKzQ9qLiwQ" rel="nofollow">Solderless "pogo" board interface</a>  by Telemachus. </li><li><a href="http://er9x.googlecode.com/svn/trunk/doc/Fuse%20Brick.pdf" rel="nofollow">Dealing with a "Fuse Brick"</a>  by Telemachus. </li><li><a href="http://er9x.googlecode.com/svn/trunk/doc/PPMinFix.pdf" rel="nofollow">PPM fix for the 9x</a> by J.H. </li><li><a href="http://www.youtube.com/watch?v=28-47SuDhT4" rel="nofollow">Trainer resistor fix</a> by ilektron. </li><li><a href="http://er9x.googlecode.com/svn/trunk/doc/ArduPilotLCDdisplayDoc.pdf" rel="nofollow">ArduPilot Display</a> by uphiearl. </li><li><a href="http://er9x.googlecode.com/svn/trunk/doc/Er9x_examples.zip" rel="nofollow">ER9x Examples</a> by ghost. </li><li><a href="http://www.rcgroups.com/forums/showpost.php?p=18759154&amp;postcount=7961" rel="nofollow">Short tutorial on how to use Flight Mode Trims</a>. </li><li><a href="http://er9x.googlecode.com/svn/trunk/doc/EEpeFirstTime.pdf" rel="nofollow">EEPE First time user's guide</a> by ghost. </li><li><a href="http://er9x.googlecode.com/svn/trunk/doc/NMEA.pdf" rel="nofollow">NMEA (GPS) how to</a> by Reinhard Strauch. </li><li><a href="http://www.youtube.com/watch?v=533OHEY-RrM" rel="nofollow">Variable travel rates</a> by rogueqd. </li></ul>


**DOWNLOAD THE MANUAL HERE**
<a href="http://er9x.googlecode.com/svn/trunk/doc/ER9x%20Users%20Guide.pdf" rel="nofollow">ER9x Manual</a>

## Build and Program Instructions

**Full programing instructions**
<a href="http://er9x.googlecode.com/svn/trunk/doc/Flashing%20the%209x.pdf" rel="nofollow">Flashing the 9x</a> by Jon Lowe.

**Required libraries**
avr-gcc, avr-libc, gcc, avrdude, libusb, bulid-essentials
Ubuntu/Debian commandline: sudo apt-get install avr-gcc avr-libc gcc avrdude libusb bulid-essential ruby

**Building from Source**
First checkout using svn: svn checkout http://er9x.googlecode.com/svn/trunk/ er9x
Enter the src/ directory.
To make the standard version type: make
To make the JETI DUPLEX enabled version type: make EXT=JETI
<a href="http://er9x.googlecode.com/svn/trunk/doc/Compile%20Tutorial.pdf" rel="nofollow">Windows Compile Tutorial</a> by Telemachus.

**Flashing (you may have to run as admin to access the USB port in Linux)**
To write the FW: make wflash AVRDUDE_PROGRAMMER=usbasp
To write the EEPROM: make weeprom AVRDUDE_PROGRAMMER=usbasp
To read FW: make rflash AVRDUDE_PROGRAMMER=usbasp TARGET=backupflash
To read the EEPROM: make reeprom AVRDUDE_PROGRAMMER=usbasp TARGET=backupeeprom
Make sure you replace "usbasp" with the name of your programmer.
To list available programmers type: avrdude -c ?

**make targets**
* make all (default): build the source
* make clean: Remove compiled files and directories.
* make wflash: Write flash (program) memory.
* make rflash: Read flash memory.
* make weeprom: Write eeprom.
* make reeprom: Read eeprom.
* make coff: Convert ELF to AVR COFF.
* make extcoff: Convert ELF to AVR Extended COFF.
* make debug: Start either simulavr or avarice as specified for debugging, with avr-gdb or avr-insight as the front end for debugging. (for debug info look into the makefile)
* make filename.s: Just compile filename.c into the assembler code only.

**make options**
* EXT=JETI: make jeti vesion.
* AVRDUDE_PROGRAMMER: Set avr programmer name (to list all available: avrdude -c ?) - default: usbasp
* TARGET: Set target name - default: er9x
* OPT: Set optimization level - default: s
* FORMAT: Set format (can be srec, ihex, binary) - default: ihex
* MCU: Set MCU - default: atmega64

**Making your own splash screen**
* Create a XBM bitmap file that is monochrome and 128x64 in size. (GIMP is a good program for handling XBM files).
* Save your file as "s9xsplash.xbm" in the src folder.
* Run "make s9xsplash.lbm" (this will convert the XBM file to the lbm format)
* Run "make" with whatever options you need.
* Once "make" finishes - the FW will be in the er9x.hex file in the src directory.
*Please note that in order to compile this under windows you need ruby*


## License and Disclaimer
This software is provided under the GNU v2.0 License. All relevant restrictions apply including the following. In case there is a conflict, the GNU v2.0 License is overriding.
This software is provided as-is in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. In no event will the authors and/or contributors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software.
2. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
3. Altered versions must be plainly marked as such, and must not be misrepresented as being the original software.
4. This notice may not be removed or altered from any distribution.  

By downloading this software you are agreeing to the terms specified in this page and the spirit of thereof.

