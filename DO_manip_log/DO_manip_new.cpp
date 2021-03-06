/* 
Deformable objects manipulation_main
*/
#include <vector>
#include <string>
#include <math.h>
#include <iostream>
#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/photo.hpp>    // denoising module
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Eigenvalues>
#include "do_manip/visual_info_msg.h"
#include "do_manip/visual_info_srv.h"
#include "DO_manip_new.h"

// namespace enc = sensor_msgs::image_encodings;
const std::string WINDOW = "New OpenCV Image";

class ImageConverter
{
  ros::NodeHandle nh_;
  ros::Publisher image_info_pub = nh_.advertise<do_manip::visual_info_msg>("visual_info", 500);
  ros::ServiceServer visual_info_service = nh_.advertiseService("visual_info_service", &ImageConverter::get_visual_info_srv, this);
  image_transport::ImageTransport it_;
  image_transport::Subscriber image_sub_;
  //image_transport::Publisher image_pub_;
  float focal_x = 683;
  float focal_y = 683;
  float depth = 58;

public:
  cv::Mat gray, prevGray;
  LK_tracker lk_tracker;
  imgExtractor extractor;
  optConstructor opt;
  deformJacobian deformJd;
  angleFeature3Pts featureAngle;
  
public:
  ImageConverter(char* ros_image_stream): it_(nh_)
  {
    //image_pub_ = it_.advertise("image_processed", 1);
    image_sub_ = it_.subscribe(ros_image_stream, 30, &ImageConverter::imageCb, this);
    lk_tracker = LK_tracker(WINDOW);
    extractor = imgExtractor(WINDOW);
    cv::namedWindow(WINDOW);
  }

  ~ImageConverter()
  {
    cv::destroyWindow(WINDOW);
  }

  void imageCb(const sensor_msgs::ImageConstPtr& msg)
  {
    cv_bridge::CvImagePtr cv_ptr;
    try{
      cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e){
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }

    cv::Point imageOrigin(320, 240), xAxisEnd(220, 240), yAxisEnd(320, 340);
    cv::Mat HSVImage;
    cv::Mat cameraMatrix = (cv::Mat_<double>(3, 3) << 683.9731, 0, 320, 
                                                      0, 683.9731, 240, 
                                                      0, 0, 1);
    cv::Mat distCoef = (cv::Mat_<double>(1, 5) << -0.7175, 2.8528, 0, 0, -5.1548);
    do_manip::visual_info_msg image_info_msg;    
  
    cv::cvtColor(cv_ptr->image, HSVImage, COLOR_BGR2HSV);
    cv::GaussianBlur(HSVImage, HSVImage, cv::Size(3, 3), 5, 5);

    cv::cvtColor(cv_ptr->image, gray, COLOR_BGR2GRAY);
    if (prevGray.empty())
      gray.copyTo(prevGray);
    lk_tracker.track(cv_ptr->image, gray, prevGray);
    featureAngle = lk_tracker.angle3Pts();

    extractor.extract(cv_ptr->image, HSVImage, gray, prevGray);
    if (extractor.extractSucceed){
      cv::drawContours(cv_ptr->image, extractor.DOContours, extractor.DOLargestCotrIdx, cv::Scalar(255, 0, 0), 2);
      extractor.effectorCharacterize();
      if (extractor.effectorCharacterizeSucceed){
        //cv::circle(cv_ptr->image, extractor.endeffectorP.sl, 5, cv::Scalar(108, 37, 189), -1);
        cv::putText(cv_ptr->image, "s_l", extractor.endeffectorP.sl - cv::Point(5,5), 
                    cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(0,0,0), 2);
        // cv::circle(cv_ptr->image, extractor.endeffectorP.sr, 5, cv::Scalar(37, 189, 184), -1);
        cv::putText(cv_ptr->image, "s_r", extractor.endeffectorP.sr - cv::Point(5,5), 
                    cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(0,0,0), 2);
        cv::Point2f neDirt(extractor.endeffectorP.ne(0), extractor.endeffectorP.ne(1));
        cv::Point2f PMid = (extractor.endeffectorP.sl + extractor.endeffectorP.sr) / 2;
        cv::circle(cv_ptr->image, PMid, 5, cv::Scalar(255, 0, 0), -1);
        cv::arrowedLine(cv_ptr->image, PMid, PMid + 60*neDirt, cv::Scalar(255, 0, 0), 2);  
      }
      else if (!extractor.DOExtractSucceed)
        std::cout << "Deformable Object Detection Failed!\n";
      else if (!extractor.PExtractSucceed)
        std::cout << "End-Effector Detection Failed!\n";
      else
        std::cout << "DO and End-Effector Detection Failed!\n";
      
      if (extractor.segment(extractor.DOContour)){
        cv::circle(cv_ptr->image, extractor.DOContour[extractor.segmentationIdx[0][0]],
                  5, cv::Scalar(255, 0, 0), -1);
        cv::circle(cv_ptr->image, extractor.DOContour[extractor.segmentationIdx[0][1]],
                  5, cv::Scalar(255, 0, 0), -1); 
        cv::circle(cv_ptr->image, extractor.DOContour[extractor.segmentationIdx[1][0]],
                  5, cv::Scalar(255, 0, 0), -1);
        cv::circle(cv_ptr->image, extractor.DOContour[extractor.segmentationIdx[1][1]],
                  5, cv::Scalar(255, 0, 0), -1); 
        cv::line(cv_ptr->image, extractor.DOContour[extractor.segmentationIdx[0][0]],
                extractor.DOContour[extractor.segmentationIdx[0][1]], cv::Scalar(0,0,255), 2); 
        cv::line(cv_ptr->image, extractor.DOContour[extractor.segmentationIdx[1][0]],
                extractor.DOContour[extractor.segmentationIdx[1][1]], cv::Scalar(0,0,255), 2);                     
      }
      std::vector<Point> newP = extractor.feaibleMotionSearch();
      cv::line(cv_ptr->image, newP[0], newP[1], cv::Scalar(32,54,134), 2);

      opt = optConstructor(extractor);
      opt.getDeformConstraint();
      std::cout << "deformtion angles: " << opt.deformAngles[0] << " " << opt.deformAngles[1] << "\n";
      cv::circle(cv_ptr->image, opt.ElCentroid, 5, cv::Scalar(0,0,200), -1);
      cv::circle(cv_ptr->image, opt.ECentroid, 5, cv::Scalar(0,0,200), -1);
      cv::circle(cv_ptr->image, opt.ErCentroid, 5, cv::Scalar(0,0,200), -1);
      cv::arrowedLine(cv_ptr->image, opt.ElCentroid, opt.ElCentroid+50*opt.PrialDirtl, cv::Scalar(0,0,200),2);
      cv::arrowedLine(cv_ptr->image, opt.ErCentroid, opt.ErCentroid+50*opt.PrialDirtr, cv::Scalar(0,0,200),2);
      if (!featureAngle.sPt.empty()){
        opt.getManipulability(featureAngle);
        cout << "sw:\n" << opt.sw << "\n";
        cv::circle(cv_ptr->image, opt.sw, 8, cv::Scalar(200,0,0),-1);
      }
    }

    if (!deformJd.initialized && lk_tracker.validFeature)
      deformJd = deformJacobian(extractor.endeffectorP, featureAngle);
    else if (lk_tracker.validFeature)
      deformJd.update(extractor.endeffectorP, featureAngle);
    cout << deformJd.initialized << "\n"  << "Deformation Jacobian:\n"
         << deformJd.JdCurr << "\n";
    
    // end of processing
    cv::imshow(WINDOW, cv_ptr->image);
    cv::waitKey(3);
    image_info_pub.publish(image_info_msg);
  }

  bool get_visual_info_srv(do_manip::visual_info_srv::Request &req,
                          do_manip::visual_info_srv::Response &res)
  {
    res.sl.push_back(extractor.endeffectorP.sl.x);
    res.sl.push_back(extractor.endeffectorP.sl.y);
    res.sr.push_back(extractor.endeffectorP.sr.x);
    res.sr.push_back(extractor.endeffectorP.sr.y);
    res.ne.push_back(extractor.endeffectorP.ne[0]);
    res.ne.push_back(extractor.endeffectorP.ne[1]);

    res.contactProjectionl.push_back(extractor.DOContour[extractor.segmentationIdx[0][0]].x);
    res.contactProjectionl.push_back(extractor.DOContour[extractor.segmentationIdx[0][0]].y);
    res.contactProjectionr.push_back(extractor.DOContour[extractor.segmentationIdx[1][0]].x);
    res.contactProjectionr.push_back(extractor.DOContour[extractor.segmentationIdx[1][0]].y);
    res.contactDistancelr.push_back(cv::norm(extractor.endeffectorP.sl 
                                    - extractor.DOContour[extractor.segmentationIdx[0][0]]));
    res.contactDistancelr.push_back(cv::norm(extractor.endeffectorP.sr 
                                    - extractor.DOContour[extractor.segmentationIdx[1][0]]));
    
    res.deformAngles.push_back(opt.deformAngles[0]);
    res.deformAngles.push_back(opt.deformAngles[1]);

    res.sw.push_back(opt.sw.x);
    res.sw.push_back(opt.sw.y);
    res.indicatorw = opt.indicatorw;
    res.distancew = opt.distancew;

    res.featureAngley = featureAngle.angle;
    res.deformJacobian.push_back(deformJd.JdCurr(0, 0));
    res.deformJacobian.push_back(deformJd.JdCurr(0, 1));

    // ROS_INFO("Successfully obtain visual information.\n");
    return true;
  }  
};

int main(int argc, char** argv)
{
  if (argc == 2){
    ros::init(argc, argv, "image_processing");
    ImageConverter ic(argv[1]);
    ros::spin();
    return 0;
  } 
  else if (argc == 1){
  // set default ros image source
    char *image_topic = (char*)"/cameras/source_camera/image";
    ros::init(argc, argv, "image_processing");
    ImageConverter ic(image_topic);
    ros::spin();
    return 1;
  }
  else{
    std::cout << "ERROR:\tusage - RosToOpencvImage <ros_image_topic>" << std::endl;    
    return 2; 
  }
}