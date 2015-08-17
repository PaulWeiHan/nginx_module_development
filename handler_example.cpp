#include "transfer.h"



example_handler(sscc_C_request_t *req_c,sscc_C_response_t *resp_c)
{
	sscc_CPP_request_t req_cpp;
	sscc_CPP_response_t resp_cpp;
	request_C_2_CPP(&req_c,&req_cpp);
	real_handler(&req_cpp,&resp_cpp);
	response_CPP_2_C(&resp_c,&resp_c);
}

realhandler(sscc_CPP_request_t *req_cpp,sscc_CPP_response_t *resp_cpp)
{
	//your code
}