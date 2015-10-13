replace the adc.h file with the one in this zip file.

It's located it:

C:\Program Files (x86)\Arduino\hardware\arduino\sam\system\libsam\include


hall effect sensor
current sensor gain = 12.8 mV/A +/- 1 mV
error = 2 mV = 156.25 mA
levels = 12
LSB V = 3.3 / (2^12-1) = 805.9 uV
error bits = error / LSB V = 2.48
filter points = (error bits * 2)^2 = 24.6
25 point averaging filter noise reduction = 0.2
averaged error bits = 0.496
averaged error voltage = 3.3 V * (0.496 / (2^12-1)) = 399.7 uV
average error current = 31.23 mA


Motor Controller
current amp gain = 60 A/V
input error tolerance = average error current / current amp gain = 520.5 uV

PWM Throttle
0 - 1.65 V
levels = 1.65 V / ( 2 * input error tolerance) = 1585.2
bits = log2(levels) = 10.63
bits usable = 10

voltage LSB = 806.06 uV
current LSB = 96.77 mA
measured LSB per controlled LSB = 1.5371


initial error bits new = 25 A * current sensor gain / 805.9 uV = 397.1
initial error bits old = 25 A * current sensor gain / 3.2258 mV = 99.2
ratio IE = 0.2498
sampling rate new = 100.0 Hz
sampling rate old = 4.762 Hz
ratio SR = 0.0476
P gain old = 0.1

reset time new = 0.1 s = 10 cycles
reset time old = 10 s = 48 cycles
I gain old = 0.01
I gain new = 0.048
P gain new = 0.0048

Pg = Ig * reset time