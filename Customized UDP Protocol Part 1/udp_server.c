/* Programming Assignment 1
Submitted by: Arushi Sabharwal
Student ID - W1468298*/

#include<strings.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdint.h>																											//Library for unsigned int datatype
#include<unistd.h>																											//Library for close function of socket


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
#define CLOSING_PACKET 0xFFF8																								//Creating a separate type for closing packet

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

void error( char *msg)
{

	perror(msg);
	exit(0);
}

void print_packet_details(struct data_packet data)																			//Function to print the details of the data packets
{
	printf("Received the following from client:\n\n");
	printf("\n%x\t%hhx\t%x\t%d\t%d\t%s\t%x\t\n", data.start_packetID,
	data.clientID, data.packet_type, data.segment_no,data.length, data.payload, data.end_packetID );

}

struct reject_packet CreateAckPacket(struct data_packet data)																//Function to create an ack packet
{
	struct reject_packet ack;
	ack.start_packetID = data.start_packetID;
	ack.clientID = data.clientID;
	ack.packet_type = ACK_PACKET_TYPE;
	ack.reject_subcode=0;
	ack.received_segment_no= data.segment_no;
	ack.end_packetID = data.end_packetID;
	return ack;
}

struct reject_packet CreateRejectPacket(struct data_packet data)															//Function to create a reject packet
{
		struct reject_packet reject;
		reject.start_packetID = data.start_packetID;
		reject.clientID = data.clientID;
		reject.packet_type = REJECT_PACKET_TYPE;
		reject.received_segment_no= data.segment_no;
		reject.end_packetID = data.end_packetID;
		return reject;
}

void print_ackreject_packet_details(struct reject_packet data)																//Function to print the details of the ack/reject packets
{
		if(data.packet_type==ACK_PACKET_TYPE)
		{
			printf("\n%x\t%hhx\t%x\t%d\t%x\t\n", data.start_packetID, data.clientID, data.packet_type,
										data.received_segment_no, data.end_packetID );
		}
		else
		printf("\n%x\t%hhx\t%x\t%x\t%d\t%x\t\n", data.start_packetID, data.clientID, data.packet_type,
										data.reject_subcode, data.received_segment_no, data.end_packetID );
}

int main(int argc, char *argv[])
{
		int sock, add_length, fromlen, n;
		struct sockaddr_in server;
		struct sockaddr_in from;
		struct data_packet data;
		struct reject_packet reject;
		struct reject_packet ack;

		if(argc<2)
		{
			printf("No port number provided");
			exit(0);
		}

		sock = socket(AF_INET, SOCK_DGRAM, 0);																				//Here we have mentioned AF_INET to notify Internet domain and SOCK_DGRAM shows that we are creating
																															//this socket for UDP protocol, 0 shows that use the regular UDP protocol, no change
		if(sock <0) 																										//If socket is not created properly socket fucntion returns a value <0
		{
			error("Opening socket");
		}
		int receivedPacketNo = 0;
		add_length = sizeof(server);
		bzero(&server, add_length);																							//Clearing the server structure variable, so that any garbage value is removed
																															//Here we are setting the address for the server based on the inputs we provided in command line
		server.sin_family = AF_INET; 																						//Here we are using Internet address in the family
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = htons(atoi(argv[1])); 																			//atoi converts port number argument to integer which is further
																															//converted to network readable format by htons(host to network short)
		if(bind(sock, (struct sockaddr*)&server, add_length)<0)
		{
			error("Binding the socket to address");
		}

		fromlen = sizeof(struct sockaddr_in);
		printf("Welcome to the Server! \n");

		while(1)																											//Creating an infinite loop for the server so that it can accept multiple user's request and keeps running
		{
				printf("\n\n***************************************************************************************************\n");
				printf("***************************************************************************************************\n\n");

				n=recvfrom(sock,&data,sizeof(struct data_packet),0,(struct sockaddr*)&from, &fromlen);

			if(n<0)
			{
				error("recvfrom");
			}

			if(data.packet_type == CLOSING_PACKET)																			//Condition to check closing packet
			{
				printf("\n\nClosing socket\n\nGood Bye!\n");
				close(sock);																								//Socket closed at server side
				exit(0);
			}
			printf("New Packet from client:\n(Expected Packet Number: %d   Received Packet Number: %d)\n\n",
															receivedPacketNo+1, data.segment_no);

			print_packet_details(data);

			int length = strlen(data.payload);

			if(data.segment_no<=receivedPacketNo)																			//Condition to check duplicate packet
			{
				reject = CreateRejectPacket(data);
				reject.reject_subcode = DUPLICATE_PACKET;
				sendto(sock, &reject, sizeof(struct reject_packet), 0, (struct sockaddr *)&from,fromlen);
				printf("\n\nDuplicate packed is received!\n");
				printf("Sending the below packet to client as rejection:\n\n");
				print_ackreject_packet_details(reject);
			}
			else if(data.length!=length)																					//condition to check length mismatch
			{
				reject = CreateRejectPacket(data);
				reject.reject_subcode = LENGTH_MISMATCH;
				sendto(sock, &reject, sizeof(struct reject_packet), 0, (struct sockaddr *)&from,fromlen);
				printf("\n\nLength is mismatched! \nExpected Length: %d Actual Length: %d\n", data.length, length);
				printf("Sending the below packet to client as rejection:\n\n");
				print_ackreject_packet_details(reject);
			}
			else if(data.end_packetID!= END_PACKET_ID)																		//condition to check end of packet missing
			{
				reject = CreateRejectPacket(data);
				reject.reject_subcode = END_OF_PACKET_ID_MISSING;
				sendto(sock, &reject, sizeof(struct reject_packet), 0, (struct sockaddr *)&from,fromlen);
				printf("\n\nEnd of packet ID is missing!\n");
				printf("Sending the below packet to client as rejection:\n\n");
				print_ackreject_packet_details(reject);
			}
			else if(!(data.segment_no==receivedPacketNo+1))																	//condition to check out of sequence packet
			{
				reject = CreateRejectPacket(data);
				reject.reject_subcode = OUT_OF_SEQUENCE_CODE;
				sendto(sock, &reject, sizeof(struct reject_packet), 0, (struct sockaddr *)&from,fromlen);
				printf("\n\nPacket is out of Sequence!\n");
				printf("Sending the below packet to client as rejection:\n\n");
				print_ackreject_packet_details(reject);
			}
			else																											//if all above conditions are false, acknowledgement is sent
			{
				ack = CreateAckPacket(data);
				sendto(sock,&ack,sizeof(struct reject_packet),0,(struct sockaddr *)&from,fromlen);
				printf("\n\nCorrect packet received!\n");
				printf("Sending the below packet to client as acknowledgement:\n\n");
				print_ackreject_packet_details(ack);
				receivedPacketNo++;
			}
		}
}
