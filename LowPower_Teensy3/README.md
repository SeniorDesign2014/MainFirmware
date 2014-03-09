LowPower_Teensy3 Library
========================
If you are not using Teensyduino 1.14 or greater then there is one edit you need to make
to the mk20dx128.c core file: Add -> "PMC_REGSC |= 0x08;" just under "SCB_VTOR = 0;" in 
the ResetHandler function. This allows the mcu to release hold of the I/O when waking 
from sleep.

<h3>ChangeLog Beta2:</h3>
1.  Added struct to store sleep configurations<br>
2.  Added RTC Alarm wake<br>
3.  Added TSI wake<br>
4.  Put usb regulator in standby for VLPR<br>
5.  Got rid of stand alone LPTMR timer fucntions<br>
6.  Cleaned up the library and example codes<br>
7.  New examples added<br>
8.  Defined GPIO wake pin names<br>

<h3>ChangeLog Stable v1.0:</h3>
1.  Fixed where VLPR was not being retained becase of LPWUI bit not being set right<br>
2.  Added feature to enable LPWUI bit to exit VLPR with any interrupt<br>
3.  Fixed issue with VLPR locking up system if being called before exiting VLPR<br>
4.  Disabled USB Regulator Standby mode during Low Power<br>
5.  Cleaned up library code.<br>

<h3>ChangeLog beta v1.1:</h3>
1.  Added all digital wakeup capable pins for DeepSleep and Hibernate function<br>
2.  User can now use callback function for DeepSleep and Hibernate<br>
3.  Added Sleep function, now any interrupt can wake the processor<br>
4.  Improved code performance<br>
5.  New example for Sleep function<br>

<h3>ChangeLog beta v1.2:</h3>
1.  Using Bitband to set peripheral clocks<br>
2.  Added support for IntervalTimer, delay, delayMicroseconds for dynamic CPU scaling<br>
3.  Added support for HardwareSerial at 16,8,4,2 MHz<br>
