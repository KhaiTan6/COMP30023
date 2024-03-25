#define _DEFAULT_SOURCE 
#define _POSIX_C_SOURCE 200112L
#include "rpc.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <endian.h>
#include <sys/select.h>

#define NOT_FOUND -1
#define MAX_LEN 8
#define FIND_CODE 0
#define CALL_CODE 1
#define CLOSE_CODE 2

//--- prototype decleration --- //
int check_rpcdata(rpc_data* pl);


struct rpc_server {
    int port; // port number
    int sockfd; //socket number
    rpc_handler *handlers; // array of functions/handlers
    int arrSize; // size of array
    char** names; // array of names
    int count; // index of current handler/name
    fd_set masterfds;
    int maxfd;
};


// server side API
rpc_server *rpc_init_server(int port) {
    int re, s;
    int sockfd;
    struct addrinfo hints, *res;
    
    // int to string
    char service[sizeof(int) * 4 + 1];
    sprintf(service, "%d", port);
    
    rpc_server* server = malloc(sizeof(rpc_server));
    if (server == NULL){
        fprintf(stderr,"server is NULL\n");
        return NULL;
    }
    // initial size decleration
    server->arrSize = 10;

    server->handlers = malloc(server->arrSize * sizeof(rpc_handler));
    if (server->handlers == NULL){
        fprintf(stderr,"malloc handlers fail\n");
    }
    server->names = malloc(server->arrSize * sizeof(char*));
    if (server->names == NULL){
        fprintf(stderr,"malloc names fail\n");
    }

    // create listening socket
	// Create address we're going to listen on (with given port number)
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    s = getaddrinfo(NULL, service, &hints, &res); // 0 if succeeds
    if(s != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    }

    // create socket
    sockfd = socket(res->ai_family, res->ai_socktype, res-> ai_protocol);
    if (sockfd < 0){
        perror("socket");
    }

    //Reuse port if possible
    re = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(int)) < 0){
        perror("setsockopt");
    }

    //bind
    if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0){
        perror("bind");
    }
    freeaddrinfo(res);

    //listen
    if (listen(sockfd, 5) < 0){
        perror("listen");
    }

    // initialise an active file descriptors set
	fd_set masterfds;
	FD_ZERO(&masterfds);
	FD_SET(sockfd, &masterfds);

    server->port = port;
    server->sockfd = sockfd;
    server->count = 0;
    server->masterfds = masterfds;
	// record the maximum socket number
    server->maxfd = sockfd;

    return server;
}

int rpc_register(rpc_server *srv, char *name, rpc_handler handler) {
    if (srv == NULL || name == NULL || handler == NULL){
        return -1;
    }
    
    if (srv->count == srv->arrSize){
        srv->arrSize *= 2;
        srv->handlers = realloc(srv->handlers, sizeof(rpc_handler) * srv->arrSize);
        srv->names = realloc(srv->names, sizeof(char*) * srv->arrSize);
        if (srv->handlers == NULL){
            fprintf(stderr,"realloc handlers failed\n");

        }
        if (srv->names == NULL){
            fprintf(stderr,"realloc names failed\n");

        }
    }

    // check array if exist same name
    for(int i = 0; i < srv->count; i++){
        // if same name, replace function
        if (strcmp(srv->names[i], name) == 0){ 
            srv->handlers[i] = handler; 
            return i;
        }
    }
    //just insert name and handler
    srv->handlers[srv->count] = handler; 

    srv->names[srv->count] = malloc(strlen(name)+1);
    strcpy(srv->names[srv->count], name);

    srv->count += 1; // increase count after adding handler

    return srv->count;
}

void rpc_serve_all(rpc_server *srv) {
    uint32_t n,nlen, index;
    uint32_t notfound = NOT_FOUND;

    if (srv == NULL){
        fprintf(stderr, "serve_all srv is NULL\n");
    }
    
    // rpc_data stuffs
    rpc_data *pl = malloc(sizeof(rpc_data));
    if (pl == NULL){
        fprintf(stderr, "server: pl is NULL\n");
    }
    memset(pl, 0, sizeof(rpc_data));


    while (1){
		// monitor file descriptors
		fd_set readfds = srv->masterfds;
		if (select(FD_SETSIZE, &readfds, NULL, NULL, NULL) < 0) {
			perror("select");
		}

		// loop all possible descriptor
		for (int curr = 0; curr <= srv->maxfd; ++curr) {
			// determine if the current file descriptor is active
			if (!FD_ISSET(curr, &readfds)) {
				continue;
			}

			// create new socket if there is new incoming connection request
			if (curr == srv->sockfd) {
				struct sockaddr_in cliaddr;
				socklen_t clilen = sizeof(cliaddr);
				int newsockfd = accept(srv->sockfd, (struct sockaddr*)&cliaddr, &clilen);
				if (newsockfd < 0) {
					perror("accept");
                }
				else {
					// add the socket to the set
					FD_SET(newsockfd, &(srv->masterfds));
					// update the maximum tracker
					if (newsockfd > srv->maxfd)
						srv->maxfd = newsockfd;
				}
			}
            else {
                // keep waiting for requests
                while (1) {
                    uint32_t code = NOT_FOUND;
                    n = read(curr, &code, sizeof(uint32_t));                        // read from client, request code
                    if (n < 0) {
                        fprintf(stderr, "server: error read request\n");
                    }
                    //change to host byte order
                    code = ntohl(code);

                    // find request
                    if (code == FIND_CODE) {
                        n = read(curr, &nlen, sizeof(uint32_t));                    // read from client, size of name
                        if (n < 0) {
                            fprintf(stderr, "server: error read size name\n");
                        }

                        //change to host byte order
                        nlen = ntohl(nlen);

                        // create name buffer
                        char name[nlen + 1];
                        memset(name,'\0',sizeof(name));

                        n = read(curr, name, sizeof(name));                         // read from client, name of function
                        if (n < 0) {
                            fprintf(stderr, "server: error read name\n");
                        }

                        // find function for given name
                        int found = 0;

                        for (int i = 0; i < srv->count; i++) {
                            // if match
                            if(strcmp(srv->names[i], name) == 0){
                                //change to network byte order
                                uint32_t indexs =  htonl(i);
                                n = write(curr, &indexs, sizeof(uint32_t));         // write to client, index of function
                                if (n < 0){
                                    fprintf(stderr, "server: error write index\n");
                                }
                                found = 1;
                                break;
                            }
                        }

                        if (found == 0) {
                            //change to network byte order
                            notfound = htonl(notfound);

                            n = write(curr, &notfound, sizeof(uint32_t));           // write to client, index not found
                        }                        
                    }
                    //call request
                    else if (code == CALL_CODE) {
                        n = read(curr, &index, sizeof(index));                      // read from client, index
                        if (n < 0) {
                            fprintf(stderr, "server: error read index\n");
                        }

                        //change to host byte order
                        index = ntohl(index);

                        uint64_t pldata1;
                        n = read(curr, &(pldata1), sizeof(uint64_t));               // read from client, payload
                        if (n < 0) {
                            fprintf(stderr, "server: error read data1\n");
                        }

                        //change to host byte order
                        pldata1 = be64toh(pldata1);
                        pl->data1 = (int) pldata1;

                        uint32_t pldata2_len;
                        n = read(curr, &(pldata2_len), sizeof(uint32_t));           // read from client, payload
                        if (n < 0) {
                            fprintf(stderr, "server: error read data2_len\n");
                        }

                        //change to host byte order
                        pldata2_len = ntohl(pldata2_len);
                        pl->data2_len = (uint32_t) pldata2_len;
                        
                        pl->data2 = NULL;

                        if (pl->data2_len > 0) {
                            //malloc after getting data2len
                            pl->data2 = malloc((pl->data2_len));
                            if (pl->data2 == NULL){
                                fprintf(stderr, "server: pl->data2 is NULL\n");
                            }

                            n = read(curr, pl->data2, pl->data2_len);                // read from client, payload
                            if (n < 0){
                                fprintf(stderr, "server: error read data2\n");
                            }
                        }

                        // process payload with function here
                        rpc_data* result = (*(srv->handlers[index]))(pl);

                        uint32_t invalid = 0;
                        if (check_rpcdata(result) == 0) {
                            invalid = htonl(invalid);
                            n = write(curr, &invalid , sizeof(uint32_t));            //send to client, result invalid
                            continue;
                        } 

                        invalid = htonl(1);
                        n = write(curr, &invalid , sizeof(uint32_t));                //send to client, result valid

                        
                        //change to network byte order
                        uint64_t resultdata1 = htobe64(result->data1);
                        n = write(curr, &(resultdata1), sizeof(uint64_t));           // write to client, results
                        if (n < 0) {
                            fprintf(stderr, "server: error write data1\n");
                        }

                        //change to network byte order
                        uint32_t resultdata2_len = htonl(result->data2_len);

                        n = write(curr, &(resultdata2_len), sizeof(uint32_t));        // write to client, results
                        if (n < 0) {
                            fprintf(stderr, "server: error write data2_len\n");
                        }

                        // if data2_len is greater than 0, send
                        if(result->data2_len > 0) {
                            n = write(curr, result->data2, result->data2_len);        // write to client, results
                            if (n < 0) {
                                fprintf(stderr, "server: error write data2\n");
                            }
                        }
                    }
                    // close request
                    else if (code == CLOSE_CODE){
                        close(curr); 
                    }
                    else { 
                        continue;
                    }
                }
            }
        }
    }
}

struct rpc_client {
    int addr;
    int port;
    int sockfd;
};

struct rpc_handle {
    uint32_t index;
};

// client side API
rpc_client *rpc_init_client(char *addr, int port) {
    int sockfd, s;
	struct addrinfo hints, *servinfo, *rp;

    rpc_client *client = malloc(sizeof(rpc_client));
    if (client == NULL) {
        fprintf(stderr,"client is NULL\n");
        return NULL;
    }

	// Create address
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;

    // int to string
    char service[sizeof(int) * 4 + 1];
    sprintf(service, "%d", port);

	// Get addrinfo of server. From man page:
	// The getaddrinfo() function combines the functionality provided by the
	// gethostbyname(3) and getservbyname(3) functions into a single interface
	s = getaddrinfo(addr, service, &hints, &servinfo);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
	}

	// Connect to first valid result
	// Why are there multiple results? see man page (search 'several reasons')
	// How to search? enter /, then text to search for, press n/N to navigate
	for (rp = servinfo; rp != NULL; rp = rp->ai_next) {
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockfd == -1)
			continue;

		if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break; // success

		close(sockfd);
	}

	if (rp == NULL) {
		fprintf(stderr, "client: failed to connect\n");
	}
	freeaddrinfo(servinfo);

    client->addr = atoi(addr); // change to string 
    client->port = port;
    client->sockfd = sockfd;
    return client;
}

// find is 0
rpc_handle *rpc_find(rpc_client *cl, char *name) {
    int n;
    uint32_t index;
    uint32_t findID = 0;
    uint32_t nlen = strlen(name);

    rpc_handle *handle = malloc(sizeof(rpc_handle));
    if (handle == NULL){
        fprintf(stderr,"client: handle is NULL\n");
        return NULL;
    }

    //change to network byte order
    findID = htonl(findID);

    n = write(cl->sockfd, &(findID), sizeof(uint32_t));                 // send to server, find request
        if (n < 0) {
        fprintf(stderr, "client: error write find request\n");
    }

    //change to network byte order
    nlen = htonl(nlen);

    n = write(cl->sockfd, &(nlen), sizeof(uint32_t));                   //send to server size of name of func u want
    if (n < 0) {
        fprintf(stderr, "client: error write name length\n");
    }

    //change back to host byte order
    nlen = ntohl(nlen);
    n = write(cl->sockfd, name, nlen+1);                                //send to server, name of func u want
    if (n < 0) {
        fprintf(stderr, "client: error write name\n");
    }


    n = read(cl->sockfd, &(index), sizeof(uint32_t));                   // read from server, get back info about that function
    if (n < 0) {
        fprintf(stderr,"client: error read handle\n");
    }
    
    //change to host byte order
    index = ntohl(index);
    if (index == -1) { // if index not found
        return NULL;
    }
    handle->index = (int) index;
    return handle;
}

// call is 1
rpc_data *rpc_call(rpc_client *cl, rpc_handle *h, rpc_data *payload) {
    int n;
    uint32_t callID = 1;

    if (check_rpcdata(payload) == 0) {
        return NULL;
    }

    rpc_data *result = malloc(sizeof(rpc_data));
    if (result == NULL) {
        fprintf(stderr, "client: result is NULL\n");
        return NULL;
    }
    memset(result, 0, sizeof(rpc_data));

    //change to network byte order
    callID = htonl(callID);

    n = write(cl->sockfd, &(callID), sizeof(uint32_t));                 // send to server, call request
    if (n < 0) {
        fprintf(stderr, "client: error write call request\n");
    }

    //change to network byte order
    uint32_t hindex = htonl(h->index);

    n = write(cl->sockfd, &(hindex), sizeof(uint32_t));                 // send to server, handle->index
    if (n < 0) {
        fprintf(stderr, "client: error write handle index\n");
    }

    //change to network byte order
    uint64_t payloaddata1 = htobe64(payload->data1);

    n = write(cl->sockfd, &(payloaddata1), sizeof(uint64_t));           //send to server, payload
    if (n < 0){
        fprintf(stderr, "client: error write data1\n");
    }

    //change to network byte order
    uint32_t payloaddata2_len = htonl(payload->data2_len);

    n = write(cl->sockfd, &(payloaddata2_len), sizeof(uint32_t));       //send to server, payload
    if (n < 0) {
        fprintf(stderr, "client: error write data2_len\n");
    }

    if(payload->data2_len > 0){
        n = write(cl->sockfd, payload->data2, payload->data2_len);      //send to server, payload
        if (n < 0) {
            fprintf(stderr, "client: error write data2\n");
        }
    }
    uint32_t invalid;
    n = read(cl->sockfd, &invalid, sizeof(uint32_t));                   // read form server, validity of results
    invalid = ntohl(invalid);
    if (invalid == 0) {
        return NULL;
    }

    uint64_t resultdata1;
    n = read(cl->sockfd, &(resultdata1), sizeof(uint64_t));             //read from server, results
    if (n < 0) {
        fprintf(stderr, "client: error read data1\n");
    }

    //change to host byte order
    resultdata1 = be64toh(resultdata1);
    result->data1 = (int) resultdata1;
    
    uint32_t resultdata2_len;
    n = read(cl->sockfd, &(resultdata2_len), sizeof(uint32_t));         //read from server, results
    if (n < 0) {
        fprintf(stderr, "client: error read data2_len\n");
    }
    
    //change to host byte order
    resultdata2_len = ntohl(resultdata2_len);
    result->data2_len = (uint32_t) resultdata2_len;

    result->data2 = NULL;

    if(result->data2_len > 0) {
        //malloc after getting data2len
        result->data2 = malloc((result->data2_len));
        if (result->data2 == NULL) {
            fprintf(stderr, "client: result->data2 is NULL\n");
            return NULL;
        }

        n = read(cl->sockfd, result->data2, result->data2_len);                 //read from server, results
        if (n < 0) {
            fprintf(stderr, "client: error read data2\n");
        }
    }
    return result;
}

void rpc_close_client(rpc_client *cl) {
    uint32_t close = CLOSE_CODE;

    //change to network byte order
    close = htonl(close);

    write(cl->sockfd, &(close), sizeof(uint32_t));
    free(cl);
}


// shared API
void rpc_data_free(rpc_data *data) {
    if (data == NULL) {
        return;
    }
    if (data->data2 != NULL) {
        free(data->data2);
    }
    free(data);
}

// checks validity of a rpc_data*.
int check_rpcdata(rpc_data* pl) {
    if (pl == NULL) {
        return 0;
    }
    if (sizeof(pl->data1) > MAX_LEN) {
        return 0;
    }
    if (pl->data2_len == 0) {
        if (pl->data2 != NULL) {
            return 0;
        }
    }
    if (pl->data2_len != 0) {
        if (pl->data2 == NULL) {
            return 0;
        }
    }
    return 1;
}