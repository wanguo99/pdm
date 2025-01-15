#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// MQTT broker information
#define MQTT_BROKER_HOST "localhost"
#define MQTT_BROKER_PORT 1883
#define MQTT_KEEPALIVE_INTERVAL 60

// Switch topic names (replace with actual topics)
#define SWITCH_ON_TOPIC "home/switch/1/on"
#define SWITCH_OFF_TOPIC "home/switch/1/off"
#define STATUS_TOPIC "home/switch/1/status"

static struct mosquitto *mosq = NULL;

// Callback for successful connection to the broker
void on_connect(struct mosquitto *m, void *obj, int result) {
    if (result == MOSQ_ERR_SUCCESS) {
        printf("Connected to MQTT broker\n");
        // Subscribe to status updates from the switch
        mosquitto_subscribe(mosq, NULL, STATUS_TOPIC, 0);
    } else {
        fprintf(stderr, "Connection failed: %d\n", result);
    }
}

// Callback for receiving messages
void on_message(struct mosquitto *m, void *obj, const struct mosquitto_message *message) {
    if (message->topic && message->payload) {
        printf("Received message on topic %s: %.*s\n", message->topic, message->payloadlen, (char *)message->payload);
        // Here you can add code to update local state or trigger actions based on the received message.
    }
}

// Initialize and connect the MQTT client
int init_mqtt_switch_client() {
    int rc;

    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Error creating mosquitto client.\n");
        return -1;
    }

    // Set callback functions
    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_message_callback_set(mosq, on_message);

    // Connect to the broker
    rc = mosquitto_connect(mosq, MQTT_BROKER_HOST, MQTT_BROKER_PORT, MQTT_KEEPALIVE_INTERVAL);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Unable to connect to MQTT broker: %d\n", rc);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return -1;
    }

    // Start a background thread to handle network communication
    rc = mosquitto_loop_start(mosq);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Failed to start MQTT loop.\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return -1;
    }

    return 0;
}

// Publish a command to turn the switch on
void switch_on() {
    const char *msg = "ON";
    mosquitto_publish(mosq, NULL, SWITCH_ON_TOPIC, strlen(msg), msg, 0, false);
}

// Publish a command to turn the switch off
void switch_off() {
    const char *msg = "OFF";
    mosquitto_publish(mosq, NULL, SWITCH_OFF_TOPIC, strlen(msg), msg, 0, false);
}

// Cleanup resources when done
void cleanup_mqtt_switch_client() {
    if (mosq) {
        mosquitto_disconnect(mosq);
        mosquitto_loop_stop(mosq, true); // Force stop the loop thread
        mosquitto_destroy(mosq);
    }
    mosquitto_lib_cleanup();
}

// Example main function to demonstrate usage
int main() {
    if (init_mqtt_switch_client() != 0) {
        fprintf(stderr, "Failed to initialize MQTT switch client.\n");
        return -1;
    }

    // Simulate turning the switch on and off after some delay
    switch_on();
    sleep(2); // Wait for 2 seconds before switching off
    switch_off();

    // Run for a while to receive any status updates
    sleep(10);

    // Clean up and exit
    cleanup_mqtt_switch_client();
    return 0;
}