/* Programming Assignment 1
Submitted by: Arushi Sabharwal
Student ID - W1468298*/

#include<stdio.h>
#include<strings.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<stdint.h>																										//Library for unsigned int data type
#include<sys/time.h> 																									//Library to access timeval
#include<unistd.h> 																										//Library for close function of socket

#define START_PACKET_ID 0xFFFF
#define CLIENT_ID 0xFF
#define LENGTH 0xFF
#define END_PACKET_ID 0xFFFF
#define DATA_PACKET_TYPE 0xFFF1
#define ACK_PACKET_TYPE 0xFFF2
#define REJECT_PACKET_TYPE 0xFFF3
#define OUT_OF_SEQUENCE_CODE 0xFFF4
#define LENGTH_MISMATCH 0xFFF5
#define END_OF_PACKET_ID_MISSING 0xFFF6
#define DUPLICATE_PACKET 0xFFF7
#define CLOSING_PACKET 0xFFF8																							//Creating a separate type for closing packet
#define TIME_OUT 3

struct data_packet
{
	uint16_t start_packetID;
	uint8_t clientID;
	uint16_t packet_type;
	uint8_t segment_no;
	uint8_t length;
	char payload[255];
	uint16_t end_packetID;
};

struct reject_packet
{
	uint16_t start_packetID;
	uint8_t clientID;
	uint16_t packet_type;
	uint16_t reject_subcode;
	uint8_t received_segment_no;
	uint16_t end_packetID;
};


void error(char *msg)
{
	perror(msg);
	exit(0);
}

struct data_packet initialize()																							//Function to load the data packet
{
	struct data_packet data;
	data.start_packetID = START_PACKET_ID;
	data.clientID = CLIENT_ID;
	data.segment_no = 0;
	data.packet_type = DATA_PACKET_TYPE;
	data.end_packetID = END_PACKET_ID;
	return data;
}

void print_packet_details(struct data_packet data)																		//Function to print the details of the data packets
{
		printf("\nSending the below packet to server:");
		printf("\n%x\t%hhx\t%x\t%d\t%d\t%s\t %x\t\n", data.start_packetID, data.clientID,
					data.packet_type, data.segment_no,data.length, data.payload, data.end_packetID );
}

void print_received_packet_details(struct reject_packet data)															//Function to print the details of the received packets
{
	if(data.packet_type==ACK_PACKET_TYPE)																				//This condition checks the type of packet if they are of ACK or REJECT Type
	{
		printf("\n%x\t%hhx\t%x\t%d\t%x\t\n", data.start_packetID, data.clientID, data.packet_type,
		 			data.received_segment_no, data.end_packetID );
	}
	else
	printf("\n%x\t%hhx\t%x\t%x\t%d\t%x\t\n", data.start_packetID, data.clientID, data.packet_type,data.reject_subcode,
					data.received_segment_no, data.end_packetID );
}

int main( int argc, char *argv[])
{
	struct data_packet data;
	struct reject_packet receivedpacket;
	int sock, length, x, n;
	struct sockaddr_in server,from;
	struct hostent *hp;
	int counter;
	int segmentNo = 1;
	int userSegmentNo;
	int option, flag;

	if(argc!=3) 																										//Condition to check that the user entered 3 arguments at commandline
	{
		printf("You did not enter port correctly\n");
		exit(1);
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);

	if(sock<0)
	{
		error("socket not created properly");
	}

	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	hp = gethostbyname(argv[1]); 																						//Converting "localhost" to network readable format

	if(hp==0)
	{
		error("Unknown host");
	}

	bcopy((char*)hp->h_addr, (char*)&server.sin_addr, hp->h_length);
	server.sin_port=htons(atoi(argv[2]));
	length = sizeof(struct sockaddr_in);

	struct timeval t;																									//Configuring the socket to timeout in 3 seconds
	t.tv_sec = TIME_OUT; 																								//3 seconds timeout
	t.tv_usec = 0;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&t,sizeof(struct timeval))>0)
	{																													//To set options at socket level, we set level = SOL_SOCKET and SO_RCVTIMEO is used to set
			error("Error setting timeout");																				//timeout value for recvfrom() Function and the value of time out is taken from the timeval variable that we pass
	}
	option = -1; 																										//Variable to store the option value as entered by user
	char sel;

do {
	data = initialize();																								//Initializing the packet details common for all inputs entered by user
	printf("\n\n***************************************************************************************************\n");
	printf("***************************************************************************************************\n\n");

	printf("Menu \n");
	printf("1. Send Correct Packet \n");
	printf("2. Send a out of sequence Packet \n");
	printf("3. Send a length mismatch Packet \n");
	printf("4. Send a Packet with incorrect end of packet identifier \n");
	printf("5. Send a Duplicate Packet \n");
	printf("0. Exit \n");
	printf("\nPlease enter your choice: ");
	scanf("%d",&option);

	switch(option) {

		case 1 :																										//Initializing the correct packet
		data.segment_no = segmentNo;
		sprintf(data.payload, "This is the payload for Packet %d" , segmentNo);
		data.length = strlen(data.payload);
		segmentNo++;
		print_packet_details(data);
		break;

		case 2 :																										//Initializing the out of sequence packet
		printf("\nPlease enter the segment number: ");
		scanf("%d", &userSegmentNo);
		if(userSegmentNo==segmentNo)
		{
			segmentNo++;
		}
		data.segment_no = userSegmentNo;
		sprintf(data.payload, "This is the payload for Packet %d" , segmentNo);
		data.length = strlen(data.payload);
		print_packet_details(data);
		break;

		case 3 :																										//Initializing the length mismatch packet
		data.segment_no = segmentNo;
		sprintf(data.payload, "This is the payload for Packet %d" , segmentNo);
		data.length = strlen(data.payload) + 2;
		print_packet_details(data);
		break;

		case 4:																											//Initializing the incorrect end of identifier packet
		data.segment_no = segmentNo;
		sprintf(data.payload, "This is the payload for Packet %d" , segmentNo);
		data.length = strlen(data.payload);
		data.end_packetID = 0x0000;
		print_packet_details(data);
		break;

		case 5:																											//Initializing the duplicate packet
		if(segmentNo == 1){
			printf("You have not sent a packet yet \n");
			break;
		}
		printf("\nPlease enter the segment number: ");
		scanf("%d", &userSegmentNo);
		if(userSegmentNo==segmentNo)
		{
			segmentNo++;
		}
		data.segment_no = userSegmentNo;
		sprintf(data.payload, "This is the payload for Packet %d" , segmentNo);
		data.length = strlen(data.payload);
		print_packet_details(data);
		break;

		case 0:
			data = initialize();
			data.segment_no = segmentNo;
			data.packet_type = CLOSING_PACKET;																			//Initializing the closing packet
			sprintf(data.payload, "This is the payload for Closing Packet");
			data.length = strlen(data.payload);
			segmentNo++;
			length = sizeof(struct sockaddr_in);
			if(sendto(sock, &data, sizeof(struct data_packet), 0, (struct sockaddr*)&server, length) <0){
				error("sendto");
			}

			close(sock);																								//closing socket at client side
			printf("\nClosing socket!\n\nGood Bye!!!\n");
			exit(0);
			break;

		default:
			printf("Error: Not a valid option. Please Try again!\n");
	}

			n=0; counter=0;
			while(n<=0 && counter<3 && !(option ==5 && segmentNo == 1) && (option>=0 && option<=5)) 					//We want to ensure that a duplicate packet option
			{																											//if triggered before 1st packet is sent doesn't get sent to server

				length = sizeof(struct sockaddr_in);

				if(sendto(sock, &data, sizeof(struct data_packet), 0, (struct sockaddr*)&server, length) <0)
				{
					error("sendto");
				}

				n=recvfrom(sock, &receivedpacket, sizeof(struct reject_packet), 0, (struct sockaddr*)&from, &length);

				if(n<0)
				{
					printf("\nNo response from server for 3 seconds. Trying to send the packet again\n");
					counter++;
				}
				else if(receivedpacket.packet_type ==ACK_PACKET_TYPE) 													//Here we checked the packet type to be ack
				{
					printf("\n\nThe packet received from server is:");
					print_received_packet_details(receivedpacket);
					printf("\n\nAcknowledgement received!\n");
				}
				else if(receivedpacket.packet_type==REJECT_PACKET_TYPE) 												//Here we checked the packet type to be reject type
				{
					if(receivedpacket.reject_subcode == OUT_OF_SEQUENCE_CODE)											//Here we checked the packet type to be reject type and out of sequence
					{
						printf("\n\nThe packet received from server is:");
						print_received_packet_details(receivedpacket);
						printf("\n\nThe Packet was rejected because of OUT of SEQUENCE and type is %x\n", receivedpacket.reject_subcode);
					}
					else if(receivedpacket.reject_subcode == LENGTH_MISMATCH)											//Here we checked the packet type to be reject type and length mismatch
					{
						printf("\n\nThe packet received from server is:");
						print_received_packet_details(receivedpacket);
						printf("\n\nThe Packet was rejected because of length mismatch and type is %x\n", receivedpacket.reject_subcode);
					}
					else if(receivedpacket.reject_subcode == END_OF_PACKET_ID_MISSING)									//Here we checked the packet type to be reject type and End of Pack ID missing
					{
						printf("\n\nThe packet received from server is:");
						print_received_packet_details(receivedpacket);
						printf("\n\nThe Packet was rejected because of end of packet id missing and type is %x\n", receivedpacket.reject_subcode);
					}
					else if(receivedpacket.reject_subcode == DUPLICATE_PACKET)											//Here we checked the packet type to be reject type and Duplicate
					{
						printf("\n\nThe packet received from server is:");
						print_received_packet_details(receivedpacket);
						printf("\n\nThe Packet was rejected because it was duplicate and type is %x\n", receivedpacket.reject_subcode);
					}
				}
			}

			if(counter==3)
			{
				printf("\nServer does not respond\n"); 																	//As the counter reaches 3, we know that this is the
				exit(0);																								//maximum count and we display the msg "server does not respond"
			}

	flag =0;
	do {
		if(flag)
			{
				printf("Error: Not a valid option. Please Try again!\n");
			}
		printf("\nDo you want to send again ? Please enter : Y/N  \n");
		scanf("%s",&sel);
		flag  = 1;
			}	while( sel != 'y' && sel!='Y' &&sel != 'n' && sel!='N');

} while( sel == 'y' || sel == 'Y');

		data = initialize();																							//sending closing packet to server
		data.segment_no = segmentNo;
		data.packet_type = CLOSING_PACKET;
		sprintf(data.payload, "This is the payload for Closing Packet");
		data.length = strlen(data.payload);
		segmentNo++;
		length = sizeof(struct sockaddr_in);
		if(sendto(sock, &data, sizeof(struct data_packet), 0, (struct sockaddr*)&server, length) <0){
			error("sendto");
		}
		close(sock);																									//socket closed at client side
		printf("\nClosing Socket!\n\nGood Bye!!!! \n");

}
