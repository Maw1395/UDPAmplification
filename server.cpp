#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include<sys/socket.h>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <map>


using namespace std;
using namespace boost;

int serv_sockfd, cli_sockfd;
const unsigned MAXBUFLEN = 512;
pthread_mutex_t accept_lock = PTHREAD_MUTEX_INITIALIZER;
map<string, string> usersMap;
map <string, vector<string>> friendsMap;
map <string, int> loggedInUsers;
map <string, int> userSockets;
map <string, vector<string> > requestMap;
map <string, vector <string> > PortMap;
ofstream outInfo;
ifstream userInfo, configFile;

void *client_function(void *junk);
int login_function( int sock, string username, string password);
int register_function(int sock, string username, string password);
int messenger_function(int sock, string username);
void logout_function();
void intHandler(int dummy) {
    logout_function();
}

int main(int argc, char *argv[])
{
  /********
  ARGUMENT COUNTING
  ********/
  if(argc != 3)
  {
    cout << "usage: messenger_server user_info_file configration_file\n";
    exit(1);
  }

  userInfo.open(argv[1]);


  /***********
  PARSING THE INPUT FILE NAMES FOR CLARIFICATION
  ***********/
  if(!userInfo)
  {
  	cerr<<"File "<< argv[1] << " does not exist\n";
  	exit(1);
  }
  configFile.open(argv[2]);
  if (!configFile)
  {
  	cerr<<"File "<<argv[2]<<" does not exist\n";
  	exit(1);
  }
  typedef boost::char_separator<char> separator_type;
  /***********
  INSERTING USERS INTO A USER MAP WITH USERNAMES AND PASSWORDS
  INSERTING USERS INTO A FRIEND MAP WITH USERNAMES AND FREINDS
  *******/

  string line;
  while(getline(userInfo, line))
  {
  	tokenizer<> tok(line);
  	string username, password;
  	bool userb, passwordb;
  	vector<string> friends;
  	userb = false;
  	passwordb=false;
  	for(tokenizer<>::iterator beg=tok.begin(); beg!=tok.end();++beg)
  	{
  		if(!userb)
  		{
  			username=*beg; userb=true;
  		}
  		else if(! passwordb)
  		{
  			password=*beg; passwordb=true;
  		}
  		else
  		{
  			friends.push_back(*beg);
  		}
  		if(userb && passwordb)
  		{
  			usersMap.insert(pair <string, string>(username, password));
  			//std::cout << username<< " password " << usersMap.find(username)->second << '\n';
  		}
  	}
  	friendsMap.insert(pair <string, vector<string>>(username, friends));
  }
  outInfo.open(argv[1]);

  /**********
  SETTING UP THE PORTS AND CONNECTIONS
  *********/

  int port;
  while(getline(configFile, line))
  {
  	tokenizer<> tok(line);
  	for(tokenizer<>::iterator beg=tok.begin(); beg!=tok.end();++beg)
  	{
  		  ++beg;
  		  port = stoi(*beg);
  	}

   }   
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t sock_len;
	pthread_t tid;
	int* tid_ptr;
	ssize_t n;
		//cout << "port = " << port << endl;
	serv_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero((void*)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	bind(serv_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));


  /**********
  SERVER GENERAL INFORMATION
  AND PTHREAD CREATION
  ********/
   char hostname[512];
   gethostname(hostname, 512); 
   cout << "Hostname of server: " << hostname << endl;
   cerr << "Port Number of Server: " << port << endl;
   if(listen(serv_sockfd, 5)<0)
   {
   	cerr<<"Could not establish listen\n";
   	exit(1);
   }

   int connections=0;
   listen(serv_sockfd, 5);
   	for (connections; connections < 5; ++connections) {
		tid_ptr = (int *)malloc(sizeof(int));
        *tid_ptr = connections;
        pthread_create(&tid, NULL, &client_function, (void *)tid_ptr);
	}
	for (;;) 
  signal(SIGINT, intHandler);
	pause();
	userInfo.close();
	configFile.close();
}



void *client_function(void *arg)
{
	int cli_sockfd;
  struct sockaddr_in cli_addr;
  socklen_t sock_len;
  ssize_t n;
  char buf[MAXBUFLEN];

  int tid = *((int *)arg);
  free(arg);

  for (; ;) 
  {
    /*******
    Setting up client sockets upon request
    *******/
  	sock_len = sizeof(cli_addr);
  	pthread_mutex_lock(&accept_lock);
  	cli_sockfd = accept(serv_sockfd, (struct sockaddr *)&cli_addr, &sock_len);
  	pthread_mutex_unlock(&accept_lock);

    /*********
    GENERAL CLIENT MENU
    *******/
    cout << "client connected" << endl;
  	while ((n = read(cli_sockfd, buf, MAXBUFLEN)) > 0) 
    {
        signal(SIGINT, intHandler);
  		  buf[n] = '\0';
  	   	vector<string> parse;
        boost::split(parse, buf, boost::is_any_of(" "));
  	    
  	    if(parse[0] =="l")
  	    {
  	    	if (login_function(cli_sockfd, parse[1], parse[2])==200)
          {
            loggedInUsers.insert(pair <string, int>(parse[1], 0));
            userSockets.insert(pair <string, int>(parse[1], cli_sockfd));
            messenger_function(cli_sockfd, parse[1]);
          }
  	    }
  	    else if(parse[0] == "r")
  	    {
  	    	register_function(cli_sockfd, parse[1], parse[2]);
  	    }
  	    else if(parse[0]== "exit")
  	    {
  	    	close(cli_sockfd);
  	    }
  	}
  	if (n == 0) {
  	    cout << "client closed" << endl;
  	} 
  	else {
  	    cout << "something wrong" << endl;
  	}
  	close(cli_sockfd);
	
  }
}



int login_function (int sock, string username, string password)
{

	char buf[MAXBUFLEN];
	int code;
	string buf2, status;
	string b2=buf;
  if (usersMap.find(username)!=usersMap.end())
	{
		if(usersMap.find(username)->second == password)
		{
      if(loggedInUsers.find(username)==loggedInUsers.end())
      {
			status="200";
			code=200;
      }
      else 
      {
        status="500";
         code=500;
      }
		}
		else
		{
		status="500";
		code=500;
		}
	}
	else
	{
		status="500";
		code=500;
	}
  	write(sock, status.c_str(), status.length()+1);
  	//cout<<username<<password<<'\n';
  	//cout<<status<<'\n';
  	//cout<<code<<'\n';
  	return code;
}



int register_function(int sock, string username, string password)
{
	string buf2;
	if (usersMap.find(username)!=usersMap.end())
	{	
		buf2="500";
		write(sock, buf2.c_str(), buf2.length()+1);
    //cout<<"Registration Failed"<<endl;
    cout<<"500"<<endl;
		return 500;
	}
	
	usersMap.insert(pair <string, string>(username, password));
  vector <string> noFriends;
  friendsMap.insert(pair<string, vector <string> >(username, noFriends));
	buf2="200";
	write(sock, buf2.c_str(), buf2.length()+1);
	//cout<<username<<password<<'\n';
  cout<<"200"<<endl;
  	return 200;
}
int messenger_function(int sock, string username)
{
  char buf[MAXBUFLEN];
  //cout<<"SOCKET"<<sock<<endl;
  while (read(sock, buf, MAXBUFLEN))
  {
    signal(SIGINT, intHandler);
    vector<string> parse;
    boost::split(parse, buf, boost::is_any_of(" "));
    /***********
    MESSAGE FUNCTION
    *********/
    if (parse[0]== "m") 
    {
      string message=" ";
      //"Press 'm' send a message to another friend. format: m friend_username whatever_message\n";
      if (PortMap.find(parse[1])!=PortMap.end())
      {
        bool friendCheck=false;
        for (auto it=friendsMap.find(username)->second.begin(); it!= friendsMap.find(username)->second.end(); ++it)
        {
          if (*it==parse[1])
          {
            friendCheck=true;
            break;
          }
        }
        if(!friendCheck)
        {
          string message="ERROR";
          cerr<< "Cannot establish a connection between 2 users who are not friends"<<endl;
          write(sock, message.c_str(), message.length()+1);
        }
        else
        {
          message=PortMap.find(parse[1])->second[0] + " " + PortMap.find(parse[1])->second[1];
          write(sock, message.c_str(), message.length()+1);
        }

      }
      else
      {
        message="ERROR";
        cerr<<"Cannot find a port for user "<<parse[1];
        write(sock, message.c_str(), message.length()+1);
      }
    }
    /*********
    INVITE FUNCTION
    *********/
    else if (parse[0]== "i")  
    {
      //"Press 'i' invite a friend. format: i potential_friend_username [whatever_message]\n";
      int socketToSendTo=0;
      if (userSockets.find(parse[1])!=userSockets.end() && parse[1] != username)
      {
        bool friendCheck = false;
        for (auto it=friendsMap.find(username)->second.begin(); it!= friendsMap.find(username)->second.end(); ++it)
        {
          if (*it==parse[1])
          {
            friendCheck=true;
            break;
          }
        }
        if(!friendCheck) // they're not already friends
        {
          socketToSendTo=userSockets.find(parse[1])->second;
          string message="";
          if (parse.size()>2)
          {
            for (int i =2; i< parse.size(); i++)
            {
              message+=parse[i] + " ";
            }
          }

          message = "You have an invite request from " + username + ": " + message;
          //cout<< message;
          if (requestMap.find(username)== requestMap.end())
          {
            vector <string> requests;
            requests.push_back(parse[1]);

            requestMap.insert(pair <string, vector <string > > (username, requests));
          }
          else
          {
            requestMap.find(username)->second.push_back(parse[1]);
          }
          //request maps work in this way
          //user who iniates request 
          //vector of users who they have requested
          write(socketToSendTo, message.c_str(), message.length()+1);
          message = "Sent request from user: " +parse[1];
          write(sock, message.c_str(), message.length()+1);
      }
      else // they're already friends
      {
        string message="You're already friends with user: " + parse[1];
        write(sock, message.c_str(), message.length()+1);
      }
     }
      else if (parse[1] == username)
      {
        string message= "You cannot send a request to yourself";
        write(sock, message.c_str(), message.length()+1);
      }
      else
      {
        string message= "User " + parse[1] + " Not online";
        write(sock, message.c_str(), message.length()+1);
      }

    }

    /*********
    INVITE ACCEPT FUNCTION
    *********/
    else if (parse[0]== "ia") 
    {
      int socketToSendTo=userSockets.find(parse[1])->second;
      string message="";
      if (parse[1] != username)
      {
        if (parse.size()>2)
        {
          for (int i =2; i< parse.size(); i++)
          {
            message+=parse[i] + " ";
          }
        }
        message = username + " has accepted your request: " + message;
      }

      if(requestMap.find(parse[1])!=requestMap.end() && parse[1]!= username)
      {
        bool ia=false;
        for (auto it = requestMap.find(parse[1])->second.begin(); it!= requestMap.find(parse[1])->second.end(); ++it)
        {
          if (*it==username)
          {
            friendsMap.find(username)->second.push_back(parse[1]);
            friendsMap.find(parse[1])->second.push_back(username);
            ia = true;
            break;
          }
        }
        if (!ia)
        {
          message = "No request found from " +parse[1];
          write(sock, message.c_str(), message.length()+1);
        }
        write(socketToSendTo, message.c_str(), message.length()+1);
        message = "Accepted request from user: " +parse[1];
        write(sock, message.c_str(), message.length()+1);
      }
      else if (parse[1] == username)
      {
        string message= "You cannot send a request to yourself";
        write(sock, message.c_str(), message.length()+1);
      }
      else
      {
        string message= "User " + parse[1] + " Not online";
        write(sock, message.c_str(), message.length()+1);
      }
    }
    /**********
    HELP FUNCTION
    *********/
    else if (parse[0]== "h")
    {
      string userL="", friendsL="";
      //loggedInUsers.insert(pair <string, int> (username, 0));
      
      for (auto it = loggedInUsers.begin(); it!= loggedInUsers.end(); ++it)
      {
        userL+=(it->first);
        userL+=(", ");
      }
      write(sock, userL.c_str(), userL.length()+1);
      read(sock, buf, MAXBUFLEN);
      for (auto it=friendsMap.find(username)->second.begin();  
            it!= friendsMap.find(username)->second.end(); ++it)
      {
        //cout<<*it<<endl;
        if ((loggedInUsers.count(*it))>0)
        {
          //cout<<loggedInUsers.count(*it);
          friendsL+=(*it);
          friendsL+=(", ");
        }
      }
      write(sock, friendsL.c_str(), friendsL.length()+1);
    }
    /**********
    INSERT A PORT FOR A USER
    ********/
    else if (parse[0]== "port")
    {
      vector <string> portsHosts;
      portsHosts.push_back(parse[1]);
      portsHosts.push_back(parse[2]);
      PortMap.insert(pair <string, vector  <string> >(username, portsHosts));
    }

    /**********
    LOGOUT CLEARING MAPS
    *********/
    else if (parse[0] == "logout")
    {
      PortMap.erase(username);
      loggedInUsers.erase(username);
      userSockets.erase(username);
      return 0;
    }
    /**********
    SOMETHING WENT WRONG
    **********/
    else
    {
      cout<< parse[0]<<endl;
      string message="An Error occured, try again";
      write(sock, message.c_str(), message.length()+1);
    }
  }

}
void logout_function()
{
  string message="logout";
  for (auto it=userSockets.begin(); it!=userSockets.end(); ++it)
  {
    write(it->second, message.c_str(), message.length()+1);
  }
  message="";
  for (auto it=usersMap.begin(); it!=usersMap.end(); ++it)
  {
    message+=it->first+"|"+it->second+"|";
    for (auto it2=friendsMap.find(it->first)->second.begin(); it2!=friendsMap.find(it->first)->second.end(); ++it2)
      message+=*it2+";";
    message+='\n';
  }
  outInfo.write(message.c_str(), message.length()+1);
  outInfo.close();
  userInfo.close();
  configFile.close();
  exit(1);
}

//TODO HANDLE SIGINT 
//SAVE FILE
//CLOSE CONNECTIONS TO CLIENTS
//WRITE LOGOUT TO CLIENTS


