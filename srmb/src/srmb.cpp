#include"srmb.h"
    
bool srmb::init_server()
{
    srmb::server_addr_local.sin_family = AF_INET;  
    srmb::server_addr_local.sin_port = htons(port[self_id]);  
    srmb::server_addr_local.sin_addr.s_addr = inet_addr((srmb::ip[self_id]).c_str()); 
    bzero(&(srmb::server_addr_local.sin_zero), 8);  
    //创建socket  
    srmb::server_sock_fd_local = socket(AF_INET, SOCK_STREAM, 0);  
    if(srmb::server_sock_fd_local == -1)  
    {  
        perror("socket error");  
        return false;  
    }  
    //绑定socket  
    srmb::bind_result = bind(srmb::server_sock_fd_local, (struct sockaddr *)&(srmb::server_addr_local), sizeof(srmb::server_addr_local));  
    if(srmb::bind_result == -1)  
    {  
        perror("bind error");  
        return false;  
    }  
    //listen  
    if(listen(srmb::server_sock_fd_local, BACKLOG) == -1)  
    {  
        perror("listen error");  
        return false;  
    }
    return true; 
}


bool srmb::init_client(int id)
{ 
    srmb::server_addr_remote[id].sin_family = AF_INET;  
    srmb::server_addr_remote[id].sin_port = htons(port[id]);  
    srmb::server_addr_remote[id].sin_addr.s_addr = inet_addr((srmb::ip[id]).c_str());  
    bzero(&(srmb::server_addr_remote[id].sin_zero), 8);
    srmb::server_sock_fd_remote[id] = socket(AF_INET, SOCK_STREAM, 0);  
    if(srmb::server_sock_fd_remote[id] == -1)  
    {  
       ROS_INFO("socket error");  
        return false;
    }
    if(connect(srmb::server_sock_fd_remote[id],(struct sockaddr *)&(srmb::server_addr_remote[id]), sizeof(struct sockaddr_in)) == -1)  
    {
        ROS_INFO("connect error");  
        return false;
    } 
    return true;
}


void srmb::server_recv_thread()
{
    while(1)  
    {  
        //srmb::tv.tv_sec = 20;  
        //srmb::tv.tv_usec = 0;
        FD_ZERO(&(srmb::server_fd_set_local)); 
       //服务器端socket  
        FD_SET(srmb::server_sock_fd_local, &(srmb::server_fd_set_local));    
        if(srmb::max_fd < srmb::server_sock_fd_local)  
        {  
            srmb::max_fd = srmb::server_sock_fd_local;  
        }
        //客户端连接  
        for(int i =0; i < CON_MAX; i++)  
        {  
            if(srmb::client_fds[i] != 0)  
            {  
                FD_SET(srmb::client_fds[i], &(srmb::server_fd_set_local));  
                if(srmb::max_fd < srmb::client_fds[i])  
                {  
                    srmb::max_fd = srmb::client_fds[i];  
                }  
            }  
        }  
        //int ret = select(srmb::max_fd + 1, &srmb::server_fd_set_local, NULL, NULL, &srmb::tv);
        int ret = select(srmb::max_fd + 1 ,  &(srmb::server_fd_set_local), NULL, NULL, NULL);  
        if(ret < 0)  
        {  
            perror("select 出错\n");  
            continue;  
        }  
        else if(ret == 0)  
        {  
            printf("select 超时\n");  
            continue;  
        }
        else  
        {  
            if(FD_ISSET(srmb::server_sock_fd_local, &(srmb::server_fd_set_local)))  
            {  
                //有新的连接请求  
                struct sockaddr_in client_address;  
                socklen_t address_len;  
                int client_sock_fd = accept(srmb::server_sock_fd_local, (struct sockaddr *)&client_address, &address_len); 
                if(client_sock_fd > 0)  
                {  
                    int index = -1;  
                    for(int i = 0; i < CON_MAX; i++)  
                    {  
                        if(srmb::client_fds[i] == 0)  
                        {  
                            index = i;  
                            client_fds[i] = client_sock_fd;  
                            break;  
                        }  
                    }
                    if(index >= 0)  
                    {  
                        printf("新客户端(%d)加入成功 %s:%d\n", index, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));  
                    }  
                    else  
                    {   
                        printf("客户端连接数达到最大值，新客户端加入失败 %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));  
                    }  
                }  
            }

            for(int i =0; i < cluster_num; i++)  
            {  
                if(srmb::client_fds[i] !=0)  
                {  
                    if(FD_ISSET(srmb::client_fds[i], &(srmb::server_fd_set_local)))  
                    {  
                        //处理某个客户端过来的消息  
                        bzero(srmb::server_recv_buffer, BUFFER_SIZE);  
                        bzero(srmb::name_buffer, NAME_SIZE); 
                        bzero(srmb::name_buffer, TYPE_SIZE);  
                        long byte_num = recv(srmb::client_fds[i], srmb::server_recv_buffer, BUFFER_SIZE, 0);  
                        if (byte_num > 0)  
                        {  
                            if(byte_num > BUFFER_SIZE)  
                            {  
                                byte_num = BUFFER_SIZE;  
                            }  
                            srmb::server_recv_buffer[byte_num] = '\0';  
                            uint32_t serial_size;
                            memcpy(srmb::name_buffer, srmb::server_recv_buffer, NAME_SIZE * sizeof(char));
                            memcpy(srmb::type_buffer, srmb::server_recv_buffer + 50, TYPE_SIZE * sizeof(char));
                            memcpy(&serial_size, srmb::server_recv_buffer + 100, sizeof(serial_size));
                            std::vector<uint8_t> buffer(serial_size);
                            memcpy(&buffer[0], srmb::server_recv_buffer + 104, buffer.size() * sizeof(uint8_t));
                            if(srmb::server_unserialize_to_local(srmb::name_buffer , srmb::type_buffer, serial_size , buffer) == false)
                            {
                                printf("消息反序列化出错\n"); 
                            }
                        }  
                        else if(byte_num < 0)  
                        {  
                            printf("从客户端(%d)接受消息出错.\n", i);  
                        }  
                        else  
                        {  
                            FD_CLR(srmb::client_fds[i], &(srmb::server_fd_set_local));  
                            srmb::client_fds[i] = 0;  
                            printf("客户端(%d)退出了\n", i);  
                        }  
                    }  
                }  
            }  
        }  
    }
}   

bool srmb::server_unserialize_to_local(char  name[NAME_SIZE], char type[TYPE_SIZE] , uint32_t serial_size , std::vector<uint8_t>  vec)
{
    char topic_type[50]  = {0};
     std::string string_topic;
    memset(topic_type, 0, sizeof(topic_type));

    string_topic = "PoseWithCovarianceStamped";
    strcpy(topic_type, string_topic.c_str());
    if (strcmp(topic_type, type) == 0)
    {
        std::string topic = name;
        srmb::pub = srmb::nh.advertise<geometry_msgs::PoseWithCovarianceStamped>(topic,10);
        geometry_msgs::PoseWithCovarianceStamped msg;
        ros::serialization::IStream stream(vec.data(), serial_size);
        ros::serialization::Serializer<geometry_msgs::PoseWithCovarianceStamped>::read(stream, msg);
        srmb::pub.publish(msg);
        return true;
    }

    string_topic = "String";
    strcpy(topic_type, string_topic.c_str());
    if (strcmp(topic_type, type) == 0)
    {
        std::string topic = name;
        srmb::pub = srmb::nh.advertise<std_msgs::String>(topic,10);
        std_msgs::String msg;
        ros::serialization::IStream stream(vec.data(), serial_size);
        ros::serialization::Serializer<std_msgs::String>::read(stream, msg);
        srmb::pub.publish(msg);
        return true;
    }
    return false;
}


void srmb::param_load()
{
    srmb::nh.param<int>("server_port/port0",port[0],9000);
    srmb::nh.param<int>("server_port/port1",port[1],9001);
    srmb::nh.param<int>("server_port/port2",port[2],9002);
    srmb::nh.param<int>("server_port/port3",port[3],9003);
    srmb::nh.param<int>("server_port/port4",port[4],9004);
    srmb::nh.param<int>("server_port/port5",port[5],9005);
    srmb::nh.param<int>("server_port/port6",port[6],9006);
    srmb::nh.param<int>("server_port/port7",port[7],9007);
    srmb::nh.param<int>("server_port/port8",port[8],9008);
    srmb::nh.param<int>("server_port/port9",port[9],9009);
    srmb::nh.param<std::string>("remote_server_ip/ip0",ip[0],"192.168.20.100");
    srmb::nh.param<std::string>("remote_server_ip/ip1",ip[1],"192.168.20.101");
    srmb::nh.param<std::string>("remote_server_ip/ip2",ip[2],"192.168.20.102");
    srmb::nh.param<std::string>("remote_server_ip/ip3",ip[3],"192.168.20.103");
    srmb::nh.param<std::string>("remote_server_ip/ip4",ip[4],"192.168.20.104");
    srmb::nh.param<std::string>("remote_server_ip/ip5",ip[5],"192.168.20.105");
    srmb::nh.param<std::string>("remote_server_ip/ip6",ip[6],"192.168.20.106");
    srmb::nh.param<std::string>("remote_server_ip/ip7",ip[7],"192.168.20.107");
    srmb::nh.param<std::string>("remote_server_ip/ip8",ip[8],"192.168.20.108");
    srmb::nh.param<std::string>("remote_server_ip/ip9",ip[9],"192.168.20.109");
    srmb::nh.param<int>("common/cluster_num",cluster_num,10);
    srmb::nh.param<int>("common/sefl_id",self_id,0);

}

srmb::srmb(ros::NodeHandle &nh)
{
    this->nh = nh;
}

srmb::~srmb()
{
}
