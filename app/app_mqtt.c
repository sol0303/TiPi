#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include "MQTTClient.h"
#include "application.h"

#define ROOT_TOPIC	"/tipi"
#define TOPIC_ADD_TIP	"/tip"
#define TOPIC_ADD_NOTE	"/note"
#define HOST_ADDR	 ((char*)"localhost")
#define HOST_PORT	1883

int mqtt_listen_stop = 0;

//输出this目录后的第一级目录的位置和长度，如输入this = /tipi ; topic_in = /tipi/config/
//输出 topic_in+5  7
static int topic_parse(const char* this, const char* topic_in, char** sub_topic, int* len) 
{
	int tmp = 0;
	char* ch = strstr(topic_in, this);
	if (ch == NULL)
		return -1;
	ch = ch + strlen(this);
	if (*ch != '/' || (*(ch+1) == '\0'))
		return -1;	
	*sub_topic = ch;
	do
	{
		ch++;
		tmp++;
	}
	while(*ch != '\0' && *ch != '/');
	*len = tmp;
	return 0;
}

void recv_callback(MessageData* md)
{
	MQTTMessage* message = md->message;

	printf("%.*s\t", md->topicName->lenstring.len, md->topicName->lenstring.data);
	printf("%.*s\n", (int)message->payloadlen, (char*)message->payload);

	char payload[1024];
	strncpy(payload, message->payload, message->payloadlen);
	payload[message->payloadlen] = '\0';


	char topic_name[256];
	if (md->topicName->lenstring.len > 255)
		return;
	strncpy(topic_name, md->topicName->lenstring.data, md->topicName->lenstring.len);
	topic_name[md->topicName->lenstring.len] = '\0';

	char this[64];
	strcpy(this, ROOT_TOPIC);
	char* sub;
	char* p_in = topic_name;
	int len;
	while(topic_parse(this, p_in, &sub, &len) == 0)
	{

		printf("sub topic:%s\n", sub);
		strncpy(this, sub, len);
		this[len] = '\0';
		if (strcmp(this, TOPIC_ADD_TIP) == 0)
		{
			add_tip(payload);
		}
		else if (strcmp(this, TOPIC_ADD_NOTE) == 0)
		{
			add_note(payload);
		}



		p_in = sub;
	}	
	//fflush(stdout);
}

struct sub_arg
{
	const char* host_addr;
	int port;
	const char* client_id;
	const char* topic;
	int qos;
};

static void* mqtt_subscribe(void * args)
{
	struct sub_arg* p = (struct sub_arg*)args;
	Network n;
	MQTTClient c;
	int rc = 0;

	unsigned char* sendbuf = NULL;
	unsigned char* recvbuf = NULL;
	sendbuf = (unsigned char*)malloc(2048);
	recvbuf = (unsigned char*)malloc(2048);
	if (sendbuf == NULL)
		return -1;
	if (recvbuf == NULL)
	{
		free(sendbuf);
		return -1;
	}

	NetworkInit(&n);
	NetworkConnect(&n, p->host_addr, p->port);
	MQTTClientInit(&c, &n, 1000, sendbuf, 100, recvbuf, 100);
 
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = p->client_id;
	data.keepAliveInterval = 10;
	data.cleansession = 1;
	printf("Connecting to %s %d\n", p->host_addr, p->port);
	
	rc = MQTTConnect(&c, &data);
	if (rc != 0)
	{
		printf("Connect fail %d\n", rc);
		rc = -2;
		goto END2;
	}	
    
    printf("Subscribing to %s\n", p->topic);
	rc = MQTTSubscribe(&c, p->topic, p->qos, recv_callback);
	if (rc != 0)
	{
		printf("Subscrib fail %d\n", rc);
		rc = -3;
		goto END1;
	}	

	while (!mqtt_listen_stop)
	{
		MQTTYield(&c, 1000);	
	}

END1:
	MQTTDisconnect(&c);
END2:
	NetworkDisconnect(&n);
	free(sendbuf);
	free(recvbuf);
	printf("subscribe ret:%d\n", rc);
	return NULL;
}

int mqtt_sub_start()
{
	int tid;
	struct sub_arg p;
	p.client_id = "sol_client@RaspberryPi";
	p.host_addr = HOST_ADDR;
	p.port = HOST_PORT;
	p.qos = 1;
	p.topic = ROOT_TOPIC"/#";					//用一个client处理所有topic，多起几个线程分别处理不同topic亦可
	pthread_create(&tid, NULL, mqtt_subscribe, &p);
	// pthread_join(tid, NULL);
	return 0;
}
