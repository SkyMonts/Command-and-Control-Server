#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <ctime>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <chrono>



using namespace std;

#define MAXPORT 6
#define MAXBUF 1024
#define MAXACTION 10


fstream fOut;
vector<string> ipList;
vector<time_t> duration;
char msgAlreadyMem[] = "\"t$ALREADY MEMBER\"";
char msgOk[] = "\"$OK\"";
char msgNotMem[] = "\"$NOT MEMBER\"";

string getTime();
void serverRequest(int socketFd, string ip);

int main (int argc, char* argv[]){


char cPort[MAXPORT];
struct sockaddr_in name;
int sd, nPort, newSocket;
char str[INET_ADDRSTRLEN];
string choice;

memset(cPort, 0, MAXPORT);
sprintf(cPort, "%s", argv[1]);
nPort = atoi(cPort);


if ((sd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf (stderr, "ERROR: socket() failed\n");
    exit (-1);
 }
 fprintf(stdout,"Socket Created\n");

 name.sin_family = AF_INET; 
 name.sin_port = htons(nPort);
 name.sin_addr.s_addr = htonl(INADDR_ANY); 
 int addrlen = sizeof(name);



 if(bind(sd, (struct sockaddr *)&name, sizeof(name)) < 0){
 	fprintf (stderr, "ERROR: bind() failed\n");
    exit (-1);
 }
 fprintf(stdout,"Bind Created\n");

 if(listen(sd, 10) < 0){
 	fprintf (stderr, "ERROR: listen() failed\n");
    exit (-1);
 }
 fprintf(stdout,"listening on port %d.......\n", nPort);

 while(1){
 	if((newSocket = accept(sd, (struct sockaddr *)&name, (socklen_t*)&addrlen)) < 0){
 		fprintf (stderr, "ERROR: accept() failed\n");
    	exit (-1);
 	}
 	fprintf(stdout,"Connection Created\n");

 	inet_ntop(AF_INET, &(name.sin_addr), str, INET_ADDRSTRLEN);
 	serverRequest(newSocket, str);
 	bzero(str, INET_ADDRSTRLEN);
 	close(newSocket);
 	
 }
 	//remove("log.txt");

	return 0;
}



string getTime(){
	
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer,80,"%I:%M:%S",timeinfo);
    return string(buffer);
    
    /*using namespace std::chrono;
    auto now = time_point_cast<milliseconds>(system_clock::now());
    return date::format("%T", now);
    */
}

void serverRequest(int socketFd, string ip){
	char cmdBuffer[MAXACTION];
	char logBuffer[MAXBUF];
	char listBuffer[MAXBUF];
	memset(cmdBuffer,0,MAXACTION);
	read(socketFd, cmdBuffer, sizeof(cmdBuffer));
	int bytesWritten;

	if(strcmp(cmdBuffer, "#JOIN") == 0){
		fOut.open("log.txt", ios_base::app | ios::out);
		fOut << "\n\"" << getTime() << "\" Received a \"#JOIN\" action from agent \"" << ip << "\"";
		if(find(ipList.begin(), ipList.end(), ip) != ipList.end()){
			if(write(socketFd, msgAlreadyMem, sizeof(msgAlreadyMem)) < 0){
				fprintf(stderr, "Write Failed\n");
			}
			fOut << "\n\"" << getTime() << "\" Responded to agent " << ip << " with \"$ALREADY MEMBER\"";
		}
		else{
			if(write(socketFd, msgOk, sizeof(msgOk)) < 0){
				fprintf(stderr, "Write Failed\n");
			}
	
			duration.push_back(time(0));
			ipList.push_back(ip);
			fOut << "\n\"" << getTime() << "\" Responded to agent " << ip << " with \"$OK\"";
			
		}
		fOut.close();
	
	}
	else if(strcmp(cmdBuffer, "#LEAVE") == 0){
		fOut.open("log.txt",ios_base::app | ios::out);
		fOut << "\n\"" << getTime() << "\" Received a \"#LEAVE\" action from agent " << ip << "";
		vector<string>::iterator it;
		vector<clock_t>::iterator it_t;
		it = find(ipList.begin(), ipList.end(), ip);
		if(it != ipList.end()){
			int dis = distance(ipList.begin(), it);
			ipList.erase(it);
			duration.erase(duration.begin() + dis);
			if(write(socketFd, msgOk, sizeof(msgOk)) < 0){
				fprintf(stderr, "Write Failed\n");
			}
			fOut << "\n\"" << getTime() << "\" Responded to agent \"" << ip << "\" with \"$OK\"";
			
		}
		else{
			if(write(socketFd, msgNotMem, sizeof(msgNotMem)) < 0){
				fprintf(stderr, "Write Failed\n");
			}
			fOut << "\n" << getTime() << " Responded to agent \"" << ip << "\" with \"$NOT MEMBER\"";
		}
		fOut.close();

	}
	else if(strcmp(cmdBuffer, "#LIST") == 0){
		fOut.open("log.txt", ios_base::app | ios::out);
		fOut << "\n\"" << getTime() << "\" Received a \"#LIST\" action from agent "
				 << "\"" << ip << "\"";
		if(find(ipList.begin(), ipList.end(), ip) != ipList.end()){
			string str;
			for(int i = 0; i < ipList.size(); i++){
				str = "<";
				str = str + ipList[i] + ", ";
				str = str + to_string(time(0) - duration[i]) + " seconds>";
				strcpy(listBuffer, str.c_str());
				if((write(socketFd, listBuffer, strlen(listBuffer)) < 0)){
					fprintf(stderr, "Write Failed\n");
				}
				bzero(listBuffer, sizeof(listBuffer));
			}
			fOut << "\n\"" << getTime() << "\" Responded to agent \"" << ip << "\" with \"$OK\"";
		}
		else{
			fOut << "\n\"" << getTime() << "\" No response is supplied to agent "
				 << "\"" << ip << "\" (agent not active)";
		}
		fOut.close();
	}
	else if(strcmp(cmdBuffer, "#LOG") == 0){
		fOut.open("log.txt",ios_base::app | ios::out);
		fOut << "\n\"" << getTime() << "\" Received a \"#LOG\" action from agent "
				 << "\"" << ip << "\"";
		fOut.close();
		
		if(find(ipList.begin(), ipList.end(), ip) != ipList.end()){
			fOut.open("log.txt",ios_base::app | ios::out);
			fOut << "\n\"" << getTime() << "\" Responded to agent " << ip << " with \"$OK\"";
			fOut.close();
			string s;
			int fd = open("log.txt", O_RDWR);
			int bytesRead;
			bytesWritten = 0;
			do{
				memset(logBuffer,0,MAXBUF);
				bytesRead = read(fd, logBuffer, sizeof(logBuffer));
				if(bytesRead < 0){
					break;
				}
				write(socketFd, logBuffer, bytesRead);
				bytesWritten += bytesRead;
			}
			while(bytesRead > 0);
			cout << "Total Bytes Written = " << bytesWritten << "\n";
			close(fd);
			
		}
		else{
			fOut.open("log.txt",ios_base::app | ios::out);
			fOut << "\n\"" << getTime() << "\" No response is supplied to agent "
				 << "\"" << ip << "\" (agent not active)";
			fOut.close();
		}
	}
	//bzero(cmdbuff, sizeof(cmdbuff));


}
