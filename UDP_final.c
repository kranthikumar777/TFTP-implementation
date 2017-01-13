//Insert Header Files
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

//Define Statements
#define TO 1
#define Window_Size 10

//Op Codes
#define RRQ 1
#define DATA 3
#define ACK 4
#define ERROR 5

int main()
{
//Declare Variables Here
int port_num,rcv_sock,socket_created,client_len,recv_check,error,bytes,send_check,ack_num,prev_ack_num;
//char quit_check; 
socket_created=0;
prev_ack_num=0; 
struct timeval tv;
struct sockaddr_storage client_addr;
tv.tv_sec=TO;
struct sockaddr_in rcv, client;
socklen_t client_addr_len; 
//fd_set read_fds;
time_t curr_time;
time_t prev_time;
char data_send[516]; //Data which will be sent to client
char data_recv[516]; //Data received from Client
char data[512]; //Data from file will be put in data_send
char file_name[2];
int data_size=516; 
FILE *fP; //File pointer

client_len=sizeof client;

printf("Code has a timeout of %f seconds \n",(float)tv.tv_sec); 

//Create Socket
rcv_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //Uses datagrams for UDP implementation
		if(rcv_sock<0)  //Check if socket is created successfully
		{
		printf("Error Attempting to Open Socket");
		}
	printf("Socket Created\n"); 


//Clear socket info 
bzero((char *) &rcv, sizeof(rcv)); //Clear receive socket
bzero((char *) &client, sizeof(client)); //Clear send socket

//Allow multiple clients
//insert setsockopt command

//Take user input for port number
//printf("Enter the port number:\n");
//scanf("%d",&port_num);
port_num=5003; 

//Set Socket Info
rcv.sin_family=AF_INET; //Uses IP_V4
rcv.sin_addr.s_addr = htonl(INADDR_ANY); //Use IP Address of Server
rcv.sin_port = htons(port_num);	 //Set port number provided by user



//Bind Socket 
		if((bind(rcv_sock, (struct sockaddr *) &rcv, sizeof(rcv)) <0))
		{
		printf("Error Attempting to Bind Socket\n"); 
		perror("Error"); 
		}
 	
	printf("Socket is Bound \n");
	//FD_ZERO(&read_fds);
	//FD_SET(rcv_sock,&read_fds);
	(void) time(&prev_time); //Update prev time to so initial timeout doesn't occur  
	socket_created=1;
	while(socket_created)
 	{ 	
	//printf("Entering while loop \n");
	(void) time(&curr_time); //Update Current time to Check for Timeout
		if(curr_time-prev_time>TO)
		{
		printf("Timeout occured must retransmit data"); 
		send_check=sendto(rcv_sock,data_send,data_size,0,(struct sockaddr *)&client_addr, client_len);
			if(send_check<0) //Error on Send
			{
			printf("Error sending packet"); 		
			}
		printf("Packet[%d] sending \n",ack_num+1);
		(void) time(&prev_time); //Update previous time to be used in timeout check
		}
	
	//Check Socket for Data
	//data_check = select (rcv_sock+1,&read_fds, NULL, NULL, &tv); //Checks socket for Data until timeout occurs
	
	//if (data_check<=0) //A timeout has occured
	//	{
		//begin while loop again due to select timeout
	//	continue;
	//	}

	bzero(data_recv,sizeof(data_recv));	
	//Read Data from Client 	
	recv_check=recvfrom(rcv_sock,data_recv,sizeof(data_recv),0,(struct sockaddr *)&client_addr,&client_addr_len);
	//printf("%d \n", recv_check); used to check the number of bytes received 
	if (recv_check<0)
	{
	printf("Error");
	exit(1);	
	}
			

 
	//Determine Opcode from message	
	int opcode;
	opcode=(ntohs(data_recv[1]))/256;  //experimented to find this, possibly due to netascii 
	//printf("Opcode is %d",opcode); used to test the opcode received 
	
	//Use Opcode to decode each message and respond 
		if(opcode==RRQ) //Read Request Received 
		{
		//Decode message into Filename and Mode  
		strcpy(file_name,&data_recv[2]);
	        printf("Read ReQuest received for %s\n",file_name);
	        fP=fopen(file_name,"r"); //read file
        if (fP==NULL)
        {
            error=1;//error code 1
            printf("File not found");
            data_send[0]=0; 
            data_send[1]=5;//opcode for error is 5
            data_send[2]=0;
            data_send[3]=error;//error number for "file not found" is 1
           strcpy(&data_send[4],"File not found");
            if(sendto(rcv_sock,data_send,sizeof(data_send),0,(struct sockaddr *)&client_addr, client_len)<0)
            {printf("Error, cannot send Data\n");exit(1);}
        }
        else
        {
            error=0;
            printf("Successfully opened the file\n");
	            
		    //Prepare header of Op Code and Block Number for message
		    data_send[0]=0; 
            	    data_send[1]=3;//opcode for Data is 3
            	    data_send[2]=0;
            	    data_send[3]=1;// First Block

		    bytes=fread(data,1,512,fP); //Read 512 bytes for data packet 
		    
		    //Check if it has reached end of file 
		    if(bytes<512) 
			{
				if(bytes==0)
				{
				data_send[4]='\0'; //send message with no data to indicate EOF
				}
				else
				{
				//place remaining bytes into final message
				strcpy(&data_send[4],data); //puts data into message
				} 
			}
			else //Has not reached end of file
			{
			strcpy(&data_send[4],data); //puts data into message
			}
   	 		    //Send first message of Data to Client
			data_size = bytes+4;
			send_check=sendto(rcv_sock,data_send,data_size,0,(struct sockaddr *)&client_addr, client_len);
			printf("Packet[1] sending \n"); //First packet is sent
			if(send_check<0) //Error on Send
			{
			printf("Error sending packet"); 		
			}
			(void) time(&prev_time); //Record the time that the first packet was sent
        	
	}
}
		if(opcode==DATA)
		{
		//Decode message into Block # and Data
		//Won't be used since Read Only 
		printf("Error, should not receive op code of DATA \n"); 
		}
		if(opcode==ACK)
		{
			//Decode message into Block #
		
			ack_num=(ntohs(data_recv[3]))/256;		
			printf("Ack %d Received \n",ack_num);
			
			//Check for Duplicate Acks
			if(prev_ack_num==ack_num)
			{
			printf("Duplicate Ack Received");
			}
		    	else
			{
			prev_ack_num=ack_num; //Store the previous ack to check for duplicate acks
			//Prepare Header for Message
			data_send[0]=0; 
            	    	data_send[1]=3;//opcode for Data is 3
            	    	data_send[2]=0;
            	    	data_send[3]=ack_num+1;// block number is packet following last ack_num 
			//Read in Data to Send in Message
			bytes=fread(data,1,512,fP); //Read 512 bytes for data packet 
		    
		   	 //Check if it has reached end of file 
		   	 if(bytes<512) 
				{
					if(bytes==0)
					{
					data_send[4]='\0'; //send message with no data to indicate EOF
					}
					else
					{
					//place remaining bytes into final message
					strcpy(&data_send[4],data); //puts data into message
					printf("Packet[%d] sending \n",ack_num+1);
					} 
				}
			else //Has not reached end of file
			{
			strcpy(&data_send[4],data); //puts data into message
			printf("Packet[%d] sending \n",ack_num+1);
			}
   	 		    //Send Data to Client
			int data_size = bytes+4;
			send_check=sendto(rcv_sock,data_send,data_size,0,(struct sockaddr *)&client_addr, client_len);
			if(send_check<0) //Error on Send
			{
			printf("Error sending packet"); 		
			}
			(void) time(&prev_time); //Update previous time to be used in timeout check
			}
		}
		if(opcode==ERROR)
		{
		//Decode message into Error Code and Error Message
		printf("Stuck here"); 
		}
		
		//printf("Enter q to quit \n");
		//scanf("%s",quit_check); 
		//if(strcmp(quit_check,"q"))
		//{
		//break; 		
		//}
		//FD_ZERO(&read_fds);
		//FD_SET(rcv_sock,&read_fds);
	}
	exit(1);	
}

