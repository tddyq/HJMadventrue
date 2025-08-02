#pragma once
class IClient
{
public:
	 IClient() = default;
	 virtual ~IClient() = default;
     virtual void login_to_server() {
         
     }
     virtual void uploadProgressToServer() {
         
     }
     virtual void getProgressFromServer() {
         
     }

     virtual void load_resources() {

     }
     virtual void init() {
         
     }
     virtual void mainLoop() {
         
     }
};

