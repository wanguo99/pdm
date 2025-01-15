#include <libwebsockets.h>
#include <mosquitto.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <cjson/cJSON.h>

// 宏定义
#define WEB_SOCKET_PORT 8080
#define MQTT_BROKER_HOST "localhost"
#define MQTT_BROKER_PORT 1883
#define MQTT_KEEPALIVE_INTERVAL 60
#define BUFFER_SIZE 512 // WebSocket消息缓冲区大小

static struct mosquitto *mosq = NULL;
static volatile int g_exit_flag = 0;

// 处理SIGINT信号以便优雅地关闭程序
void sigint_handler(int sig) {
    g_exit_flag = 1;
}

// MQTT连接回调函数
void on_mqtt_connect(struct mosquitto *m, void *obj, int result) {
    printf("Connected to MQTT broker with code %d\n", result);
}

// 初始化 MQTT 客户端
void mqtt_init() {
    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Error creating mosquitto client.\n");
        exit(1);
    }

    // 设置MQTT连接回调函数
    mosquitto_connect_callback_set(mosq, on_mqtt_connect);

    if (mosquitto_connect(mosq, MQTT_BROKER_HOST, MQTT_BROKER_PORT, MQTT_KEEPALIVE_INTERVAL)) {
        fprintf(stderr, "Unable to connect to MQTT broker.\n");
        mosquitto_lib_cleanup();
        exit(1);
    }
}

// 发布 MQTT 消息
void mqtt_publish_message(const char *message, const char *topic) {
    if (mosq && message && topic) {
        mosquitto_publish(mosq, NULL, topic, strlen(message), message, 0, false);
    }
}

// 定义事件类型枚举
typedef enum {
    EVENT_PUBLISH_MQTT_EVENT,
    EVENT_REQUEST_TOPICS,
    EVENT_INVALID
} EventType;

// 将事件字符串映射到枚举值的函数
EventType event_type_from_string(const char *type_str) {
    if (strcmp(type_str, "publish_mqtt_event") == 0)
        return EVENT_PUBLISH_MQTT_EVENT;
    else if (strcmp(type_str, "request_topics") == 0)
        return EVENT_REQUEST_TOPICS;
    else
        return EVENT_INVALID;
}

// 发布 MQTT 事件的处理器
void handle_publish_mqtt_event(cJSON *root, struct lws *wsi) {
    cJSON *topic_item = cJSON_GetObjectItem(root, "topic");
    cJSON *payload_item = cJSON_GetObjectItem(root, "payload");

    if (cJSON_IsString(topic_item) && topic_item->valuestring &&
        cJSON_IsString(payload_item) && payload_item->valuestring) {
        // 发布 MQTT 消息
        mqtt_publish_message(payload_item->valuestring, topic_item->valuestring);

        // 回复客户端
        const char reply[] = "MQTT event published!";
        lws_write(wsi, (unsigned char *)reply, strlen(reply), LWS_WRITE_TEXT);
    } else {
        // 处理无效的topic或payload情况
        const char reply[] = "Invalid topic or payload.";
        lws_write(wsi, (unsigned char *)reply, strlen(reply), LWS_WRITE_TEXT);
    }
}

// 请求主题列表的处理器
void handle_request_topics(struct lws *wsi) {
    // 创建包含主题列表的JSON响应
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "type", "topics");

    cJSON *topics_array = cJSON_CreateArray();
    // 假定有一个全局变量或函数可以获取主题列表
    // 这里我们使用静态数组作为示例
    const char *topics[] = {"mqtt_topic_switch", "mqtt_topic_dimmer", "mqtt_topic_sensor", "mqtt_topic_nvmem"};
    for (size_t i = 0; i < sizeof(topics)/sizeof(topics[0]); ++i) {
        cJSON_AddItemToArray(topics_array, cJSON_CreateString(topics[i]));
    }
    cJSON_AddItemToObject(response, "topics", topics_array);

    // 将JSON对象转换为字符串并发送给客户端
    char *json_str = cJSON_PrintUnformatted(response);
    lws_write(wsi, (unsigned char *)json_str, strlen(json_str), LWS_WRITE_TEXT);
    free(json_str); // 释放由 cJSON_PrintUnformatted 分配的内存

    // 清理 JSON 对象
    cJSON_Delete(response);
}

// 处理未知请求类型的处理器
void handle_invalid_request(struct lws *wsi) {
    // 处理其他类型的消息或错误情况
    const char reply[] = "Unknown request type.";
    lws_write(wsi, (unsigned char *)reply, strlen(reply), LWS_WRITE_TEXT);
}

// WebSocket 回调函数
static int callback_websocket(struct lws *wsi,
                              enum lws_callback_reasons reason,
                              void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_RECEIVE:
            {
                char buffer[LWS_PRE + BUFFER_SIZE]; // 使用宏定义的缓冲区大小
                memcpy(buffer + LWS_PRE, in, len);
                buffer[LWS_PRE + len] = '\0'; // 确保字符串以null结尾

                printf("Received message: %s\n", buffer + LWS_PRE);

                // 解析JSON消息
                cJSON *root = cJSON_Parse(buffer + LWS_PRE);
                if (!root) {
                    const char reply[] = "Invalid JSON.";
                    lws_write(wsi, (unsigned char *)reply, strlen(reply), LWS_WRITE_TEXT);
                    break;
                }

                cJSON *type = cJSON_GetObjectItem(root, "type");
                if (!cJSON_IsString(type) || !type->valuestring) {
                    cJSON_Delete(root);
                    handle_invalid_request(wsi);
                    break;
                }

                // 将事件字符串映射到枚举值，并根据事件类型调用对应的处理器
                EventType event_type = event_type_from_string(type->valuestring);
                switch (event_type) {
                    case EVENT_PUBLISH_MQTT_EVENT:
                        handle_publish_mqtt_event(root, wsi);
                        break;
                    case EVENT_REQUEST_TOPICS:
                        handle_request_topics(wsi);
                        break;
                    case EVENT_INVALID:
                        handle_invalid_request(wsi);
                        break;
                }

                cJSON_Delete(root);
            }
            break;

        default:
            break;
    }
    return 0;
}

static struct lws_protocols protocols[] = {
    {
        "", // 如果不需要特定的子协议，则可以为空字符串
        callback_websocket, // 回调函数
        0 // 每个连接的用户数据大小
    },
    { NULL, NULL, 0 } // 结束标志
};

int main() {
    struct lws_context *context;
    struct lws_context_creation_info info;

    signal(SIGINT, sigint_handler); // 注册SIGINT信号处理器

    memset(&info, 0, sizeof(info));
    info.port = WEB_SOCKET_PORT; // 使用宏定义的WebSocket监听端口
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;

    context = lws_create_context(&info);
    if (context == NULL) {
        fprintf(stderr, "libwebsockets init failed\n");
        return -1;
    }

    // 初始化 MQTT 客户端
    mqtt_init();

    printf("WebSocket server started on port %d\n", WEB_SOCKET_PORT);

    // 进入事件循环
    while (!g_exit_flag) {
        lws_service(context, 50);
    }

    // 断开 MQTT 连接
    if (mosq) {
        mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
    }
    mosquitto_lib_cleanup();

    lws_context_destroy(context);
    return 0;
}