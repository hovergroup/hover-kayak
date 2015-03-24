Pinout information:
GPIO 5                  increases TX volume level
GPIO 6                  decreases TX volume level

GPIO 1                  Will go high when MM is in RX
GPIO 2                  Will go high when MM is in TX
GPIO 4                  Power Cycle the Modem by holding this for 6-15 secs.
GPIO 7                  Line will go high when battery voltage is low.
 

Information about power control:
http://www.intersil.com/en/products/data-converters/digital-potentiometers--dcps-/dcps/ISL22511.html#0.html

Pull pin low to increase/decrease volume.  
Pin must be high at least 2ms between button presses.  (gap time)
Pin must be held low for greater than 15ms typical, 28ms max to increase/decrease volume (debounce time)

Tslow = 100ms min, 250 typ, 390 max
Tfast = 25ms min, 50 typ, 78 max

For the first second after the debounce time, volume will change by one every Tslow
Following the first second, volume will change by one every Tfast
There are 32 volume levels.
Assuming maximum times, holding a pin low for 4 seconds is more than enough time to cover all 32 steps.  


Shell scripts can be executed from within a c++ program using the command:
system("./script.sh")
included from <stdlib.h>

System will return the value the script returned, multiplied by 256.  So ending the script with "exit 1" will cause the system call to return 256.  

 

 ---

 #!/bin/bash -f

for i in 1 2 3 4 5 6 7 8
do
  echo "Testing GPIO $i"

  echo "Turning on"
  read -p "Press any key to continue... " -n1 -s
  echo out > /gpio/boardio$i/direction
  echo 1 > /gpio/boardio$i/value

  echo ""

  echo "Turning off"
  read -p "Press any key to continue... " -n1 -s
  echo 0 > /gpio/boardio$i/value

  echo ""

  echo "Reading Value"
  read -p "Press any key to continue... " -n1 -s
  echo ""
  echo in > /gpio/boardio$i/direction
  sleep 1
  cat /gpio/boardio$i/value

  echo ""
  read -p "Press any key to continue... " -n1 -s
  echo ""
done