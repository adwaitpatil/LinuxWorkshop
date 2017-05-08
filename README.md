##### Setup

[Aster Carrier Board](https://www.toradex.com/products/carrier-boards/aster-carrier-board)

[Colibri VF61](https://www.toradex.com/computer-on-modules/colibri-arm-family/nxp-freescale-vybrid-vf6xx)

[Pioneer600](http://www.waveshare.com/wiki/Pioneer600)

<img src="https://github.com/SanchayanMaity/LinuxWorkshop/blob/master/images/aster-pioneer600.jpg" width="640" height="480">

##### Cloning this repository

```
git clone https://github.com/SanchayanMaity/LinuxWorkshop.git
```

##### Building and installing SDK

Build and install the OE-Core aka Yocto SDK, for instructions related to building and installing SDK, have a look at this knowledge base [article](http://developer.toradex.com/knowledge-base/linux-sdks).

To facilitate development, a sample SDK can be downloaded from this [link](https://share.toradex.com/erezkkl0c5yunve). Install the SDK by executing the following from the command line

```
chmod a+x angstrom-glibc-x86_64-armv7at2hf-neon-v2016.12-toolchain.sh
./angstrom-glibc-x86_64-armv7at2hf-neon-v2016.12-toolchain.sh
```

Please use the standard default /usr/local/oecore-x86_64 path for SDK installation. The Eclipse projects included in this repository assume that this path has been used for SDK installation. Any generated SDK and image used on the module must include "libsoc" library for the code samples to work. Additionally gtkmm code has dependency on gtkmm library.

##### Sourcing Environment Script and starting Eclipse
```
. /usr/local/oecore-x86_64/environment-setup-armv7at2hf-neon-angstrom-linux-gnueabi
```

Note the space between the dot and rest of the command. Now start Eclipse from the same terminal from where the above script was sourced.

```
eclipse &
```

Import the projects in Eclipse and build the project. Transfer the binary executable which will be available in Debug directory to the module using scp as follows, for example

```
scp <binary> root@<module_ip_address>:/home/root
```

##### Sample Output

###### gpio interrupt
```
root@colibri-vf:~# ./gpio_interrupt 
Enter the GPIO number to use as interrupt:      10
Enter the GPIO number used for LED output:      11
Waiting for interrupt. Press 'q' and 'Enter' at any time to exit
```

###### pcf8574
```
root@colibri-vf:~# ./pcf8574

Enter choice: 1. Joy stick status 2. Led On/Off 3. Buzzer On/Off 4. Exit
1
Key A pressed

Enter choice: 1. Joy stick status 2. Led On/Off 3. Buzzer On/Off 4. Exit
2

Enter choice: 1. Joy stick status 2. Led On/Off 3. Buzzer On/Off 4. Exit
2

Enter choice: 1. Joy stick status 2. Led On/Off 3. Buzzer On/Off 4. Exit
3

Enter choice: 1. Joy stick status 2. Led On/Off 3. Buzzer On/Off 4. Exit
3

Enter choice: 1. Joy stick status 2. Led On/Off 3. Buzzer On/Off 4. Exit
4
```

###### pcf8591
```
root@colibri-vf:~# ./pcf8591 
PCF8591 ADC and DAC test code

1. ADC 2. DAC 3. Exit
2
Enter DAC value:        78

1. ADC 2. DAC 3. Exit
1

Enter ADC channel number 0-3:   3

ADC Channel 3: value: 128

1. ADC 2. DAC 3. Exit
1

Enter ADC channel number 0-3:   3

ADC Channel 3: value: 78

1. ADC 2. DAC 3. Exit
1

Enter ADC channel number 0-3:   3

ADC Channel 3: value: 78

1. ADC 2. DAC 3. Exit
3
```

###### pwm
```
root@colibri-vf:~# ./pwm 
Enter PWM chip number:  0
Enter PWM number:       0
Enter PWM Period:       1000000
Enter PWM Duty Cycle:   500000
PWM will be running for 10 seconds
Disabling PWM
```
