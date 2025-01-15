#include <libwebsockets.h>
#include <mosquitto.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <cjson/cJSON.h>

static struct mosquitto *mosq = NULL;
static volatile int g_exit_flag = 0;

void sigint_handler(int sig)
{
	g_exit_flag = 1;
}

void on_mqtt_connect(struct mosquitto *m, void *obj, int result)
{
	printf("Connected to MQTT broker with code %d\n", result);
}

void mqtt_init()
{
	mosquitto_lib_init();

	mosq = mosquitto_new(NULL, true, NULL);
	if (!mosq) {
		fprintf(stderr, "Error creating mosquitto client.\n");
		exit(1);
	}

	mosquitto_connect_callback_set(mosq, on_mqtt_connect);

	if (mosquitto_connect(mosq, "localhost", 1883, 60)) {
		fprintf(stderr, "Unable to connect to MQTT broker.\n");
		mosquitto_lib_cleanup();
		exit(1);
	}
}

void mqtt_publish_message(const char *message, const char *topic)
{
	if (mosq && message && topic) {
		mosquitto_publish(mosq, NULL, topic, strlen(message), message, 0, false);
	}
}

static int callback_websocket(struct lws *wsi,
							  enum lws_callback_reasons reason,
							  void *user, void *in, size_t len)
{
	switch (reason) {
		case LWS_CALLBACK_RECEIVE:
		{
			char buffer[LWS_PRE + 512];
			memcpy(buffer + LWS_PRE, in, len);
			buffer[LWS_PRE + len] = '\0';

			printf("Received message: %s\n", buffer + LWS_PRE);

			cJSON *root = cJSON_Parse(buffer + LWS_PRE);
			if (root && cJSON_GetObjectItem(root, "type")->valuestring &&
				!strcmp(cJSON_GetObjectItem(root, "type")->valuestring, "publish_mqtt_event")) {

				const char *topic = cJSON_GetObjectItem(root, "topic")->valuestring;
				const char *payload = cJSON_GetObjectItem(root, "payload")->valuestring;

				// 发布 MQTT 消息
				mqtt_publish_message(payload, topic);

				// 回复客户端
				const char reply[] = "MQTT event published!";
				lws_write(wsi, (unsigned char *)reply, strlen(reply), LWS_WRITE_TEXT);
			} else {
				// 处理其他类型的消息或错误情况
				const char reply[] = "Invalid request.";
				lws_write(wsi, (unsigned char *)reply, strlen(reply), LWS_WRITE_TEXT);
			}

			cJSON_Delete(root);
			break;
		}
		default:
		{
			break;
		}
	}
	return 0;
}

static struct lws_protocols protocols[] = {
	{
		"", // 如果不需要特定的子协议，则可以为空字符串
		callback_websocket, // 回调函数
		0 // 每个连接的用户数据大小
	},
	{ NULL, NULL, 0 }
};

int main() {
	struct lws_context *context;
	struct lws_context_creation_info info;

	signal(SIGINT, sigint_handler);

	memset(&info, 0, sizeof(info));
	info.port = 8080;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;

	context = lws_create_context(&info);
	if (context == NULL) {
		fprintf(stderr, "libwebsockets init failed\n");
		return -1;
	}

	mqtt_init();

	printf("WebSocket server started on port %d\n", info.port);

	while (!g_exit_flag) {
		lws_service(context, 50);
	}

	if (mosq) {
		mosquitto_disconnect(mosq);
		mosquitto_destroy(mosq);
	}
	mosquitto_lib_cleanup();

	lws_context_destroy(context);
	return 0;
}
