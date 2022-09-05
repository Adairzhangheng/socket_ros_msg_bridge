
#include "srmb.h"
#include <thread>

int main(int argc, char **argv)
{
	ros::init(argc,argv,"srmbtestpub");
	ros::NodeHandle nh;
	
	srmb SRMB(nh);
	SRMB.param_load();

	if (SRMB.init_server() == false)
	{
		ROS_INFO("srmb init server error");
	}

	SRMB.server_recv_thread();
	return 0;
}
