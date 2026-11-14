#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav_msgs/msg/occupancy_grid.hpp"
#include "nav_msgs/msg/path.hpp"
class Test1 : public rclcpp::Node{
public:
    short x;
    short y;
    rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr publisher;
    Test1(const std::string& name):Node(name), x(0), y(0) {
        publisher = this->create_publisher<nav_msgs::msg::Path>("test", 10);
        timer_ = this->create_wall_timer(std::chrono::milliseconds(1000),std::bind(&Test1::pub, this));
    }
    void pub(){
        auto message = geometry_msgs::msg::PoseStamped();
        message.header.stamp = now();
        message.header.frame_id = "map";
        message.pose.position.x = 0;
        message.pose.position.y = 0;
        message.pose.position.z = 0;
        message.pose.orientation.w = 1;
        nav_msgs::msg::Path path;
        path.header = message.header;
        path.poses.push_back(message);
        message.pose.position.x = 10;
        message.pose.position.y = 10;
        path.poses.push_back(message);
        publisher->publish(path);
    }
    rclcpp::TimerBase::SharedPtr timer_;
};
int main(int argc, char *argv[]){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Test1>("test1");
    rclcpp::spin(node);
    rclcpp::shutdown();
}