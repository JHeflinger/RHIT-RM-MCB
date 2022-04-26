#include "tap/algorithms/smooth_pid.hpp"
#include "tap/board/board.hpp"
#include "drivers_singleton.hpp"
#include <cmath>

static constexpr tap::motor::MotorId MOTOR_ID1 = tap::motor::MOTOR1;
static constexpr tap::motor::MotorId MOTOR_ID2 = tap::motor::MOTOR2;
static constexpr tap::motor::MotorId MOTOR_ID3 = tap::motor::MOTOR3;
static constexpr tap::motor::MotorId MOTOR_ID4 = tap::motor::MOTOR4;

static constexpr tap::can::CanBus CAN_BUS = tap::can::CanBus::CAN_BUS1;
static constexpr int DESIRED_RPM = 12000;

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
    float RIGHT_STICK_HORIZ = 0;
    float RIGHT_STICK_VERT = 0;
    int MAX_RPM = 0;
    int CURR_RPM = 0;
	
    int LEFT_RAIL_RPM = 0;
    int RIGHT_RAIL_RPM = 0;
    
    float percent_LR = 0;
    float percent_RR = 0;
    
    int mode = 0; //double stick is mode 2, single stick is mode 1, gimble override is mode 0

    while (1)
    {
        drivers->remote.read();
    
	if(drivers->remote.isConnected()){
	
		//set max rpm to left switch value
		if(drivers->remote.getSwitch(drivers->remote.Switch::LEFT_SWITCH) == drivers->remote.SwitchState::MID){
			MAX_RPM = (DESIRED_RPM * 6)/10;
		}else if(drivers->remote.getSwitch(drivers->remote.Switch::LEFT_SWITCH) == drivers->remote.SwitchState::UP){
			MAX_RPM = (DESIRED_RPM * 8)/10;
		}else if(drivers->remote.getSwitch(drivers->remote.Switch::LEFT_SWITCH) == drivers->remote.SwitchState::DOWN){
			MAX_RPM = (DESIRED_RPM * 4)/10;
		}else{
			MAX_RPM = (DESIRED_RPM * 2)/10;
		}
	
		//set mode based on right switch
		if(drivers->remote.getSwitch(drivers->remote.Switch::RIGHT_SWITCH) == drivers->remote.SwitchState::MID){
			mode = 1;
		}else if(drivers->remote.getSwitch(drivers->remote.Switch::RIGHT_SWITCH) == drivers->remote.SwitchState::UP){
			mode = 2;
		}else if(drivers->remote.getSwitch(drivers->remote.Switch::RIGHT_SWITCH) == drivers->remote.SwitchState::DOWN){
			mode = 0;
		}else{
			mode = 0;
		}
	
		//update stick values
		RIGHT_STICK_VERT = drivers->remote.getChannel(drivers->remote.Channel::RIGHT_VERTICAL);
		RIGHT_STICK_HORIZ = drivers->remote.getChannel(drivers->remote.Channel::RIGHT_HORIZONTAL);
    		LEFT_STICK_VERT = drivers->remote.getChannel(drivers->remote.Channel::LEFT_VERTICAL);
    		LEFT_STICK_HORIZ = drivers->remote.getChannel(drivers->remote.Channel::LEFT_HORIZONTAL);
    		
    		//get curr rpm
    		CURR_RPM = RIGHT_STICK_VERT * MAX_RPM;

		//distribute to rails based on left stick horiz
		if(mode == 2){
			percent_LR = (1.0 + LEFT_STICK_HORIZ)/2.0;
			percent_RR = (1.0 - LEFT_STICK_HORIZ)/2.0; 
		}else if(mode == 1){
			percent_LR = (1.0 + RIGHT_STICK_HORIZ)/2.0;
			percent_RR = (1.0 - RIGHT_STICK_HORIZ)/2.0; 
		}else{
			percent_LR = 0;
			percent_RR = 0;
		}
		
		//handle beyblading
		if(RIGHT_STICK_VERT == 0){
			if(mode == 2){
				CURR_RPM = LEFT_STICK_HORIZ * MAX_RPM;
			}else if(mode == 1){
				CURR_RPM = RIGHT_STICK_HORIZ * MAX_RPM;
			}else{
				CURR_RPM = 0;
			}
			percent_LR = 1;
			percent_RR = -1;
		}
		
		//set assigned percentages to the rails
		LEFT_RAIL_RPM = percent_LR * CURR_RPM;
		RIGHT_RAIL_RPM = percent_RR * CURR_RPM;

		//set motors based on data
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
            			
            		//proccess into CAN
			drivers->djiMotorTxHandler.processCanSendData();
       	}	
       	//proccess into CAN
		drivers->canRxHandler.pollCanData();
	}

	modm::delay_us(10);
    }
    return 0;
}  
