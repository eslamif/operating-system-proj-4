//Frank Eslami, CS344-400, Program 4
//otp_enc
//Run command: run_client plaintext1 227766

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>


//LOG
//Server client communication works, including response. Use this as baseline.
//Change infinite read loop to one message.
//Client sends message, server responds, client closes, server waits for more connections
//Send authentication message to server
//Put send() to function
//Send plaintext to server
//Send key to server
//Validate key length = plaintext length
//Receive ciphertext from server
//Output cipher text to stdout
//comment out all printf's except last
//Add newline and save to file
//client runs in 3 ways specified in instructions
//>>>>>>> NEXT STEP >>>>>>>>

//Function declarations
int sendMessage(char client_message[], char server_message[], int *m_len_sent, int *m_len_rec, int *sock, int *read_size); 

int main(int argc, char *argv[]) 
{
		int sock;
		struct sockaddr_in server;

		//Port
		int port_num = atoi(argv[3]);
		//printf("Client: port_num = %d\n", port_num);

		//Create socket
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == -1)
		{
				printf("Client: could not create socket\n");
				return 1;
		}
		//printf("Client: socket created\n");

	    //Setup host address
	    struct hostent *server_ip_address = gethostbyname("eos-class.engr.oregonstate.edu");
	    if(server_ip_address == NULL) {
		        printf("Client: could not resolve server hot name\n");
				return 1;
	    }
	
	    server.sin_family = AF_INET;
	    server.sin_port = htons(port_num);
	    memcpy(&server.sin_addr, server_ip_address->h_addr, server_ip_address->h_length);

		//Connect to remote server
		if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
		{
				perror("Client: connect failed\n");
				return 1;
		}	
		//printf("Client: connection successful\n");

		//Send authentication message to server
		int m_len_sent, m_len_rec;
		char client_message[1000], server_message[512];
		strcpy(client_message, "prog4_connection");
		int read_size = 2000;
		sendMessage(client_message, server_message, &m_len_sent, &m_len_rec, &sock, &read_size);


		//Open plaintext file specified by user's input
        FILE *fp_pt = fopen(argv[1], "r");
        if(fp_pt == NULL) {
                perror("Client: unable to open plaintext file");
                return 1;
        }

        //Get file size of plaintext
        struct stat st;
        stat(argv[1], &st);
        int plaintext_size = st.st_size;
        //printf("Client: file_size of plaintext = %d\n", plaintext_size);

        //Store plaintext file into array
        char plaintext[plaintext_size];
        fgets(plaintext, plaintext_size, fp_pt);
        //printf("Client: plaintext = %s\n", plaintext);

        //Send plaintext size to server
        char filesize_char[512];
        sprintf(filesize_char, "%d", plaintext_size);
		m_len_sent = 0; m_len_rec = 0;
		read_size = 2000;
		sendMessage(filesize_char, server_message, &m_len_sent, &m_len_rec, &sock, &read_size);
	
		//Send plaintext to server
		m_len_sent = 0; m_len_rec = 0;
		read_size = 2000;
		sendMessage(plaintext, server_message, &m_len_sent, &m_len_rec, &sock, &read_size);

        //Open mykey file specified by user's input
        FILE *fp_key = fopen(argv[2], "r");
        if(fp_key == NULL) {
                printf("Client: unable to open plaintext file\n");
                return 1;
        }

        //Get file size of mykey
        stat(argv[2], &st);
        int mykey_size = st.st_size;
        //printf("Client: file_size of mykey  = %d\n", mykey_size);
        //printf("Client: file_size of plaintext  = %d\n", plaintext_size);

		//Validate mykey = plaintext length
		if (plaintext_size != mykey_size) {
				errno = EMSGSIZE;
				perror("Error: key's length does not match plaintext");
				return 1;
		}
		else {
				//printf("key '%s' matches '%s' length\n", argv[2], argv[1]);
		}

        //Store mykey file into array
        char mykey[mykey_size];
		memset(mykey, 0, sizeof(mykey));
        fgets(mykey, mykey_size, fp_key);
        //printf("Client: mykey  = %s\n", mykey);

        //Send mykey size to server
        memset(filesize_char, 0, sizeof(filesize_char));
        sprintf(filesize_char, "%d", mykey_size);
        m_len_sent = 0; m_len_rec = 0;
		read_size = 2000;
        sendMessage(filesize_char, server_message, &m_len_sent, &m_len_rec, &sock, &read_size);

        //Send mykey to server
        m_len_sent = 0; m_len_rec = 0;
		read_size = 2000;
        sendMessage(mykey, server_message, &m_len_sent, &m_len_rec, &sock, &read_size);

        //Receive cipher text from server
        m_len_sent = 0; m_len_rec = 0;
		char request_cipher[] = "client requests ciphertext";	
		
		//Send cipher request to server
		while (1) {
				m_len_sent = send(sock, request_cipher, strlen(request_cipher), 0);
				if (m_len_sent < 0)
				{
						printf("Client: send failed\n");
						break;
				}
				//Receive message
				if ((m_len_rec = recv(sock, server_message, sizeof(mykey), 0)) < 0)
				{
						printf("Client: recv failed\n");
						break;		
				}
				break;
		} 
		//printf("Client: server reply = %s\n", server_message);

		//Add newline to ciphertext
		char ciphertext[strlen(server_message) + 1];
		strcpy(ciphertext, server_message);
		ciphertext[strlen(ciphertext)] = '\n';
		//Add new line to cipher text if not already present
		//printf("len = %d, size = %d\n", strlen(server_message), sizeof(server_message));
		//printf("len = %d, size = %d\n", strlen(ciphertext), sizeof(ciphertext));

		printf("%s", ciphertext);		

		fclose(fp_key);
		fclose(fp_pt);
		close(sock);
		return 0;
}

/***** FUNCTION DEFINITIONS *****/
//Send authentication message to server
int sendMessage(char client_message[], char server_message[], int *m_len_sent, int *m_len_rec, int *sock, int *read_size) {
		//printf("Client: about to send message = %s\n", client_message);
		while (1) {
				//Send message to server
				*m_len_sent = send(*sock, client_message, strlen(client_message), 0);
				if (m_len_sent < 0)
				{
						printf("Client: send failed\n");
						break;
				}

				//Receive message
				if ((*m_len_rec = recv(*sock, server_message, *read_size, 0)) < 0)
				{
						printf("Client: recv failed\n");
						break;		
				}

				else if (strcmp(server_message, "success") == 0){
						break;
				}
		} 
		//printf("Client: server reply = %s\n", server_message);

		//If server did not respond with authentication, kill program
		if (strcmp(server_message, "success") == 0){
				//printf("Client: server authenticated client.\n");
		}
		else {
				printf("Client: server did not authenticate client. Killing client.\n");
				return 1;	
		}
		return 0;
}
