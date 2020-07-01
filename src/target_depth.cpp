#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
 
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Point.h>
#include <std_msgs/String.h>
#include <person_navigation/Polar.h>
 
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>  
#include <cv_bridge/cv_bridge.h>
#include <opencv2/core/utility.hpp>
 
#define PI 3.14159265
 
ros::Publisher pub_depth_angle;
 
 
float x, y;
sensor_msgs::Image depth_image;
 
void bbox_callback(const geometry_msgs::PointConstPtr& msgcenter)
{
    x = msgcenter->x;
    y = msgcenter->y;
 
    cv::Mat DepthImageCopy_;
    cv_bridge::CvImageConstPtr cam_depth;
    try {
        cam_depth = cv_bridge::toCvCopy(depth_image, sensor_msgs::image_encodings::TYPE_32FC1);
    }
    catch (const std::exception& e){
        std::cout<<e.what()<<std::endl;
        std::cout<<"Maybe check if your subscribed topic is empty?"<<std::endl;
        return;
    }

    DepthImageCopy_ = cam_depth->image.clone();
    float depth_val = (float)DepthImageCopy_.at<float>( y, x );
    float Z = depth_val;
 
    float X = ((x-322.4541015625)*Z)/616.3174438476562;           //X=((U-Cx)*Z)/fx
    float Y = ((y-243.06459045410156)*Z)/616.468017578125;           //Y=((V-Cy)*Z)/fy
   
    float angle = tan((-X / 1000.0) / (Z / 1000.0)) * (180/PI);
    std::cout<<"    angle: "<<angle<<std::endl;

    depth_val = depth_val/1000.0;    
    std::cout << "Depth value: " << depth_val << std::endl;

    person_navigation::Polar msg;
    msg.angle = angle;
    msg.depth = depth_val;

    pub_depth_angle.publish(msg);
 
    //msg_goal.position = msg_position.pose.position;
    //msg_goal.orientation.x = angle;
    //pub_goal.publish(msg_goal);
}
 
void depth_callback(const sensor_msgs::Image& msgdepth)
{
    depth_image = msgdepth;
}
 
int main (int argc, char **argv)
{
    ros::init(argc, argv, "target_depth");
    ros::NodeHandle nh;
    ros::Subscriber sub_depthImage = nh.subscribe("/camera/aligned_depth_to_color/image_raw", 1000, depth_callback);
    ros::Subscriber sub_boundingBox = nh.subscribe("/person_tracking/bbox_center", 1000, bbox_callback);
 
    pub_depth_angle = nh.advertise<person_navigation::Polar>("/person_navigation/depth_angle", 10);
 
    ros::spin();
}
