#include "tap/algorithms/smooth_pid.hpp"
#include "tap/board/board.hpp"
#include "drivers_singleton.hpp"
#include <cmath>

static constexpr tap::motor::MotorId MOTOR_ID1 = tap::motor::MOTOR1;
static constexpr tap::motor::MotorId MOTOR_ID2 = tap::motor::MOTOR2;
static constexpr tap::motor::MotorId MOTOR_ID3 = tap::motor::MOTOR3;
static constexpr tap::motor::MotorId MOTOR_ID4 = tap::motor::MOTOR4;
static constexpr tap::motor::MotorId MOTOR_ID5 = tap::motor::MOTOR5;
static constexpr tap::motor::MotorId MOTOR_ID6 = tap::motor::MOTOR6;
static constexpr tap::motor::MotorId MOTOR_ID7 = tap::motor::MOTOR7;
static constexpr tap::motor::MotorId MOTOR_ID1B = tap::motor::MOTOR1;
static constexpr tap::motor::MotorId MOTOR_ID2B = tap::motor::MOTOR2;

static constexpr tap::can::CanBus CAN_BUS = tap::can::CanBus::CAN_BUS1;
static constexpr tap::can::CanBus CAN_BUS2 = tap::can::CanBus::CAN_BUS2;
static constexpr int DESIRED_RPM = 12000;
static constexpr int GIMBAL_RPM = 900;

float LEFT_STICK_VERT = 0; //left stick vertical stick ratio [-1,1]
float LEFT_STICK_HORIZ = 0; //left stick horizontal stick ratio [-1,1]
float RIGHT_STICK_HORIZ = 0; //right stick horizontal stick ratio [-1,1]
float RIGHT_STICK_VERT = 0; //right stick vertical stick ratio [-1,1]
float BB_FACTOR = 0.092; //ratio factor to affect gimble beyblading balance

int MAX_RPM = 0; //maximum rpm, changes with mode
int CURR_RPM = 0; //current rpm
	
int LEFT_RAIL_RPM = 0; //left rail specific rpm
int RIGHT_RAIL_RPM = 0; //right rail specific rpm
    
int LOAD_RPM = 0; //load motor rpm
int FEED_RPM = 0; //feeding motor rpm
int HORIZ_RPM = 0; //horizontal gimbal rpm
int VERT_RPM = 0; //vertical gimbal rpm
int TURRET_RPM = 0; //turret rpm

float percent_LR = 0; //ratio value of left rail rpm
float percent_RR = 0; //ratio value of right rail rpm
float percent_GH = 0; //ratio value of horizontal gimble rpm
float percent_GV = 0; //ratio value of vertical gimble rpm
    
int mode = 0; //double stick is mode 2, single stick is mode 1, gimble override is mode 0

tap::arch::PeriodicMilliTimer sendMotorTimeout(2);
tap::algorithms::SmoothPid pidController(20, 0, 0, 0, 8000, 1, 0, 1, 0);
tap::motor::DjiMotor motor1(::DoNotUse_getDrivers(), MOTOR_ID1, CAN_BUS, false, "cool motor 1"); //left rail motor
tap::motor::DjiMotor motor2(::DoNotUse_getDrivers(), MOTOR_ID2, CAN_BUS, false, "cool motor 2"); //left rail motor
tap::motor::DjiMotor motor3(::DoNotUse_getDrivers(), MOTOR_ID3, CAN_BUS, false, "cool motor 3"); //right rail motor
tap::motor::DjiMotor motor4(::DoNotUse_getDrivers(), MOTOR_ID4, CAN_BUS, false, "cool motor 4"); //right rail motor
tap::motor::DjiMotor loadMotor(::DoNotUse_getDrivers(), MOTOR_ID5, CAN_BUS, false, "cool motor 5"); //motor to load ammo to feeder
tap::motor::DjiMotor feedMotor(::DoNotUse_getDrivers(), MOTOR_ID6, CAN_BUS, false, "cool motor 6"); //motor to feed ammo to turret
tap::motor::DjiMotor horizGimbal(::DoNotUse_getDrivers(), MOTOR_ID7, CAN_BUS, false, "cool motor 7"); //motor to control the gimbal horizontally
tap::motor::DjiMotor vertGimbal(::DoNotUse_getDrivers(), MOTOR_ID1B, CAN_BUS2, false, "cool motor 1b"); //motor to control the gimbal vertically
tap::motor::DjiMotor turretMotor(::DoNotUse_getDrivers(), MOTOR_ID2B, CAN_BUS2, false, "cool motor 2b"); //motor to fire the turret

void ESTOP_gimbal(){
    HORIZ_RPM = 0; //horizontal gimbal rpm
    VERT_RPM = 0; //vertical gimbal rpm
}

void ESTOP_turret(){
    LOAD_RPM = 0; //load motor rpm
    FEED_RPM = 0; //feeding motor rpm
    TURRET_RPM = 0;
}

void ESTOP_rails(){
    percent_LR = 0;
    percent_RR = 0;
}

void mainControl(int mode){
    switch(mode){
        case 0: //gimbal
            ESTOP_rails(); //stop rails

            //turret firing
            if(LEFT_STICK_VERT != 0){
                LOAD_RPM = MAX_RPM * LEFT_STICK_VERT;
                HORIZ_RPM = -6000;
            }else{
                LOAD_RPM = 0;
                HORIZ_RPM = 0;
            }
            
            //turret control
            
            break;
        case 2: //double stick
            ESTOP_turret(); //stop firing mechanism

            //handle rails
            percent_LR = (1.0 + LEFT_STICK_HORIZ)/2.0;
            percent_RR = (1.0 - LEFT_STICK_HORIZ)/2.0; 

            //beyblading
            if(RIGHT_STICK_VERT == 0){
                CURR_RPM = LEFT_STICK_HORIZ * MAX_RPM;
                HORIZ_RPM = LEFT_STICK_HORIZ * MAX_RPM * BB_FACTOR;
            }
            break;
        case 1: //single stick
            ESTOP_turret(); //stop firing mechanism

            //handle rails
            percent_LR = (1.0 + RIGHT_STICK_HORIZ)/2.0;
            percent_RR = (1.0 - RIGHT_STICK_HORIZ)/2.0; 
            
            //beyblading
            if(RIGHT_STICK_VERT == 0){
                CURR_RPM = RIGHT_STICK_HORIZ * MAX_RPM;
                HORIZ_RPM = RIGHT_STICK_HORIZ * MAX_RPM * BB_FACTOR;
            }
            break;
    }
}

int main(){
    
    //initialize all components
    ::Drivers *drivers = ::DoNotUse_getDrivers();
    Board::initialize();
    drivers->remote.initialize();
    drivers->leds.init();
    drivers->can.initialize();
    motor1.initialize();
    motor2.initialize();
    motor3.initialize();
    motor4.initialize();
    loadMotor.initialize();
    feedMotor.initialize();

    //update loop
    while (1){
        //connect to remote
        drivers->remote.read();
	    if(drivers->remote.isConnected()){
	
            //set max rpm to left switch value
            if(drivers->remote.getSwitch(drivers->remote.Switch::LEFT_SWITCH) == drivers->remote.SwitchState::MID){
                MAX_RPM = (DESIRED_RPM * 8)/10;
            }else if(drivers->remote.getSwitch(drivers->remote.Switch::LEFT_SWITCH) == drivers->remote.SwitchState::UP){
                MAX_RPM = (DESIRED_RPM * 10)/10;
            }else if(drivers->remote.getSwitch(drivers->remote.Switch::LEFT_SWITCH) == drivers->remote.SwitchState::DOWN){
                MAX_RPM = (DESIRED_RPM * 6)/10;
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

            //main controls based on mode
            mainControl(mode);
            
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
                        
                //loadMotor
                pidController.runControllerDerivateError(LOAD_RPM - loadMotor.getShaftRPM(), 1);
                loadMotor.setDesiredOutput(static_cast<int32_t>(pidController.getOutput()));

                //feedMotor
                pidController.runControllerDerivateError(FEED_RPM - feedMotor.getShaftRPM(), 1);
                feedMotor.setDesiredOutput(static_cast<int32_t>(pidController.getOutput()));

                //horizMotor
                pidController.runControllerDerivateError(HORIZ_RPM - horizGimbal.getShaftRPM(), 1);
                horizGimbal.setDesiredOutput(static_cast<int32_t>(pidController.getOutput()));

                //vertMotor
                pidController.runControllerDerivateError(VERT_RPM - vertGimbal.getShaftRPM(), 1);
                vertGimbal.setDesiredOutput(static_cast<int32_t>(pidController.getOutput()));

                //turretMotor
                pidController.runControllerDerivateError(TURRET_RPM - turretMotor.getShaftRPM(), 1);
                turretMotor.setDesiredOutput(static_cast<int32_t>(pidController.getOutput()));

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
