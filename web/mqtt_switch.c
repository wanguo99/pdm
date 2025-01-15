#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 宏定义
#define MQTT_BROKER_HOST "localhost"
#define MQTT_BROKER_PORT 1883
#define MQTT_KEEPALIVE_INTERVAL 60
#define MQTT_TOPIC_SWITCH "mqtt_topic_switch"

static struct mosquitto *mosq = NULL;

// MQTT连接回调函数
void on_mqtt_connect(struct mosquitto *m, void *obj, int result) {
    printf("Connected to MQTT broker with code %d\n", result);
    if (result == MOSQ_ERR_SUCCESS) {
        // 成功连接后订阅 mqtt_topic_switch 主题
        mosquitto_subscribe(mosq, NULL, MQTT_TOPIC_SWITCH, 0);
    }
}

// MQTT消息回调函数
void on_mqtt_message(struct mosquitto *m, void *obj, const struct mosquitto_message *message) {
    if (message && message->payload) {
        printf("Received message on topic '%s': %s\n", message->topic, (char *)message->payload);

        // 根据接收到的消息内容执行相应操作
        if (strcmp(MQTT_TOPIC_SWITCH, message->topic) == 0) {
            char payload[256];
            snprintf(payload, sizeof(payload), "%.*s", message->payloadlen, (char *)message->payload);

            // 这里可以根据实际需要处理开关逻辑
            if (strcmp(payload, "ON") == 0) {
                printf("Switch is ON.\n");
                // 执行开启动作的代码
            } else if (strcmp(payload, "OFF") == 0) {
                printf("Switch is OFF.\n");
                // 执行关闭动作的代码
            } else {
                printf("Unknown command for switch: %s\n", payload);
            }
        }
    }
}

// 初始化 MQTT 客户端
void mqtt_init() {
    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Error creating mosquitto client.\n");
        exit(1);
    }

    // 设置MQTT连接和消息回调函数
    mosquitto_connect_callback_set(mosq, on_mqtt_connect);
    mosquitto_message_callback_set(mosq, on_mqtt_message);

    // 尝试连接到MQTT broker
    if (mosquitto_connect(mosq, MQTT_BROKER_HOST, MQTT_BROKER_PORT, MQTT_KEEPALIVE_INTERVAL)) {
        fprintf(stderr, "Unable to connect to MQTT broker.\n");
        mosquitto_lib_cleanup();
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    // 初始化 MQTT 客户端
    mqtt_init();

    // 进入事件循环
    while (1) {
        mosquitto_loop(mosq, -1, 1); // 使用非阻塞模式
    }

    // 断开 MQTT 连接并清理资源
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}
