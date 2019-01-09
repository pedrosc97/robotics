//////////////////////////////////////////////////////////////////////////////////////////////////////
//  
//	ImageProcessor 
//  
//  Node source code for the processing of the image. It uses neural networks to detect faces,
//  then tracks the detected face using a tracking algorithm. It publishes the coordinates of
//  the centroid of the face. This node also handles the GUI elements of the main window of 
// 	the project.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////

#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <sensor_msgs/image_encodings.h>
#include <geometry_msgs/Point.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/Int16.h>
#include <std_msgs/Empty.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/utils/trace.hpp>
#include <iostream>

using namespace std;

static const std::string OPENCV_WINDOW = "OpenCV Image Window";

//Image Processor Class

class ImageProcessor
{

  //Object declarations for the ROS elements
  ros::NodeHandle 	nh_;
  ros::Publisher 	centerPub;
  ros::Publisher 	imgResPub;
  ros::Publisher 	faceSizePub;
  ros::Subscriber 	velocityDataSub;
  ros::Subscriber 	flightModeSub;
  ros::Subscriber 	takeOffSub;
  ros::Subscriber 	landingSub;

  //Image Transport Elements (For receiving images either from drone or Webcam)
  image_transport::ImageTransport it_;
  image_transport::Subscriber image_sub_;
  image_transport::Publisher image_pub_;

  //Object initialization for neural network
  cv::dnn::Net net;

  //ROS Messages
  geometry_msgs::Point faceCentroid;
  geometry_msgs::Point imgResolution;
  geometry_msgs::Point faceSize;

  //OpenCV Variables
  cv::Scalar flightColor;

  //Variables
  uint16_t columns, rows;				//Variables for the original size of the image
  uint16_t resizeValue = 300;			//Resize value for the image entering the NN
  uint16_t startX, startY, endX, endY;	//Start and end coordinates of the bounding box for face
  uint16_t centerX, centerY;			//Coordinates of the center
  uint16_t objID = 0;					//Face ID
  uint16_t resX = 1080, resY = 800;		//Resolution values for image upscale
  uint16_t flightMode = 0;				//Flight mode indicator
  uint16_t droneStatus = 0;				//Status indicator
  float lX = 0, lY = 0, lZ = 0, aZ = 0; //Linear and angular velocities
  float confidence;						//Confidence values for the Neural network output
  char confDisp[100];					//Confidence string for GUI
  char ID[100];							//ID display for GUI
  char info1[100], info2[100], 			//Information variables for GUI
  	   info3[100], info4[100],
  	   info5[100];


public:
  ImageProcessor()
    : it_(nh_)
  {
 	
 	//Assign publisher and subscriber to receive image and output it

    image_sub_        = it_.subscribe("imageinput", 1, &ImageProcessor::imagePr, this);
    image_pub_        = it_.advertise("/artrack_imgproc/output_video", 1);

    //Publisher and Subscriber declarations

    centerPub         = nh_.advertise<geometry_msgs::Point>("/artrack_imgproc/faceCentroid", 1);
    imgResPub 		  = nh_.advertise<geometry_msgs::Point>("/artrack_imgproc/imgResolution", 1);
    faceSizePub 	  = nh_.advertise<geometry_msgs::Point>("/artrack_imgproc/faceSize", 1);

    velocityDataSub	  = nh_.subscribe("/artrack_imgproc/cmd_vel", 1, &ImageProcessor::velocityMonitor, this);
    flightModeSub 	  = nh_.subscribe("/artrack_imgproc/flightModeSub", 1, &ImageProcessor::AutonomousDisp, this);
    takeOffSub        = nh_.subscribe("/artrack_imgproc/takeOffSub", 1, &ImageProcessor::takeOff, this);
    landingSub 		  = nh_.subscribe("/artrack_imgproc/landingSub", 1, &ImageProcessor::landing, this);
    
    //Initializing an OpenCV Window

    cv::namedWindow(OPENCV_WINDOW);

    //Initializing strings containing paths for the prototxt and caffemodel files

    cv::String modelTxt = "/home/pedrosc/robotics/src/artrack/src/deploy.prototxt.txt";
    cv::String modelBin = "/home/pedrosc/robotics/src/artrack/src/res10_300x300_ssd_iter_140000.caffemodel";

    //Initializing and configuring the neural network

    try {
        net = cv::dnn::readNetFromCaffe(modelTxt, modelBin);
    }
    catch (cv::Exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        if (net.empty())
        {
            std::cerr << "Can't load network by using the following files: " << std::endl;
            std::cerr << "prototxt:   " << modelTxt << std::endl;
            std::cerr << "caffemodel: " << modelBin << std::endl;
            std::cerr << "bvlc_googlenet.caffemodel can be downloaded here:" << std::endl;
            std::cerr << "http://dl.caffe.berkeleyvision.org/bvlc_googlenet.caffemodel" << std::endl;
            exit(-1);
        }
    }

  }

  ~ImageProcessor()
  {
  	//Class destructor

    cv::destroyWindow(OPENCV_WINDOW);
  }

  void imagePr(const sensor_msgs::ImageConstPtr& msg)
  {
    
    //Convert ROS Message Image into OpenCV Image

    cv_bridge::CvImagePtr cv_ptr;
    try
    {
      cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }
	
	//Assigning converted image into a Mat variable 

	cv::Mat original = cv_ptr->image;
	cv::Mat resizedOriginal;
	resize(original, resizedOriginal, cv::Size(resizeValue,resizeValue));
	resize(original, original, cv::Size(resX, resY));

	//Extracting original dimensions

	columns = original.cols;
	rows = original.rows;

	//Creating a blob and resizing the input

	cv::Mat inputBlob = cv::dnn::blobFromImage(resizedOriginal, 1.0f, cv::Size(resizeValue, resizeValue), cv::Scalar(104, 177, 123), false);

	//Input and run the blob through the neural network

	net.setInput(inputBlob);
	cv::Mat detections = net.forward();

	//Elimination of the first two dimensions of the output matrix

	cv::Mat detectionMat(detections.size[2], detections.size[3], CV_32F, detections.ptr<float>());

	//Extraction of the confidence values; for ease of use purposes, only the first 5 elements of the matrix will be checked
	//since the demo of the project will only feature one person guiding the drone
	
	confidence = detectionMat.at<float>(0,2) * 100;

	if (confidence >= 55.0) {

		startX = uint16_t(detectionMat.at<float>(0,3) * resX);
		startY = uint16_t(detectionMat.at<float>(0,4) * resY);
		endX = uint16_t(detectionMat.at<float>(0,5) * resX);
		endY = uint16_t(detectionMat.at<float>(0,6) * resY);
			
		centerX = (endX - startX)/2 + startX;
		centerY = (endY - startY)/2 + startY;

		sprintf(confDisp, "%d", uint16_t(confidence));
			
		cv::rectangle(original, cv::Point(startX, startY), cv::Point(endX, endY), cv::Scalar(0,255,0), 2);
		cv::circle(original, cv::Point(centerX, centerY), 0.1, cv::Scalar(255,255,255), 2);
		cv::putText(original, confDisp, cv::Point(centerX, centerY - 10), 2, 0.45, cv::Scalar(255, 255, 255), 1);

		faceSize.x = endX - startX;
		faceSize.y = endY - startY;
			
	} else {

		centerX = resX / 2;
		centerY = resY / 2;

		faceSize.x = 150;
		faceSize.y = 200;

	}

	faceCentroid.x = centerX;
	faceCentroid.y = centerY;

	//Print commands for debugging purposes
	//cout << "Confidence " << confidence << endl;
	//cout << "M = "<< endl << " "  << detectionMat << endl << endl;
	//cout << "Start X: " << startX << " End X: " << endX << endl;
	//cout << "Start Y: " << startY << " End Y: " << endY << endl;

	//Preparing output data on resolution 

	imgResolution.x = resX;
	imgResolution.y = resY;

	//Publishing data on topics

	imgResPub.publish(imgResolution);
	centerPub.publish(faceCentroid);
	faceSizePub.publish(faceSize);

	//GUI Elements////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//Title Message
	cv::putText(original, "ARDrone Tracker V2.1", cv::Point(3,18), 2, 0.6, cv::Scalar(255,255,255), 1, 16);

	//Coordinates of the in-focus object
	sprintf(info1, "X = %u, Y = %u", centerX, centerY);
    cv::putText(original, info1, cv::Point(3,original.rows-5), 2, 0.8, cv::Scalar(255,255,255), 1, 16);

    //Flight mode indicator (Autonomous or Manual)
    if (flightMode) {
      sprintf(info2, "Autonomous Flight");
      putText(original, info2, cv::Point(3,original.rows-30), 2, 0.8, cv::Scalar(0,0,255), 1, 16);
    } else {
      sprintf(info2, "Manual Flight");
      putText(original, info2, cv::Point(3,original.rows-30), 2, 0.8, cv::Scalar(0,255,0), 1, 16);
    }
    
    //Drone Status Indicator
    switch (droneStatus) {
      case 1:
        sprintf(info3, "Flying");
        putText(original, info3, cv::Point(3,original.rows-55), 2, 0.8, cv::Scalar(0,0,255), 1, 16);
      break;
      default:  
        sprintf(info3, "Landed");
        putText(original, info3, cv::Point(3,original.rows-55), 2, 0.8, cv::Scalar(0,255,0), 1, 16);
      break;
    }

    //Drone Odometry Data     
    sprintf(info4, "X = %f  Y = %f", lX, lY);
    putText(original, info4, cv::Point(original.cols - 400, original.rows - 5), 2, 0.8, cv::Scalar(255,255,255), 1, 16);
    sprintf(info5, "Z = %f  R = %f", lZ, aZ);
    putText(original, info5, cv::Point(original.cols - 400, original.rows - 30), 2, 0.8, cv::Scalar(255,255,255), 1, 16);
    putText(original, "Odometry Data", cv::Point(original.cols - 200, original.rows - 60), 2, 0.8, cv::Scalar(255,255,255), 1, 16);

    //Reticle for center
    cv::rectangle(original, cv::Point(original.cols/2 + 15, original.rows/2 + 15), cv::Point(original.cols/2 - 15, original.rows/2 - 15), cv::Scalar( 0, 255, 0 ), 1, 8);
    cv::line( original, cv::Point(original.cols/2, original.rows/2 + 5), cv::Point(original.cols/2,  original.rows/2 - 5), cv::Scalar( 0, 255, 0 ), 1, 8);
    cv::line( original, cv::Point(original.cols/2 + 5, original.rows/2), cv::Point(original.cols/2 - 5, original.rows/2), cv::Scalar( 0, 255, 0 ), 1, 8);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//Display results

	cv::imshow(OPENCV_WINDOW, original);
	cv::waitKey(1);
  }

  void velocityMonitor(geometry_msgs::Twist msg) {
    //Velocity data receiver
    lX = msg.linear.x;
    lY = msg.linear.y;
    lZ = msg.linear.z;
    aZ = msg.angular.z;
  }

  void AutonomousDisp(std_msgs::Int16 msg) {
    //Flightmode receiver
    if (msg.data) {
      flightMode = 1;
    } else {
      flightMode = 0;
    }
  }

  void takeOff(std_msgs::Empty msg) {
    droneStatus = 1;
  }

  void landing(std_msgs::Empty msg) {
    droneStatus = 0;
  }

    
};

//Main

int main(int argc, char** argv)
{
  
  //Initialize ROS Node called imageProcessor
  ros::init(argc, argv, "imageProcessor");

  //Initialize a new object called ip, class ImageProcessor
  ImageProcessor ip;

  //"Loop" the ROS cycle
  ros::spin();

  return 0;
}