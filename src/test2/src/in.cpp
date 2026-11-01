#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
class Test2 : public rclcpp::Node
{
public:
  Test2(const std::string& name)
  : Node(name)
  {
    subscription_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
      "test", 10,
      std::bind(&Test2::listen, this, std::placeholders::_1));
  }
  void listen(const geometry_msgs::msg::PoseStamped & msg) const
  {
    RCLCPP_INFO(this->get_logger(), "'%f' '%f'", msg.pose.position.x, msg.pose.position.y);
  }

  rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr subscription_;
};
int main(int argc, char *argv[]){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Test2>("test2");
    rclcpp::spin(node);
    rclcpp::shutdown();
}