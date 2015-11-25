//Frank Eslami, CS344-400, Program 4
//otp_enc_d
//Run command: run_server 22776 &

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>

//LOG
//Server client communication works, including response. Use this as baseline.
//Client sends message, server responds, client closes, server waits for more connections
//Authenticate the right client connected
//Fork new process on authenticated clients
//Log server messags to log.txt
//Fork new process immediately after connection
//Validate client in forked process
//SOLVED - recv reads all client messages at once. Need to process individual messages
//Obtain plaintext from client and send to forked process
//Obtain key from client
//strip newline from plaintext
//encode plaintext to ciphertext
//return ciphertext to client
//>>>>>>> NEXT STEP >>>>>>>>

int main(int argc, char *argv[])
{
		int socket_desc, client_sock = 0, c, read_size;
		struct sockaddr_in server, client;
		char client_message[2000];
		int valid_client = 0;
		pid_t cpid;

        //Track program status in log
        umask(0);       //unmask the file mode
        FILE *fp_log = NULL;
        fp_log = fopen("log_otp_enc_d.txt", "w+");
        if(fp_log == NULL) {
                perror("Server: failed to create log.txt file");
                return 1;
        }
        fprintf(fp_log, "\nServer: log.txt created\n");
        fflush(fp_log);

		//Port
		int port_num = atoi(argv[1]);	
		fprintf(fp_log, "Server: port num = %d\n", port_num);
        fflush(fp_log);

        /********* CREATE DAEMON *********/
        //Fork process
        pid_t pid = fork();
        if (pid < 0) {
                perror("Server: daemon fork failed");
                return 1;
        }
        //Parent
        else if (pid > 0) {
                //Kill parent process. Child belongs to the init process
                exit(0);
        }

        //Set new session
        pid_t sid = setsid();
        if (sid < 0) {
                perror("Server: daemon set new session failed");
                return 1;
        }
        fprintf(fp_log, "Server: daemon session created\n");
        fflush(fp_log);

        //Close file descriptors for daemon
		/*
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
		*/
        /********* FINISHED CREATING DAEMON *********/

		//Create socket
		socket_desc = socket(AF_INET, SOCK_STREAM, 0);
		if (socket_desc == -1)
		{
				fprintf(fp_log, "Server: could not create socket");
				fflush(fp_log);
				return 1;
		}
		fprintf(fp_log, "Server: socket created\n");
		fflush(fp_log);

		//Prepare the sockaddr_in structure
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = htons(port_num);

		//Bind socket
		if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
		{
				fprintf(fp_log, "Server: bind failed");
				fflush(fp_log);
				return 1;
		}
		fprintf(fp_log, "Server: bind done\n");
        fflush(fp_log);

        //Run daemon until -KILL signal
        fprintf(fp_log, "Server: daemon server running until -KILL signal received\n");
        fflush(fp_log);
        while(1) {
				sleep(1);

				//Listen
				listen(socket_desc, 5);

				//Waiting for incoming connection
				fprintf(fp_log, "Server: waiting for incoming connection\n");
				fflush(fp_log);
				c = sizeof(struct sockaddr_in);

				//Accept Incoming connection
				client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
				if (client_sock < 0)
				{
						fprintf(fp_log, "Server: client accept failed\n");
						fflush(fp_log);
						close(client_sock);
						continue;
				}
				else if (client_sock > 0) {
						fprintf(fp_log, "Server: connected to client\n");
						fflush(fp_log);
				}

				//Fork new process
				cpid = fork();	
				if (cpid < 0) {		//fork failed
						fprintf(fp_log, "Server: fork failed\n");
						fflush(fp_log);
				}					
				else if (cpid > 0) {	//parent
						close(client_sock);	
						wait(NULL);	
				}
							
				/****** CHILD PROCESS ******/	
				fprintf(fp_log, "Child: in child process\n");
				fflush(fp_log);

				//Receive authentication message
				char server_message[20] = "success";
				read_size = 0;
				memset(client_message, 0, sizeof(client_message));
				while (1) {

						read_size = recv(client_sock, client_message, sizeof(client_message), 0);
						fprintf(fp_log, "Child: received authentication message = %s\n", client_message);
						fflush(fp_log);
						write(client_sock, server_message, strlen(server_message));
						break;
				}	

				if (read_size == 0)
				{
						fprintf(fp_log, "Child: client closed during authentication\n");
						fflush(fp_log);
				}	
				else if (read_size == -1)
				{
						fprintf(fp_log, "Child: recv failed during authentication\n");	
						fflush(fp_log);
						close(client_sock);
						continue;
				}

				//Validate client is authentic and fork process
				if (strcmp(client_message, "prog4_connection") == 0) {
						fprintf(fp_log, "Child: client is authentic\n");
						fflush(fp_log);
						valid_client = 1;
				}
				else {
						fprintf(fp_log, "Child: client is not authentic. Closing connection\n");
						fflush(fp_log);
						close(client_sock);	
						continue;
				}
			
				//Receive plaintext size 
				read_size = 0;
                memset(client_message, 0, sizeof(client_message));
                while (1) {
                        read_size = recv(client_sock, client_message, sizeof(client_message), 0);
                        fprintf(fp_log, "Child: received plaintext size = %s\n", client_message);
                        fflush(fp_log);
                        write(client_sock, server_message, strlen(server_message));
                        break;
                }

				if (read_size == 0)
				{
						fprintf(fp_log, "Child: client closed during plaintext filesize\n");
						fflush(fp_log);
				}	
				else if (read_size == -1)
				{
						fprintf(fp_log, "Child: recv failed during plaintext filesize\n");	
						fflush(fp_log);
						close(client_sock);
						continue;
				}

				//Receive plaintext
				read_size = 0;
				int size = atoi(client_message);
				char plaintext[size];
				fprintf(fp_log, "Child: plaintext int file size = %d\n", size);	
				fflush(fp_log);

                while (1) {
                        read_size = recv(client_sock, plaintext, sizeof(plaintext), 0);
                        fprintf(fp_log, "Child: received plaintext = %s\n", plaintext);
                        fflush(fp_log);
                        write(client_sock, server_message, strlen(server_message));
                        break;
                }

                if (read_size == 0)
                {
                        fprintf(fp_log, "Child: client closed during plaintext\n");
                        fflush(fp_log);
                }
                else if (read_size == -1)
                {
                        fprintf(fp_log, "Child: recv failed during plaintext\n");
                        fflush(fp_log);
                        close(client_sock);
						continue;
                }
	
                //Receive mykey size
                read_size = 0;
                memset(client_message, 0, sizeof(client_message));
                while (1) {
                        read_size = recv(client_sock, client_message, sizeof(client_message), 0);
                        fprintf(fp_log, "Child: received mykey = %s\n", client_message);
                        fflush(fp_log);
                        write(client_sock, server_message, strlen(server_message));
                        break;
                }

                if (read_size == 0)
                {
                        fprintf(fp_log, "Child: client closed during mykey filesize\n");
                        fflush(fp_log);
                }
                else if (read_size == -1)
                {
                        fprintf(fp_log, "Child: recv failed during mykey filesize\n");
                        fflush(fp_log);
                        close(client_sock);
						continue;
                }

                //Receive mykey
                read_size = 0;
                size = atoi(client_message);
                char mykey[size];
				memset(mykey, 0, sizeof(mykey));
                fprintf(fp_log, "Child: mykey int file size = %d\n", size);
                fflush(fp_log);

                while (1) {
                        read_size = recv(client_sock, mykey, sizeof(mykey), 0);
                        fprintf(fp_log, "Child: received mykey = %s\n", mykey);
                        fflush(fp_log);
                        write(client_sock, server_message, strlen(server_message));
                        break;
                }

                if (read_size == 0)
                {
                        fprintf(fp_log, "Child: client closed during mykey\n");
                        fflush(fp_log);
                }
                else if (read_size == -1)
                {
                        fprintf(fp_log, "Child: recv failed during mykey\n");
                        fflush(fp_log);
                        close(client_sock);
						continue;
                }

				//Strip newline from plaintext
				int i;
				for (i = 0; i < sizeof(plaintext); i++) {
						if (plaintext[i] == '\n') {
								plaintext[i] = '\0';
						}
				} 

				/****** ENCODE PLAINTEXT TO CIPHERTEXT ******/
				//uncomment printf from line 299 to 378 for debugging
				//printf("\n------------ PLAINTEXT TO CIPHERTEXT --------------\n");
        		char alpha_vals[] = "a bcdefghijklmnopqrstuvwxyz";
        		//printf("plaintext = %s\n", plaintext);
        		//printf("mykey = %s\n", mykey);
        		//printf("alpha_vals = %s\n", alpha_vals);

        		//Assign int values to plaintext
        		int plaintext_int[strlen(plaintext)];
        		int  j;
        		int cipher_size = sizeof(plaintext_int) / sizeof(int);

        		for (i = 0; i < strlen(plaintext); i++) {
        		        for (j = 0; j < strlen(alpha_vals); j++) {
        		                if (tolower(plaintext[i]) == alpha_vals[j]) {
        		                        plaintext_int[i] = j;
        		                }
        		        }
        		}
        		//Print plaintext_int
        		//printf("plaintext_int = ");
        		for (i = 0; i < cipher_size; i++) {
        		        //printf("%d ", plaintext_int[i]);
        		}
        		//printf("\n");

        		//Assign int values to key
        		int mykey_int[strlen(mykey)];
        		for (i = 0; i < strlen(mykey); i++) {
        		        for (j = 0; j < strlen(alpha_vals); j++) {
        		                if (mykey[i] == alpha_vals[j]) {
        		                        mykey_int[i] = j;
        		                }
        		        }
        		}
        		//Print mykey_int
        		//printf("mykey_int = ");
        		for (i = 0; i < cipher_size; i++) {
        		        //printf("%d ", mykey_int[i]);
        		}
        		//printf("\n");

        		//Add plaintext_int and mykey_int values
        		int cipher_sum[cipher_size];
        		for (i = 0; i < cipher_size; i++) {
        		        cipher_sum[i] = plaintext_int[i] + mykey_int[i];
        		}
        		//Print sum
        		//printf("cipher_sum = ");
        		for (i = 0; i < cipher_size; i++) {
        		        //printf("%d ", cipher_sum[i]);
        		}
        		//printf("\n");

        		//Mod 26 cipher_sum
        		int cipher_mod[cipher_size];
        		for (i = 0; i < cipher_size; i++) {
        		        cipher_mod[i] = (cipher_sum[i] % 26);
        		}
        		//Print mod
        		//printf("cipher_mod = ");
        		for (i = 0; i < cipher_size; i++) {
        		        //printf("%d ", cipher_mod[i]);
        		}
        		//printf("\n");

        		//Convert cipher_mod to ciphertext
        		char ciphertext[cipher_size];
        		for (i = 0; i < cipher_size; i++) {
        		        for (j = 0; j < sizeof(alpha_vals); j++) {
        		                if (cipher_mod[i] == j) {
        		                        ciphertext[i] = alpha_vals[j];
        		                }
        		        }
        		}
        		//Print ciphertext
        		//printf("ciphertext = ");
        		for (i = 0; i < cipher_size; i++) {
        		        //printf("%c ", ciphertext[i]);
        		}
        		//printf("\n");

                //Send ciphertext to client
                read_size = 0;
                memset(client_message, 0, sizeof(client_message));
                while (1) {
                        read_size = recv(client_sock, client_message, sizeof(client_message), 0);
                        fprintf(fp_log, "Child: received authentication message = %s\n", client_message);
                        fflush(fp_log);
                        write(client_sock, ciphertext, strlen(ciphertext));
                        fprintf(fp_log, "Child: message sent to client\n");
                        fflush(fp_log);
                        break;
                }

                if (read_size == 0)
                {
                        fprintf(fp_log, "Child: client closed during authentication\n");
                        fflush(fp_log);
                }
                else if (read_size == -1)
                {
                        fprintf(fp_log, "Child: recv failed during authentication\n");
                        fflush(fp_log);
                        close(client_sock);
						continue;
                }

								
		} //end daemon while loop
		return 0; //end program
} //main







