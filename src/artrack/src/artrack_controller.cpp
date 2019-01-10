#include <ros/ros.h>
#include <geometry_msgs/Point.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/String.h>
#include <std_msgs/Int16.h>
#include <std_msgs/Empty.h>

class Controller
{
  //ROS Elements
  ros::NodeHandle n;
  ros::Publisher  velocity_pub;

  ros::Subscriber imgcoord_sub;
  ros::Subscriber autonomous_sub;
  ros::Subscriber resolution_sub;
  ros::Subscriber takeOff_sub;
  ros::Subscriber landing_sub;
  ros::Subscriber faceSize_sub;

  //Message declarations
  geometry_msgs::Twist drone;
  geometry_msgs::Point resolution;
  geometry_msgs::Point pCoord;
  geometry_msgs::Point faceSize;

  //Variables
  int flightMode = 0;
  int droneStatus = 0;
  double velPos = 0.1;
  double velNeg = -0.1;

  //PD Controller Variables
  double errorX = 0, errorY = 0;
  double errorXMax = 0, errorYMax = 0;
  double Xmax = 0, Ymax = 0;
  double Vz = 0, Vy = 0, Vx = 0;
  double Kpz = 0, Kpy = 0, Kdz = 0.0002, Kdy = 0.0002;
  double prevXError = 0, prevYError = 0, derivativeX = 0, derivativeY = 0;
  
  double faceArea = 1000000, maxFaceArea = 30000;
  double Kpx = 0, errorArea = 0;
  
public:
  Controller()
  {
    //Subscribers
    autonomous_sub =    n.subscribe("/artrack_controller/autonomous", 10, &Controller::autonomousDetect, this);
    imgcoord_sub   =	  n.subscribe("/artrack_controller/imgcoord", 10, &Controller::droneControl, this);
    resolution_sub =	  n.subscribe("/artrack_controller/resolution", 10, &Controller::resCallback, this);
    takeOff_sub    = 	  n.subscribe("/artrack_controller/takeOff", 1, &Controller::takeOff, this);
    landing_sub    = 	  n.subscribe("/artrack_controller/landing", 1, &Controller::landing, this);
    faceSize_sub   =    n.subscribe("/artrack_controller/faceSize", 1, &Controller::faceSizeRec, this);

    //Publishers
    velocity_pub   =	  n.advertise<geometry_msgs::Twist>("/cmd_vel", 10);
  }

  ~Controller()
  {
    
  }

  void droneControl(geometry_msgs::Point msg) {
  	
  	pCoord.x = msg.x;
  	pCoord.y = msg.y;

    Xmax = double(resolution.x);
    Ymax = double(resolution.y);

    errorXMax = Xmax / 2;
    errorYMax = Ymax / 2;

    Kpy = -0.2 / Xmax;
    Kpz = -0.35 / Ymax;
    Kpx = 0.1 / maxFaceArea;

  	if (droneStatus) {
	  	
      if (flightMode == 1) {
		  	
        errorX = double(pCoord.x) - errorXMax;
        errorY = double(pCoord.y) - errorYMax;
        errorArea = maxFaceArea - faceArea;

        derivativeX = errorX - prevXError;
        derivativeY = errorY - prevYError;

        Vz = Kpz * errorY + Kdz * derivativeY;
        Vy = Kpy * errorX + Kdy * derivativeX;
        Vx = Kpx * errorArea;

        prevXError = errorX;
        prevYError = errorY;

        drone.linear.z = Vz;
        drone.linear.y = Vy;
        drone.linear.x = Vx;

        velocity_pub.publish(drone);
        
	  	}
  	}
  }

  void resCallback(geometry_msgs::Point msg) {
  		resolution.x = msg.x;
	  	resolution.y = msg.y;
   }

  void autonomousDetect(std_msgs::Int16 msg) {
		flightMode = msg.data;
   }

  void takeOff(std_msgs::Empty msg) {
    droneStatus = 1;
  }

  void landing(std_msgs::Empty msg) {
    droneStatus = 0;
   }

  void faceSizeRec(geometry_msgs::Point msg) {
    faceSize.x = msg.x;
    faceSize.y = msg.y;
    faceArea = faceSize.x * faceSize.y;
  }
};


int main(int argc, char **argv)
{
  
  ros::init(argc, argv, "droneController");
  Controller droneController;
  ros::Rate loop_rate(10);
  ros::spin();
  return 0;
}