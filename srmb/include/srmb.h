#include "ros/ros.h"
#include<stdio.h>  
#include<stdlib.h>  
#include<netinet/in.h>  
#include<sys/socket.h>  
#include<arpa/inet.h>  
#include<string.h>  
#include<unistd.h>  
#include<geometry_msgs/PoseWithCovarianceStamped.h>
#include<std_msgs/String.h>

#define BACKLOG 5     //完成三次握手但没有accept的队列的长度  
#define CON_MAX 10   //应用层同时可以处理的连接  


#define BUFFER_SIZE 1024*4  
#define NAME_SIZE 50  
#define TYPE_SIZE 50 


class srmb
{
private:
public:
//*************ros变量********************//
    ros::Publisher pub;
    ros::NodeHandle nh;
//*************server变量*****************//
    int client_fds[CON_MAX];
    struct sockaddr_in server_addr_local;  
    int server_sock_fd_local;
    int bind_result;
    fd_set server_fd_set_local;  
    int max_fd = -1;
//*************client变量*****************//
    struct sockaddr_in server_addr_remote[CON_MAX];
    int server_sock_fd_remote[CON_MAX];
    fd_set client_fd_set[CON_MAX];
//*************公共变量********************// 
    //struct timeval tv;
    std::string ip[CON_MAX];
    int self_id;
    int cluster_num;
    int  Pay_loadLen = BUFFER_SIZE;
    int  recvlen = 0;
    int  port[CON_MAX];
    char clien_send_buffer[BUFFER_SIZE];
    char server_recv_buffer[BUFFER_SIZE];
    char ReceiveBuff[BUFFER_SIZE];
    char name_buffer[NAME_SIZE];
    char type_buffer[TYPE_SIZE];
//*************用户接口***********************//
    bool init_server();
    void server_recv_thread();
    bool init_client(int id);

    template<typename T>
    void client_send(T msg,std::string  name,std::string  type,int id)
    {
        char namebuf[NAME_SIZE];
        char typebuf[TYPE_SIZE];
        strcpy(namebuf,name.c_str());
        strcpy(typebuf,type.c_str());
        FD_ZERO(&srmb::client_fd_set[id]);  
        FD_SET(srmb::server_sock_fd_remote[id], &srmb::client_fd_set[id]);  
        std::vector<uint8_t> vec;//用vec是因为boost buffer memcopy
        uint32_t serial_size = ros::serialization::serializationLength(msg);
        boost::shared_array<uint8_t> buffer(new uint8_t[serial_size]);
        ros::serialization::OStream stream(buffer.get(), serial_size);
        ros::serialization::serialize(stream, msg);
        vec.resize(serial_size);
        std::copy(buffer.get(), buffer.get() + serial_size, vec.begin());
        unsigned int msg_lenth = serial_size;
        //读出数据，发给对方
        memset(srmb::clien_send_buffer, 0, BUFFER_SIZE * sizeof(char));
        memcpy(srmb::clien_send_buffer, namebuf, NAME_SIZE * sizeof(char));
        memcpy(srmb::clien_send_buffer + 50, typebuf, TYPE_SIZE * sizeof(char));
        memcpy(srmb::clien_send_buffer + 100, &msg_lenth, sizeof(&msg_lenth));
        memcpy(srmb::clien_send_buffer + 104, &vec[0], vec.size() * sizeof(uint8_t));
        if(send(srmb::server_sock_fd_remote[id], srmb::clien_send_buffer, sizeof(srmb::clien_send_buffer), 0) == -1)  
        {
            perror("发送消息出错!\n");  
        }
    }


//*************其他函数***********************//
    srmb(ros::NodeHandle &nh);
    ~srmb();
    void param_load();
    bool server_unserialize_to_local(char  name[NAME_SIZE], char type[TYPE_SIZE] , uint32_t serial_size , std::vector<uint8_t>  vec);
};
