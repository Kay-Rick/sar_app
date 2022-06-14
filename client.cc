
#include "sar_application.srpc.h"
#include "workflow/WFFacilities.h"
#include"serviceMgr/serviceMgr.h"
#include <iostream>

using namespace srpc;

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
	wait_group.done();
}

static void sar_open_interface_done(SAR_Open_Response *response, srpc::RPCContext *context)
{
}

static void sar_close_interface_done(SAR_Close_Response *response, srpc::RPCContext *context)
{
}

int main(int argc, char *argv[])
{
	if(argc!=3){
		printf("Usage: ./sar_client [ip] [port]\n");
		return 0;
	}

	//This will make timeout to 300s, which is 5min
	struct WFGlobalSettings settings = GLOBAL_SETTINGS_DEFAULT;
	settings.endpoint_params.connect_timeout = 2 * 1000;
	settings.endpoint_params.response_timeout = 300 *1000; //300 * 1000ms
	WORKFLOW_library_init(&settings);

	char *sar_ip = argv[1];
	int port = atoi(argv[2]);

	GOOGLE_PROTOBUF_VERIFY_VERSION;

	string open_sar_cmd = "open sar";
	string close_sar_cmd = "close sar";
	string get_time_cmd = "get time";

	printf("All commands are: \n");
	printf("%s\n", open_sar_cmd.c_str());
	printf("%s\n", close_sar_cmd.c_str());
	printf("%s\n", get_time_cmd.c_str());

	SAR::SRPCClient client(sar_ip, port);

	// example for RPC method call
	SAR_Close_Request sar_close_interface_req;
	SAR_Open_Request sar_open_interface_req;
	SAR_Close_Response sar_close_interface_resp;
	SAR_Open_Response sar_open_interface_resp;

	SAR_ReconstructTime_Request sar_time_req;
	SAR_ReconstructTime_Response sar_time_resp;



	sar_open_interface_req.set_longitude(1.0);
	sar_open_interface_req.set_latitude(2.0);
	//sar_open_interface_req.set_message("Hello, srpc!");

	RPCSyncContext target_Location_synctx;



	char str[16];
	for(;;){
		cin.getline(str, 16);
		//printf("str is %s\n", str);

		string s(str);

		if(s == open_sar_cmd){
			client.SAR_Open_Interface(&sar_open_interface_req, &sar_open_interface_resp, 
				&target_Location_synctx);
        	if (target_Location_synctx.success){
            	printf("Asyn invoke function open sar()\n"); 
         	 }else{
            	printf("Syn invoke function open sar failed ...\n");
         	 }
		}
		else if(s == close_sar_cmd){
			client.SAR_Close_Interface(&sar_close_interface_req, &sar_close_interface_resp, 
				&target_Location_synctx);
        	if (target_Location_synctx.success){
            	printf("Asyn invoke function close sar()\n");
         	 }else{
            	printf("Syn invoke function close sar failed ...\n");
         	 }
		}else if(s == get_time_cmd){
			client.SAR_Get_Reconstruct_Time(&sar_time_req, &sar_time_resp, 
				&target_Location_synctx);
        	if (target_Location_synctx.success){
            	printf("Asyn invoke function get time of sar, reconstruct time is %.3f\n", sar_time_resp.timelen());
         	 }else{
            	printf("Syn invoke function get time of sar failed ...\n");
         	 }


		}else
			printf("Unknown command\n");

	}


	//wait_group.wait();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
