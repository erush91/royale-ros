// -*- c++ -*-
/*
 * Copyright (C) 2017 Love Park Robotics, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distribted on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __ROYALE_ROS_CAMERA_NODELET_H__
#define __ROYALE_ROS_CAMERA_NODELET_H__

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <image_transport/image_transport.h>
#include <nodelet/nodelet.h>
#include <ros/ros.h>
#include <royale_ros/Config.h>
#include <royale_ros/Dump.h>
#include <royale_ros/Start.h>
#include <royale_ros/Stop.h>
#include <royale_ros/SetExposureTime.h>
#include <royale_ros/SetExposureTimes.h>
#include <royale.hpp>

namespace royale_ros
{
  /**
   * This class implements both the ROS nodelet interface and
   * the Royale IDepthDataListener interface. It is the central figure of this
   * application used to manage, configure, and acquire data from a single
   * Royale camera.
   */
  class CameraNodelet
    : public nodelet::Nodelet, public royale::IDepthDataListener
  {
  private:
    //
    // Nodelet lifecycle functions
    //
    virtual void onInit() override;

    //
    // ROS services
    //
    bool Dump(royale_ros::Dump::Request& req, royale_ros::Dump::Response& resp);
    bool Config(royale_ros::Config::Request& req,
                royale_ros::Config::Response& resp);
    bool Start(royale_ros::Start::Request& req,
               royale_ros::Start::Response& resp);
    bool Stop(royale_ros::Stop::Request& req, royale_ros::Stop::Response& resp);

    //
    // Handlers for topic subscriptions
    //
    void SetExposureTimeCb(const royale_ros::SetExposureTime::ConstPtr& msg);
    void SetExposureTimesCb(const royale_ros::SetExposureTimes::ConstPtr& msg);

    //
    // Royale callback, basically functions as our main
    // publishing loop
    //
    void onNewData(const royale::DepthData *data) override;

    //
    // Helpers
    //
    void InitCamera();
    void RescheduleTimer();
    void CacheIntrinsics();

    //
    // State
    //

    // Acts as a s/w switch for the camera stream
    bool on_;
    std::mutex on_mutex_;

    std::unique_ptr<royale::ICameraDevice> cam_;
    std::mutex cam_mutex_;
    std::string access_code_;
    std::string serial_number_;
    float poll_bus_secs_;
    float timeout_secs_;
    std::string optical_frame_;
    std::string sensor_frame_;
    std::string initial_use_case_;

    bool instantiated_publishers_;
    std::uint32_t access_level_;
    ros::Time last_frame_;
    std::mutex last_frame_mutex_;

    ros::NodeHandle nh_, np_;
    ros::ServiceServer dump_srv_;
    ros::ServiceServer config_srv_;
    ros::ServiceServer start_srv_;
    ros::ServiceServer stop_srv_;
    ros::Timer timer_;

    ros::Subscriber exp_time_sub_;
    ros::Subscriber exp_times_sub_;

    std::unique_ptr<image_transport::ImageTransport> it_;
    std::vector<ros::Publisher> cloud_pubs_;
    std::vector<ros::Publisher> exposure_pubs_;
    std::vector<image_transport::Publisher> noise_pubs_;
    std::vector<image_transport::Publisher> gray_pubs_;
    std::vector<image_transport::Publisher> conf_pubs_;
    std::vector<image_transport::Publisher> xyz_pubs_;
    // REP 104 suggests publishing the intrinsics with every frame
    // see: http://www.ros.org/reps/rep-0104.html
    //
    // So, we register each calibration message to a frame and on each stream
    // for mixed-mode use cases.
    std::vector<ros::Publisher> intrinsic_pubs_;
    sensor_msgs::CameraInfo intrinsic_msg_;
    std::mutex intrinsic_mutex_;

    std::string current_use_case_;
    std::mutex current_use_case_mutex_;
    std::map<std::string, std::vector<std::uint16_t> > stream_id_lut_;

  }; // end: class CameraNodelet

} // end: namespace royale_ros

#endif // __ROYALE_ROS_CAMERA_NODELET_H__
