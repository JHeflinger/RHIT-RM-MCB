#include "tap/algorithms/smooth_pid.hpp"
#include "tap/board/board.hpp"
#include "drivers_singleton.hpp"
#include <cmath>

static constexpr tap::motor::MotorId MOTOR_ID1 = tap::motor::MOTOR1;
static constexpr tap::motor::MotorId MOTOR_ID2 = tap::motor::MOTOR2;
static constexpr tap::motor::MotorId MOTOR_ID3 = tap::motor::MOTOR3;
static constexpr tap::motor::MotorId MOTOR_ID4 = tap::motor::MOTOR4;

static constexpr tap::can::CanBus CAN_BUS = tap::can::CanBus::CAN_BUS1;
static constexpr int DESIRED_RPM = 9000;

tap::arch::PeriodicMilliTimer sendMotorTimeout(2);
tap::algorithms::SmoothPid pidController(20, 0, 0, 0, 8000, 1, 0, 1, 0);
tap::motor::DjiMotor motor1(::DoNotUse_getDrivers(), MOTOR_ID1, CAN_BUS, false, "cool motor 1");
tap::motor::DjiMotor motor2(::DoNotUse_getDrivers(), MOTOR_ID2, CAN_BUS, false, "cool motor 2");
tap::motor::DjiMotor motor3(::DoNotUse_getDrivers(), MOTOR_ID3, CAN_BUS, false, "cool motor 3");
tap::motor::DjiMotor motor4(::DoNotUse_getDrivers(), MOTOR_ID4, CAN_BUS, false, "cool motor 4");


int main()
{
    /*
     * NOTE: We are using DoNotUse_getDrivers here because in the main
     *      robot loop we must access the singleton drivers to update
     *      IO states and run the scheduler.
     */
    ::Drivers *drivers = ::DoNotUse_getDrivers();

    Board::initialize();
    drivers->remote.initialize();
    drivers->leds.init();
    drivers->can.initialize();
    motor1.initialize();
    motor2.initialize();
    motor3.initialize();
    motor4.initialize();

    float LEFT_STICK_VERT = 0;
    float LEFT_STICK_HORIZ = 0;
    int MAX_RPM = 6000;
	
    int LEFT_RAIL_RPM = 0;
    int RIGHT_RAIL_RPM = 0;
    float STICK_DIST = 0;

    while (1)
    {
        drivers->remote.read();
    
	if(drivers->remote.isConnected()){
	
    		LEFT_STICK_VERT = drivers->remote.getChannel(drivers->remote.Channel::LEFT_VERTICAL);
    		LEFT_STICK_HORIZ = drivers->remote.getChannel(drivers->remote.Channel::LEFT_HORIZONTAL);
   		STICK_DIST = (float)(sqrt((LEFT_STICK_VERT * LEFT_STICK_VERT) + (LEFT_STICK_HORIZ * LEFT_STICK_HORIZ)));

		if(STICK_DIST > 1){
			STICK_DIST = 1;
		}
		//now you have distance, ratio it lol
		if(LEFT_STICK_HORIZ > 0){
			LEFT_RAIL_RPM = LEFT_STICK_VERT * MAX_RPM;
			RIGHT_RAIL_RPM = (1 - LEFT_STICK_HORIZ) * LEFT_RAIL_RPM;
		}else if(LEFT_STICK_HORIZ < 0){
			RIGHT_RAIL_RPM = LEFT_STICK_VERT * MAX_RPM;
			LEFT_RAIL_RPM = (1 + LEFT_STICK_HORIZ) * RIGHT_RAIL_RPM;
		}else{
			LEFT_RAIL_RPM = LEFT_STICK_VERT * MAX_RPM;
			RIGHT_RAIL_RPM = LEFT_RAIL_RPM;
		}

		if (sendMotorTimeout.execute()){
			//motor 1
	       		pidController.runControllerDerivateError(LEFT_RAIL_RPM - motor1.getShaftRPM(), 1);
          		motor1.setDesiredOutput(static_cast<int32_t>(pidController.getOutput()));
            		
			//motor 2
         		pidController.runControllerDerivateError(LEFT_RAIL_RPM - motor2.getShaftRPM(), 1);
            		motor2.setDesiredOutput(static_cast<int32_t>(pidController.getOutput()));
	
			//motor 3
         		pidController.runControllerDerivateError((-1 * RIGHT_RAIL_RPM) - motor3.getShaftRPM(), 1);
            		motor3.setDesiredOutput(static_cast<int32_t>(pidController.getOutput()));

			//motor 4
			pidController.runControllerDerivateError((-1 * RIGHT_RAIL_RPM) - motor4.getShaftRPM(), 1);
            		motor4.setDesiredOutput(static_cast<int32_t>(pidController.getOutput()));
            			
			drivers->djiMotorTxHandler.processCanSendData();
       		}	
		drivers->canRxHandler.pollCanData();
	}

	modm::delay_us(10);
    }
    return 0;
}  
