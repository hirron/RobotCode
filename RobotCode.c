#pragma config(Sensor, in2,    InfraCollector1, sensorReflection)
#pragma config(Sensor, in3,    InfraCollector2, sensorReflection)
#pragma config(Sensor, dgtl1,  button1,        sensorTouch)
#pragma config(Sensor, dgtl2,  button2,        sensorTouch)
#pragma config(Sensor, dgtl5,  RSensor,        sensorTouch)
#pragma config(Sensor, dgtl6,  S3,             sensorSONAR_cm)
#pragma config(Sensor, dgtl8,  LSensor,        sensorTouch)
#pragma config(Sensor, dgtl11, AStateLED,      sensorDigitalOut)
#pragma config(Sensor, dgtl12, DStateLED,      sensorDigitalOut)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           motor2,        tmotorVex393_HBridge, openLoop, encoderPort, I2C_1)
#pragma config(Motor,  port2,           motor3,        tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           motor1,        tmotorVex393_MC29, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

bool button1_pushed; //flag to store button1 input
// 2 Global IR readings, they get updated by monitor input
int IR1High=4000;
int IR2High=4000;
int SensorAv=4000;
// detects if wheel has been turned in the past
bool WheelRotated=false;

bool beaconFound = false;




typedef enum{
	StartState=0,
	AlignState,
	MoveState,
	DropState,
	ParkState,
	EndState,
	DeadState,

}T_FSMState;

T_FSMState FSMState;







void Turn(int Dir)
{
	clearTimer(T1);
	//1 is a left turn, 0 is a right turn
	if(Dir==1){
		// turns front wheel
		while (time1[T1]<=500){
			motor[motor1]= 0;
			motor[motor2]= -50;
		}
		// goes forward, turning robot
		while (time1[T1]>=500&&time1[T1]<=1250){
			motor[motor1]= 55;
			motor[motor2]= 0;
		}
		// realigns front wheel to default state
		motor[motor1]= 0;
		while (time1[T1]<=1700&&time1[T1]>=1250){
			motor[motor1]= 0;
			motor[motor2]= 43;
		}
		motor[motor2]= 0;
		}else{

		// same thing in opposite direction
		while (time1[T1]<=500){
			motor[motor1]= 0;
			motor[motor2]= 50;
		}

		while (time1[T1]>=500&&time1[T1]<=1250){
			motor[motor1]= 55;
			motor[motor2]= 0;
		}
		motor[motor1]= 0;
		while (time1[T1]<=1700&&time1[T1]>=1250){
			motor[motor1]= 0;
			motor[motor2]= -43;
		}
		motor[motor2]= 0;
	}
}

// sets the front wheel to 90 degrees to enable on spot turning
void SpotRot(){
	// makes sure that front motor is deactivated while turning
	motor[motor1]=0;
	if(WheelRotated==false){
		while(SensorValue[I2C_1]<1150){
			motor[motor2]=-50;
		}
		}else{
		while(SensorValue[I2C_1]>110){
			motor[motor2]=50;
		}
	}	
	motor[motor2]= 0;
	WheelRotated=!WheelRotated;
}


void LEDREAD1(){
	clearTimer(T2);
	// Reads value of first led.
	// Check if 100 msecs have elapsed.
	IR1High=4000;
	while(time1[T2]<100){
		int lightLevel1 = SensorValue[InfraCollector1];
		if (lightLevel1<IR1High) {
			IR1High=lightLevel1;
		}
	}
}

// reads value of 2nd LED
void LEDREAD2(){
	clearTimer(T2);
	// Reads value of first led.


	IR2High=4000;
	// Check if 100 msecs have elapsed.
	while(time1[T2]<100){
		int lightLevel2 = SensorValue[InfraCollector2];
		if (lightLevel2 < IR2High) {
			IR2High=lightLevel2;
		}
	}
}

void monitorInput()
{

	if(SensorValue(button1) && !button1_pushed)
	{
		button1_pushed=true;
	}
	

	// reads both resistor loops
	LEDREAD1();
	LEDREAD2();
	
	if((SensorValue[LSensor]==true||SensorValue[LSensor])&&(FSMState!=ParkState&&FSMState!=EndState)){
			if(WheelRotated==true){
				SpotRot();
				}
			motor[motor1]=-50;
			delay(1000);
			motor[motor1]=0;
			
		}

	
}

void AmStuck(){
	int StartSonar=SensorValue[S3];
	delay(500);
	int EndSonar=SensorValue[S3];

	if((StartSonar-EndSonar)<4){
		motor[motor1]=-50;
		delay (1500);
		Turn(1);
	}
}






task main()
{
	button1_pushed = false;
	SensorValue[I2C_1]=0;

	while(true){

		switch(FSMState){
		default:
			FSMState=StartState;
			break;


			// checks to make sure that there is no contact with walls at start
		case StartState:
				monitorInput();
				FSMState=AlignState;// change this back to AlignState
				clearTimer(T1);
			break;


			// Detects peak of IR (when robot is best facing sensor)
		case AlignState:
			monitorInput();
			if(WheelRotated==false){
				SpotRot();
			}

			// helps to determine strength of signal, provides threshold
			SensorAv=(IR1High+IR2High)/2;
			// buffer added due to electrical difficulties
			if(IR1High>IR2High-350){
				motor[motor1]=0;
				motor[motor1]=20;
			}

			if(IR2High>IR1High-350){
				motor[motor1]=0;
				motor[motor1]=-20;
			}

			if(SensorAv<1000){
				SpotRot();
				// indicates that a beacon has been spotted
				SensorValue[AStateLED]=1;
				// a short delay before continuing to move
				delay(100);
				FSMState=MoveState;
			}

			// presuming beacon is not within detection range, robot will move forward for a set period of time
			if (time1[T1]>15000){
				SpotRot();
				delay(100);
				FSMState=MoveState;
			}

			break;



		case MoveState:
		if(time1(T1)>4500){
			clearTimer(T1);
		}


			monitorInput();
			SensorAv=(IR1High+IR2High)/2;
			motor[motor1]=30;
			if(/*SensorAv<3800&&*/SensorValue[S3]<15){
				motor[motor1]=0;
				delay(100);
				if(SensorValue[S3]<13){
					FSMState=DropState;
					break;
				}
			}
			//	AmStuck();

			if(time1(T1)>2000&&FSMState!=DropState){
				motor[motor1]=0;
				FSMState=AlignState;
				delay(100);
				SpotRot();
			}

			break;


			//Defines behavior when dropping object
		case DropState:
			clearTimer(T1);

			// waits a bit of time before dropping
			while(time1[T1]<1500){
				motor[motor1]=0;
			}

			while (time1[T1]>=1500&&time1[T1]<=1750){
				motor[motor1]= 0;
				motor[motor3]= -50;
			}
			//after letting go will reverse away from beacon
			while (time1[T1]>=1750&&time1[T1]<=2650){
				motor[motor1]= -50;
				motor[motor3]= 0;
			}

			motor[motor1]= 0;

			// transitions to looking to park
			FSMState=ParkState;

			//turns robot away from beacon so it won't bump on the way out
			Turn(1);

			break;

		case ParkState:
			monitorInput();
			//Turns robot left so that it doesn't run into beacon
			// if wall is too distant turn

			if(SensorValue[S3]<100&&SensorValue[S3]>20){
				motor[motor1]=40;
			}


			if(SensorValue[S3]>100){
				motor[motor1]=35;
			}


			// switches to an ending state at 20m, -1 prevents it from shutting down if sonar gets disconnected
			if(SensorValue[S3]<30&&SensorValue[S3]!=-1){
				FSMState=EndState;
			}


			break;


		case EndState:

			motor[motor1]=25;
			if(WheelRotated==true){
				motor[motor1]=-25;
				if(SensorValue[RSensor]&&!SensorValue[LSensor]){
					motor[motor1]=25;
				}
			}


			if(SensorValue[RSensor]==1||SensorValue[LSensor]==1){
				if(SensorValue[RSensor]==1&&SensorValue[LSensor]==1){
					motor[motor1]=0;
					FSMState=DeadState;
					break;
					}else if(WheelRotated==false){
					SpotRot();
				}



				break;


				// Robot stops all motors
			case DeadState:
				motor[motor2]=0;
				// functions to signal end of code
				motor[motor3]=100;
				if(WheelRotated==true){
					SpotRot();
				}
			}
		}
	}
}
