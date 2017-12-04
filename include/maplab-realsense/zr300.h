#ifndef MAPLAB_REALSENSE_ZR300_H_
#define MAPLAB_REALSENSE_ZR300_H_

#include <string>

#include <cuckoo_time_translator/DeviceTimeTranslator.h>
#include <image_transport/publisher.h>
#include <librealsense/rs.hpp>
#include <librealsense/rsutil.h>
#include <opencv2/highgui/highgui.hpp>
#include <ros/ros.h>

#include "maplab-realsense/time-synchronizer.h"

namespace maplab_realsense {

struct RealSenseConfiguration {
  // Imu config.
  bool imu_enabled = true;

  // Fisheye config.
  bool fisheye_enabled = true;
  bool fisheye_enable_auto_exposure = true;
  // 40ms is minimum
  double fisheye_exposure_ms = 25.;
  double fisheye_gain = 9.;
  int fisheye_subsample_factor = 1.;
  int fisheye_width = 640;
  int fisheye_height = 480;
  int fisheye_fps = 30;

  // Color config.
  bool color_enabled = true;
  int color_width = 640;
  int color_height = 480;
  int color_fps = 30;
  int color_subsample_factor = 1;

  // Depth config.
  bool depth_enabled = true;
  int depth_width = 640;
  int depth_height = 480;
  int depth_fps = 30;
  int depth_subsample_factor = 1;
  bool depth_median_filter_enabled = false;
  bool depth_min_max_filter_enabled = false;
  int depth_min_max_filter_size = 3;
  float depth_min_max_filter_threshold = 0.3f;

  // IR config.
  bool infrared_enabled = true;
  int infrared_subsample_factor = 1;

  // Pointcloud config.
  bool pointcloud_enabled = true;
  bool pointcloud_color_filter_enabled = false;
  int pointcloud_hsv_min_h = 0;
  int pointcloud_hsv_min_s = 0;
  int pointcloud_hsv_min_v = 0;
  int pointcloud_hsv_max_h = 255;
  int pointcloud_hsv_max_s = 255;
  int pointcloud_hsv_max_v = 255;

  // Depth control config.
  static constexpr rs::option kDepthControlOptions[10] = {
      rs::option::r200_depth_control_estimate_median_decrement,
      rs::option::r200_depth_control_estimate_median_increment,
      rs::option::r200_depth_control_median_threshold,
      rs::option::r200_depth_control_score_minimum_threshold,
      rs::option::r200_depth_control_score_maximum_threshold,
      rs::option::r200_depth_control_texture_count_threshold,
      rs::option::r200_depth_control_texture_difference_threshold,
      rs::option::r200_depth_control_second_peak_threshold,
      rs::option::r200_depth_control_neighbor_threshold,
      rs::option::r200_depth_control_lr_threshold};

  // DEFAULT:
  // Default settings on chip. Similiar to the medium
  // setting and best for outdoors.
  static constexpr double kDepthControlDefault[10] = {5, 5,  192, 1, 512,
                                                      6, 24, 27,  7, 24};
  // OFF:
  // Disable almost all hardware-based outlier removal
  static constexpr double kDepthControlOff[10] = {5, 5, 0, 0, 1023,
                                                  0, 0, 0, 0, 2047};

  // LOW:
  // Provide a depthmap with a lower number of outliers
  // removed, which has minimal false negatives.
  static constexpr double kDepthControlLow[10] = {5, 5,  115, 1, 512,
                                                  6, 18, 25,  3, 24};
  // MEDIUM:
  // Provide a depthmap with a medium number of outliers
  // removed, which has balanced approach.
  static constexpr double kDepthControlMedium[10] = {5, 5,  185, 5,  505,
                                                     6, 35, 45,  45, 14};

  // OPTIMIZED:
  // Provide a depthmap with a medium/high number of
  // outliers removed. Derived from an optimization function.
  static constexpr double kDepthControlOptimized[10] = {5, 5,  175, 24, 430,
                                                        6, 48, 47,  24, 12};
  // HIGH:
  // Provide a depthmap with a higher number of outliers
  // removed, which has minimal false positives.
  static constexpr double kDepthControlHigh[10] = {5, 5,  235, 27, 420,
                                                   8, 80, 70,  90, 12};

  // Default: OPTIMIZED
  const double* depth_control_values = kDepthControlHigh;

  static RealSenseConfiguration getFromRosParams(
      const ros::NodeHandle& private_nh);
};

class ZR300 {
 public:
  ZR300(
      ros::NodeHandle nh, ros::NodeHandle private_nh,
      const std::string& frameId = "");

  bool start();
  void stop();

 private:
  void motionCallback(const rs::motion_data& entry);
  void frameCallback(const rs::frame& frame);

  void initializePublishers(ros::NodeHandle* nh);
  void enableSensorStreams();
  void configureStaticOptions();
  void registerCallbacks();

  ros::NodeHandle nh_;
  ros::NodeHandle private_nh_;

  RealSenseConfiguration config_;

  image_transport::Publisher fisheye_publisher_;
  image_transport::Publisher color_publisher_;
  image_transport::Publisher infrared_publisher_;
  image_transport::Publisher infrared_2_publisher_;
  image_transport::Publisher depth_publisher_;
  ros::Publisher imu_publisher_;
  ros::Publisher pointcloud_publisher_;

  rs::context zr300_context_;
  rs::device* zr300_device_;

  static const std::string kFisheyeTopic;
  static const std::string kColorTopic;
  static const std::string kImuTopic;
  static const std::string kInfraredTopic;
  static const std::string kInfrared2Topic;
  static const std::string kDepthTopic;
  static const std::string kPointCloudTopic;

  static constexpr double kImuTimestampToSeconds = 1e-3;

  static constexpr int kSecondsToNanoseconds = 1e9;
  static constexpr int kMillisecondsToNanoseconds = 1e6;

  ImuSynchronizer imu_synchronizer_;

  double last_color_frame_timestamp_s_ = -1.0;
  double last_fisheye_frame_timestamp_s_ = -1.0;
  double last_infrared_frame_timestamp_s_ = -1.0;
  double last_infrared_2_frame_timestamp_s_ = -1.0;
  double last_depth_frame_timestamp_s_ = -1.0;

  size_t latest_depth_frame_number_ = 0u;
  cv::Mat latest_depth_map_;

  std::unique_ptr<cuckoo_time_translator::UnwrappedDeviceTimeTranslator>
      device_time_translator_;

  size_t gyro_measurement_index_;
  static constexpr size_t kSkipNFirstGyroMeasurements = 100u;
};

}  // namespace maplab_realsense

#endif  // MAPLAB_REALSENSE_ZR300_H_
