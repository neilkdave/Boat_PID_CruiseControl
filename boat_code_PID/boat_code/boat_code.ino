#include<RunningMedian.h>
#include <pwm01.h>
#include <Timer.h>

// Variables for general control
        const int pwm_freq = 40000;
        const int serialBaudRate = 115200;        
        int Tlast = 0;
        int Tnow = 0;
        const int loopInterval = 75; //Milliseconds, 
        const int nFilter = 25; //Number of samples looked at by Running Median
        const int loopRead = 5; 
        
//Timer used so control loop is called every interval sending output signal simultaneously.
//This is impossible with build in Delay IDE where a delay holds up all microcontroller functions.
//This way current can be calcualted while sending a steady output signal to the motor controllers

Timer t;

// Variable IDs for I/O
  	const int Analog_In_Manual_Throttle = A0;			// Analog Input : Manual throttle
  	const int Analog_In_Battery_Current= A1;			// Analog input :Battery measured current
        const int Analog_In_Battery_Voltage_1 = A2; 
        const int Analog_In_Battery_Voltage_2 = A3;
        const int Analog_In_Battery_Voltage_3 = A4;
        
  	const int Analog_Out_Motor_Throttle = 9;			// Analog output : Motor controller throttle
  	
        const int Digital_In_Throttle_Mode = 7;				// Digital input : Throttle auto or manual
  	const int Digital_In_Timer_Mode = 8;				// Digital input : Timer on or off
        
        const int ARresolution = 12; //Read Resolution
        const int AWresolution = 10; //Write resolution
          
//PWM setup
        
        
// Variables for current measurement
        const float bits_to_amps = 0.252; 
        const float zero_amps = (1 << (ARresolution - 2)) - 1;
        RunningMedian Mcurrent = RunningMedian(nFilter);
        
// Variables for voltage measurement
        const float bits_to_volts_1 = 1;
        const float bits_to_volts_2 = 1;
        const float bits_to_volts_3 = 1;
        RunningMedian Mvoltage1 = RunningMedian(nFilter);
        RunningMedian Mvoltage2 = RunningMedian(nFilter);
        RunningMedian Mvoltage3 = RunningMedian(nFilter);
        
// Variables for control loop
	boolean Control_Mode = LOW;				// HIGH = auto, LOW = manual	

	const float Gain_Proportional = .04;				// Control loop proportional gain
	const float Gain_Integral = .00004;					// Control loop integral gain
        const float Gain_Derivative = 0.4;
	const float Battery_Current_Ideal = zero_amps - (25 * .0128 / 3.3)*((1 << (AWresolution - 1)) - 1);			// Ideal battery current1
	
        float Battery_Current = 0.0;
        float Error_Proportional = 0.0;				// Present current error
	float Error_Integral = 0.0;					// Current error integral
        float Error_Derivative = 0.0;
        float Error_Last = 0.0;
        
	float Throttle_Derivative = 0;
	int Throttle_value = 0;					// Control present


        RunningMedian Control_Manual = RunningMedian(nFilter);
        int manual_throttle_iter = 0;
        const int max_throttle = (1 << (AWresolution - 1)) - 1;
        
// Timing Variables
        boolean Timer_Mode = LOW;
        int sys_time = 0;
        int timer_time = 0;
	const int refresh = 100;				// milliseconds. loop delay
        




void take_measurements() {
  for (int i = 0; i < loopRead; i++) {
// read manual throttle
        Control_Manual.add(analogRead(Analog_In_Manual_Throttle));
// Read battery current
        Mcurrent.add(analogRead(Analog_In_Battery_Current));
// Read battery voltage 1
        Mvoltage1.add(analogRead(Analog_In_Battery_Voltage_1));
// Read battery voltage 2
        Mvoltage2.add(analogRead(Analog_In_Battery_Voltage_2));
// Read battery voltage 3
        Mvoltage3.add(analogRead(Analog_In_Battery_Voltage_3));
  }
// Read Control mode
        Control_Mode = digitalRead(Digital_In_Throttle_Mode);
// Read timer Mode
        Timer_Mode = digitalRead(Digital_In_Timer_Mode);
}

void control() {
        sys_time += 1;
        if (Timer_Mode) {
          timer_time += 1;
        }
        manual_throttle_iter = (int)Control_Manual.getAverage();
        Battery_Current = Mcurrent.getAverage();
        if (! Control_Mode) {
          Error_Integral = 0;
          Throttle_value = manual_throttle_iter;
        } else {
          Error_Last = Error_Proportional;
          Error_Proportional = -Battery_Current_Ideal +Battery_Current;
	  Error_Integral += Error_Proportional;
          Error_Derivative = Error_Proportional - Error_Last;
          
          Throttle_Derivative = Gain_Proportional * Error_Proportional +
                                Gain_Integral * Error_Integral +
                                Gain_Derivative * Error_Derivative;

          Throttle_value += Throttle_Derivative;
        }

        if (Throttle_value > max_throttle) {
          Throttle_value = max_throttle;
        } else if (Throttle_value < 0) {
          Throttle_value = 0;
        }
}


void loop(){
  t.update();
}
void loopInstance() {
        take_measurements();
        control();

// Write control function result to motor control output
	pwm_write_duty(Analog_Out_Motor_Throttle , (int)Throttle_value);
        
        Tlast = Tnow;
        Tnow = micros();
        
        Serial.print("TD=");
        Serial.print(Tnow - Tlast);
    
        Serial.print(" TM=");
        Serial.print(Timer_Mode);
        Serial.print(" SC=");
        Serial.print(sys_time);
        Serial.print(" TC=");
        Serial.print(timer_time);
        
	Serial.print(" VC=");
	Serial.print(Battery_Current);

        Serial.print(" CM=");
        Serial.print(Control_Mode);
	Serial.print(" TA=");
        Serial.print(Throttle_value);
        Serial.print(" TM=");
        Serial.print(manual_throttle_iter);

        Serial.print("EP=");
        Serial.print(Error_Proportional);
        Serial.print(" EI=");
        Serial.print(Error_Integral);
        
	// sleep...
	//delay(refresh);	
}


void setup(){
  
  
	// begin sending over serial port
	Serial.begin(serialBaudRate);

	// declaration of pin modes
	pinMode(Analog_Out_Motor_Throttle, OUTPUT);

	pinMode(Analog_In_Manual_Throttle , INPUT);
	pinMode(Analog_In_Battery_Current, INPUT);
	pinMode(Digital_In_Throttle_Mode , INPUT);
	pinMode(Digital_In_Timer_Mode , INPUT);
        
        pwm_setup(Analog_Out_Motor_Throttle, pwm_freq, 1);
        
        analogReadResolution(ARresolution);
        pwm_set_resolution(AWresolution);
        
        t.every(loopInterval, loopInstance);
        
}
