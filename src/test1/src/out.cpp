#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
class Test1 : public rclcpp::Node{
public:
    short x;
    short y;
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr publisher;
    Test1(const std::string& name):Node(name), x(0), y(0) {
        publisher = this->create_publisher<geometry_msgs::msg::PoseStamped>("test", 10);
        timer_ = this->create_wall_timer(std::chrono::milliseconds(1000),std::bind(&Test1::pub, this));
    }
    void pub(){
        auto message = geometry_msgs::msg::PoseStamped();
        message.pose.position.x = x;
        message.pose.position.y = y;
        publisher->publish(message);
    }
    rclcpp::TimerBase::SharedPtr timer_;
};
int main(int argc, char *argv[]){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Test1>("test1");
    rclcpp::spin(node);
    rclcpp::shutdown();
}