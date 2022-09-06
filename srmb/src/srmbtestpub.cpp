
#include "srmb.h"

int main(int argc, char **argv)
{ 
	ros::init(argc,argv,"srmbtestpub");
	ros::NodeHandle nh;
	
	srmb SRMB(nh);
    SRMB.param_load();


	std_msgs::String msg;
	std::stringstream ss;
	ss << "Hello!";
	msg.data = ss.str();
        
     if (SRMB.init_client(0) == false)
	{
		ROS_INFO("srmb init client:0 error");
	}
	ros::Rate loop_rate(10);
    while(ros::ok())
	{
		SRMB.client_send<std_msgs::String>(msg,"testmsg","String",0);
		loop_rate.sleep();
   }
	return 0;
}
