#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav_msgs/msg/occupancy_grid.hpp"
#include "nav_msgs/msg/path.hpp"
#include<iostream>
#include<cmath>
#include<vector>
#include<cstdint>
#include<queue>
#include<utility>
#include<functional>
using entry = std::pair<float,int>;
class Test2 : public rclcpp::Node
{
public:
  rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr map_;
  rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr goal_;
  rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr publisher;
  std::vector<std::uint8_t> digit_map;
  double resolution;
  int width,height;
  float start_x,start_y;
  geometry_msgs::msg::Pose origin;
  Test2(const std::string& name)
  : Node(name)
  {
    goal_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
      "/goal_pose", 10,
      std::bind(&Test2::plan, this, std::placeholders::_1));
    map_ = this->create_subscription<nav_msgs::msg::OccupancyGrid>(
      "/costmap/costmap", 10,
      std::bind(&Test2::update, this, std::placeholders::_1));
    publisher = this->create_publisher<nav_msgs::msg::Path>("plan", 10);
  }
  void update(const nav_msgs::msg::OccupancyGrid & msg) {
    digit_map.assign(msg.data.begin(),msg.data.end());
    resolution = msg.info.resolution;
    width = msg.info.width;
    height = msg.info.height;
    origin = msg.info.origin;
  }
  float g(int idx){
    return digit_map[idx]/98.0f+1;
  }
  void plan(const geometry_msgs::msg::PoseStamped & msg){
    std::vector<int> parent(digit_map.size(),-1);
    std::vector<float> value(digit_map.size(),-1.0f);
    std::priority_queue<entry,std::vector<entry>,std::greater<entry>> find;
    start_x=0;start_y=0;
    float origin_x = origin.position.x;
    float origin_y = origin.position.y;
    float goal_x = msg.pose.position.x;
    float goal_y = msg.pose.position.y;
    float dx = goal_x-origin_x;
    float dy = goal_y-origin_y;
    int goal_c=std::floor(dx/resolution);
    int goal_r=std::floor(dy/resolution);
    int goal_idx = goal_c+goal_r*width;
    float dx2 = start_x-origin_x;
    float dy2 = start_y-origin_y;
    int start_c=std::floor(dx2/resolution);
    int start_r=std::floor(dy2/resolution);
    int start_idx = start_c+start_r*width;
    value[start_idx]=0;
    find.push({value[start_idx],start_idx});
    while (!find.empty()){
      float val = find.top().first;
      int idx = find.top().second;
      find.pop();
      if((idx%width!=width-1&&digit_map[idx+1]<98)&&(value[idx+1]<0||value[idx+1]>g(idx+1)+val)){
        parent[idx+1]=idx;
        value[idx+1]=g(idx+1)+val;
        find.push({value[idx+1],idx+1});
      }
      if((idx%width!=0&&digit_map[idx-1]<98)&&(value[idx-1]<0||value[idx-1]>g(idx-1)+val)){
        parent[idx-1]=idx;
        value[idx-1]=g(idx-1)+val;
        find.push({value[idx-1],idx-1});
      }
      if((idx/width!=height-1&&digit_map[idx+width]<98)&&(value[idx+width]<0||value[idx+width]>g(idx+width)+val)){
        parent[idx+width]=idx;
        value[idx+width]=g(idx+width)+val;
        find.push({value[idx+width],idx+width});
      }
      if((idx/width!=0&&digit_map[idx-width]<98)&&(value[idx-width]<0||value[idx-width]>g(idx-width)+val)){
        parent[idx-width]=idx;
        value[idx-width]=g(idx-width)+val;
        find.push({value[idx-width],idx-width});
      }
    }
    int i = goal_idx;
    if(parent[i]<0) return;
    auto message = geometry_msgs::msg::PoseStamped();
    message.header.stamp = now();
    message.header.frame_id = "map";
    message.pose.position.x = (i%width+0.5)*resolution+origin_x;
    message.pose.position.y = (i/width+0.5)*resolution+origin_y;
    message.pose.position.z = 0;
    message.pose.orientation.w = 1;
    nav_msgs::msg::Path path;
    path.header = message.header;
    path.poses.push_back(message);
    while (parent[i]>=0){
      i=parent[i];
      message.pose.position.x = (i%width+0.5)*resolution+origin_x;
      message.pose.position.y = (i/width+0.5)*resolution+origin_y;
      path.poses.push_back(message);
    }
    publisher->publish(path);
  }
};
int main(int argc, char *argv[]){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Test2>("test2");
    rclcpp::spin(node);
    rclcpp::shutdown();
}