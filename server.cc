
#include "sar_application.srpc.h"
#include "workflow/WFFacilities.h"
#include  <iostream>
#include <signal.h>
#include <unistd.h>
#include"serviceMgr/serviceMgr.h"
using namespace srpc;
using namespace std;

std::string dsp_ip;
int dsp_port;

std::string sar_arm_ip;
int sar_arm_port;

Feature_Etraction_Request feature_req;

static void registService(const char* serviceIP, int servicePort, int check_health_port,
                        const char *serviceName, int weight);


void sig_handler(int signo)
{
}

static void dsp_do_nothing(DSPControllerResponse *response, srpc::RPCContext *ctx)
{
	printf("dsp empty do nothing response!\n");
}

static void noise_call_back(NoiseReduction_Response *response, srpc::RPCContext *ctx)
{
	feature_req.set_longitude(response->longitude());
	feature_req.set_latitude(response->latitude());
	feature_req.set_image_vector(response->image_vector());
	printf("noise_reduction  response!\n");
}

static void feature_call_back(Feature_Etraction_Response *response, srpc::RPCContext *ctx)
{
	printf("Feature_Etraction  response!\n");
}

class SARServiceImpl : public SAR::Service
{
public:

	void SAR_Open_Interface(SAR_Open_Request *request, SAR_Open_Response *response, srpc::RPCContext *ctx) override
	{
		Log(NOTICE,"received request for function SAR_Open_Interface() \n");

		DSPController::SRPCClient client(dsp_ip.c_str(), dsp_port);
		DSPControllerRequest req;
		DSPControllerResponse resp;
		Log(NOTICE,"Step 1 start loading sar!");
		//1. load sar
        RPCSyncContext target_Location_synctx;

        client.LoadSarApp(&req, &resp, &target_Location_synctx);
        if (target_Location_synctx.success){
            Log(NOTICE,"Asyn invoke function load sar()\n");
          }else{
            Log(ERROR,"Syn invoke function load sar() failed ...");
          }
		
		Log(NOTICE,"Step 2 sleep!");
		//2. sleep 45s
		sleep(7);
		#if 0



		Log(NOTICE,"Step 3 start sar!");
		//3. start sar
		client.StartSarApp(&req, &resp, &target_Location_synctx);
        if (target_Location_synctx.success){
            Log(NOTICE,"Asyn invoke function start sar() \n");
          }else{
            Log(ERROR,"Syn invoke function start sar() failed ...");
          }


		Log(NOTICE,"Step 4 start noise reduction!");
		//4. noise reduction
		SAR_iamge_processing_service::SRPCClient sar_arm_client(sar_arm_ip.c_str(), sar_arm_port);
		NoiseReduction_Request noireq;
		NoiseReduction_Response noiresp;
		noireq.set_longitude(request->longitude());
		noireq.set_latitude(request->latitude());
		Log(NOTICE,"Noise reduction param is (%f, %f)", noireq.longitude(), noireq.latitude());

		sar_arm_client.noise_reduction(&noireq, &noiresp, &target_Location_synctx);
        if (target_Location_synctx.success){
            Log(NOTICE,"Asyn invoke function noise_reduction()\n");
          }else{
            Log(ERROR,"Syn invoke function noise_reduction failed ...");
          }
	

		//5. feature todo
		Log(NOTICE,"Step 5 start feature eraction!");
		Feature_Etraction_Request feature_req;
		Feature_Etraction_Response feature_resp;
		feature_req.set_longitude(noiresp.longitude());
		feature_req.set_latitude(noiresp.latitude());
		feature_req.set_image_vector(noiresp.image_vector());

		sar_arm_client.feature_etraction(&feature_req, &feature_resp, &target_Location_synctx);
        if (target_Location_synctx.success){
            Log(NOTICE,"Asyn invoke function Feature_Etraction()\n");
          }else{
            Log(ERROR,"Syn invoke function Feature_Etraction failed ...");
          }

        Log(NOTICE,"function SAR_Open_Interface() is invoked over\n");
		printf("get_req:\n%s\n",request->DebugString().c_str());
		#endif
	}

	void SAR_Close_Interface(SAR_Close_Request *request, SAR_Close_Response *response, srpc::RPCContext *ctx) override
	{
		Log(NOTICE,"received request for function SAR_Close_Interface() \n");
        
		
		DSPController::SRPCClient client(dsp_ip.c_str(), dsp_port);
		DSPControllerRequest req;
		DSPControllerResponse resp;

        RPCSyncContext target_Location_synctx;

        client.StopSarApp(&req, &resp, &target_Location_synctx);
        if (target_Location_synctx.success){
            Log(NOTICE,"Asyn invoke function close sar()\n");
          }else{
            Log(ERROR,"Syn invoke function close sar failed ...");
          }

        Log(NOTICE,"function SAR_Close_Interface() is invoked over\n");
	}

	void SAR_Get_Reconstruct_Time(SAR_ReconstructTime_Request *request, SAR_ReconstructTime_Response *response,
					srpc::RPCContext *ctx) override
	{
		Log(NOTICE,"received request for function SAR_Get_Time_Interface() \n");
        
		DSPController::SRPCClient client(dsp_ip.c_str(), dsp_port);
		DSPControllerRequest req;
		ReconstructTimeResponse resp;

        RPCSyncContext target_Location_synctx;
		float time = 0.0;


		//get reconstruct time until not zero or failed
		do{
			client.GetSarReconstructTime(&req, &resp, &target_Location_synctx);
        	if (target_Location_synctx.success){
            	time = resp.time();
          	}else{
				  Log(NOTICE,"Asyn invoke function get sar time() failed ...\n");
				  break;
          	}
		}while(time == 0.0);

		Log(NOTICE,"Asyn invoke function get sar time()\n");
		response->set_timelen(time);
        Log(NOTICE,"function SAR_Get_Reconstruct_Time() is invoked over\n");


	}
};

int main(int argc, char *argv[])
{
	if(argc != 3){
        printf("Usage: ./main [ip] [weight]\n");
        return 1;
    }

    char *sar_ip = argv[1];
    int weight = atoi(argv[2]);

	struct WFGlobalSettings settings = GLOBAL_SETTINGS_DEFAULT;
	settings.endpoint_params.connect_timeout = 2 * 1000;
	settings.endpoint_params.response_timeout = 300 *1000;
	WORKFLOW_library_init(&settings);

	
	signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
	log_file = fopen("log.txt","w");


	std::ifstream ifs;
    ifs.open("config.json");
    assert(ifs.is_open());

    Json::Reader reader;
	Json::Value root;
    // 解析到root，root将包含Json里所有子元素
	if (!reader.parse(ifs, root, false))
	{
		cerr << "parse failed \n";
		return -1;
	}

	string dsp_agent = root["dsp_agent_name"].asString();
	string sar_arm = root["sar_arm_name"].asString();
    int sar_port = root["sar_port"].asInt();
    int check_health_port = root["check_health_port"].asInt();
    int is_regist_service = root["is_regist_service"].asInt();
    string service_name = root["sar_name"].asString();

	if(is_regist_service)
        registService(sar_ip, sar_port, check_health_port, 
				service_name.c_str(), weight);

	//init dsp address
	dsp_ip = "192.168.1.147";
	dsp_port = 5103;

	printf("init server done!\n");


	//init current server
	SRPCServer server;

	SARServiceImpl sar_impl;
	server.add_service(&sar_impl);
	printf("The address of Sar_application is %s:%d\n",sar_ip, sar_port);
	server.start(sar_port);

	printf("Sar_application started, press enter to exit......\n");
    getchar(); // press "Enter" to end.
	server.stop();
	//google::protobuf::ShutdownProtobufLibrary();

	#if 0

	SRPCServer server;

	SARServiceImpl sar_impl;
	server.add_service(&sar_impl);

	server.start(1412);
	printf("Sar_application started, press enter to exit......\n");
	getchar();
	server.stop();
	#endif
	return 0;
}


static void registService(const char* serviceIP, int servicePort, int check_health_port,
                        const char *serviceName, int weight){
    string serviceAddress = string(serviceIP)+":"+to_string(servicePort);
    string serviceProtocol = "srpc";
    string serviceVersion = "v1.0";
    string serviceProvider = "gpf";

    int checkType = TCP_CHECK;
    string interval = "10s";
    string timeout = "3s";
    string checkDeregisterCriticalServiceAfter = "2m";

    static int count = 0;
    string checkURL = string(serviceIP)+ ":" + to_string(check_health_port);
    count++;

    ServiceInfo *serviceInfo = new ServiceInfo(serviceName,"",serviceIP,servicePort,serviceProtocol,serviceVersion,serviceProvider,
    weight,checkType,checkURL,interval,checkDeregisterCriticalServiceAfter,timeout);

    printf("%s:%d %s\n", serviceIP, servicePort, serviceName);
    int regRes = registerService(serviceInfo);
    if(regRes != 0){
        printf("Register service failed, the service stop...\n");
    }else{
        printf("Regist service %s done!\n", serviceName);
    }

    delete serviceInfo;
}
