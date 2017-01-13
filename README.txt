# TFTP-implementation
ECEN602 Programming Assignment 02
----------------------------------------------
Team Number:23
Member 1: Matthew Roe          (UIN: 321007055)
Member 2: Jifang Mu            (UIN: 821005313)
Member 3: Kranthi Kumar Katam  (UIN: 225009204)
-----------------------------------------------
In this project assignment, we built a UDP server. This was tested with client code found on github. The client code is included to show that it is working, in case it does not work with the Client.
----------------------------------------------
The package contains 2 files (1 optional):
1.server.c
2.makefile

(optional client code used for testing)
***Client code is not ours, only used to test our server code. This was obtained from https://github.com/ideawu/tftpx
-----------------------------------------------
Functions implemented:
sendto() This is used to send datagram packets
recvfrom() This is used to receive datagram packets
time() This is used to determine current time of system

Functions attempted to implement:
select() This is used to handle multiple clients
-----------------------------------------------
server.c
1) Server creates a socket and binds it to the local IP address of the server and port number 5003 (by default) this can be adjusted. 
2) The server uses TFTP RFC 1350, with datagram packets. 
3) The server waits until it receives a RRQ packet.
4) After receiving an RRQ packet, decodes the message into opcode and filename.
5) The server uses file pointer to open and read from a file. If the file does not exist it sends an error message to the client.
6) It reads 512 bytes of data at a time, sending the data to the client. It then waits until an ACK is received or a timeout occurs.
7) Upon timeout, the server will resend the data packet.
8) After an ACK is received it will continue reading in 512 bytes of data, sending packets and waiting for acks until an EOF is reached.
9) The server is able to determine if it reaches EOF file if there are less than 512 bytes of data to read. The remaining bits are packed into a final packet and sent to the client.
10) If the bytes are a multiple of 512, leaving 0 bytes, it will send a packet with \0 (empty) to indicate EOF is reached to client.
11) Timeouts are obtained by measuring the time with time() function. This is recorded at the start of the while loop and is measured against the last previous transmission time. 

client.c 
1) This is used to act as a TFTP client. For testing purposes it writes the results of get command to test2.txt. This was used to distinguish between original file and file obtained via TFTP.
  
-----------------------------------------------

To compile the code without included client, run the makefile: make -f make_udp
Run the server by using the command line: ./server_udp

To compile the code with included client run the makefile make -f make_w_client
Run the server by using the command line: ./server_udp
Run the client by using the command line: ./client_udp 127.0.0.1 5003 
The result of TFTP get command will be stored into test2.txt (this was changed from source for testing purposes)

***This is not our client code so it is not cleaned up, will display warning with makefile
-----------------------------------------------
Structure of Code

int main()
{
Define Variables

1) Create and Bind Sockets

while
{
11) Check for timeout at the start of each loop iteration
	if(timeout occured)
	{
	7) Resend Data
	}
3) Receive a RRQ request
4) Decode RRQ into opcode and filename
	if(RRQ received)
	{
	5) Open and read file using filename
	6) Send first data packet
	}
	if(ACK received)
	{
	8) Read in next bytes of data and send the next message
	}
}
