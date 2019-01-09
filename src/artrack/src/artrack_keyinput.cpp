#include <ros/ros.h>
#include <std_msgs/String.h>
#include <std_msgs/Int16.h>
#include <std_msgs/Empty.h>
#include <geometry_msgs/Twist.h>
#include <keyboard_reader/Key.h>
#include <iostream>
#include <sstream>

class KeyboardInput
{
  //ROS Node Elements
  ros::NodeHandle 	n;
  ros::Publisher 	directions_pub;
  ros::Publisher 	commands_pub;
  ros::Publisher 	takeoff_pub;
  ros::Publisher	land_pub;
  ros::Publisher 	autonomous_pub;
  ros::Subscriber 	keyReceiver_sub;

  //Message Declaration
  std_msgs::String  output;		//String to identify direction of drone
  std_msgs::Empty   mTakeoff;	//Empty takeoff message
  std_msgs::Empty   mLand;		//Empty landing message
  std_msgs::Int16   autonomous;	//Autonomous indicator
  
  //Message declaration
  geometry_msgs::Twist drone;	//Velocity values for drone

  //Velocity value variables
  double velPos = 0.5;
  double velNeg = -0.5;

  //Array that keeps track of the status of the keys
  uint8_t keyStatus[11] = {0,0,0,0,0,0,0,0,0,0,0};

public:
  KeyboardInput()
  {
  	//Subcribers
    keyReceiver_sub  = 	n.subscribe("/artrack_keyinput/keyReceiver", 10, &KeyboardInput::keyPressUpdate, this);

    //Publishers
    directions_pub = 	n.advertise<std_msgs::String>("/artrack_keyinput/directions", 1000);
    commands_pub   = 	n.advertise<geometry_msgs::Twist>("/cmd_vel", 100);
    takeoff_pub    = 	n.advertise<std_msgs::Empty>("/artrack_keyinput/takeoff", 10);
    land_pub       = 	n.advertise<std_msgs::Empty>("/artrack_keyinput/land", 10);
    autonomous_pub =    n.advertise<std_msgs::Int16>("/artrack_keyinput/flightModePub", 10);

  }

  ~KeyboardInput()
  {
    
  }

  void keyPressUpdate(keyboard_reader::Key msg) {
	
	//Cases for every key pressed (that is of interest)
	
	switch (msg.key_code) {
			case 103: //up
				if (msg.key_pressed && !keyStatus[0]) {
					keyStatus[0] = 1;
					output.data = "up";
					drone.linear.z = velPos;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				} else {
					keyStatus[0] = 0;
					drone.linear.z = 0;
					commands_pub.publish(drone);
				}
			break;
			case 108: //down
				if (msg.key_pressed && !keyStatus[1]) {
					keyStatus[1] = 1;
					output.data = "down";
					drone.linear.z = velNeg;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				} else {
					keyStatus[1] = 0;
					drone.linear.z = 0;
					commands_pub.publish(drone);
				}
			break;
			case 106: //right
				if (msg.key_pressed && !keyStatus[2]) {
					keyStatus[2] = 1;
					output.data = "right";
					drone.angular.z = velNeg;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				} else {
					keyStatus[2] = 0;
					drone.angular.z = 0;
					commands_pub.publish(drone);
				}
			break;
			case 105: //left
				if (msg.key_pressed && !keyStatus[3]) {
					keyStatus[3] = 1;
					output.data = "left";
					drone.angular.z = velPos;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				} else {
					keyStatus[3] = 0;
					drone.angular.z = 0;
					commands_pub.publish(drone);
				}
			break;
			case 17: //w
				if (msg.key_pressed && !keyStatus[4]) {
					keyStatus[4] = 1;
					output.data = "w";
					drone.linear.x = velPos;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				} else {
					keyStatus[4] = 0;
					drone.linear.x = 0;
					commands_pub.publish(drone);
				}
			break;
			case 30: //a
				if (msg.key_pressed && !keyStatus[5]) {
					keyStatus[5] = 1;
					output.data = "a";
					drone.linear.y = velPos;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				} else {
					keyStatus[5] = 0;
					drone.linear.y = 0;
					commands_pub.publish(drone);
				}
			break;
			case 31: //s
				if (msg.key_pressed && !keyStatus[6]) {
					keyStatus[6] = 1;
					output.data = "s";
					drone.linear.x = velNeg;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				} else {
					keyStatus[6] = 0;
					drone.linear.x = 0;
					commands_pub.publish(drone);
				}
			break;
			case 32: //d
				if (msg.key_pressed && !keyStatus[7]) {
					keyStatus[7] = 1;
					output.data = "d";
					drone.linear.y = velNeg;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				} else {
					keyStatus[7] = 0;
					drone.linear.y = 0;
					commands_pub.publish(drone);
				}
			break;
			case 16: //q
				if (msg.key_pressed && !keyStatus[8]) {
					keyStatus[8] = 1;
					output.data = "q";
					takeoff_pub.publish(mTakeoff);
					directions_pub.publish(output);
				} else {
					keyStatus[8] = 0;	
				}

			break;
			case 18: //e
				if (msg.key_pressed && !keyStatus[9]) {
					keyStatus[9] = 1;
					output.data = "e";
					land_pub.publish(mLand);
					directions_pub.publish(output);
				} else {
					keyStatus[9] = 0;
				}
			break;
			case 25: //p
				if (msg.key_pressed) {
					output.data = "p";
					if (autonomous.data == 1) {
						autonomous.data = 0;
					} else {
						autonomous.data = 1;
					}	
					commands_pub.publish(drone);
					autonomous_pub.publish(autonomous);
				} 
			break;
	}
	
  }

};



int main(int argc, char **argv)
{
  
  ros::init(argc, argv, "keyboardMonitor");
  KeyboardInput cont;
  ros::Rate loop_rate(10);
  ros::spin();
  return 0;
}