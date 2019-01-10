#include <ros/ros.h>
#include <std_msgs/String.h>
#include <std_msgs/Int16.h>
#include <std_msgs/Empty.h>
#include <geometry_msgs/Twist.h>
#include <keyboard/Key.h>
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
  ros::Subscriber 	keydown_in_sub;
  ros::Subscriber 	keyup_in_sub;

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


public:
  KeyboardInput()
  {
  	//Subcribers
    keydown_in_sub = 	n.subscribe("/artrack_keyinput/idownkeyboard", 10, &KeyboardInput::processInputDown, this);
    keyup_in_sub   = 	n.subscribe("/artrack_keyinput/iupkeyboard", 10, &KeyboardInput::processInputUp, this);

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
	
  void processInputDown(keyboard::Key msg) {
		
		switch (msg.code) {
				case 273: //up
					output.data = "up";
					drone.linear.z = velPos;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				break;
				case 274: //down
					output.data = "down";
					drone.linear.z = velNeg;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				break;
				case 275: //right
					output.data = "right";
					drone.angular.z = velNeg;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				break;
				case 276: //left
					output.data = "left";
					drone.angular.z = velPos;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				break;
				case 119: //w
					output.data = "w";
					drone.linear.x = velPos;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				break;
				case 97: //a
					output.data = "a";
					drone.linear.y = velPos;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				break;
				case 115: //s
					output.data = "s";
					drone.linear.x = velNeg;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				break;
				case 100: //d
					output.data = "d";
					drone.linear.y = velNeg;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				break;
				case 113: //q
					output.data = "q";
					takeoff_pub.publish(mTakeoff);
					directions_pub.publish(output);
				break;
				case 101: //e
					output.data = "e";
					land_pub.publish(mLand);
					directions_pub.publish(output);
				break;
				case 112: //p
					output.data = "p";
					if (autonomous.data == 1) {
						autonomous.data = 0;
					} else {
						autonomous.data = 1;
					}	
					commands_pub.publish(drone);
					autonomous_pub.publish(autonomous);
				break;
		}
		
	  }

	  void processInputUp(keyboard::Key msg) {
		switch (msg.code) {
				case 273: //up
					output.data = "up";
					drone.linear.z = 0;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				break;
				case 274: //down
					output.data = "down";
					drone.linear.z = 0;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				break;
				case 275: //right
					output.data = "right";
					drone.angular.z = 0;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				break;
				case 276: //left
					output.data = "left";
					drone.angular.z = 0;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				break;
				case 119: //w
					output.data = "w";
					drone.linear.x = 0;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				break;
				case 97: //a
					output.data = "a";
					drone.linear.y = 0;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				break;
				case 115: //s
					output.data = "s";
					drone.linear.x = 0;
					commands_pub.publish(drone);
					directions_pub.publish(output);
				break;
				case 100: //d
					output.data = "d";
					drone.linear.y = 0;
					commands_pub.publish(drone);
					directions_pub.publish(output);
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