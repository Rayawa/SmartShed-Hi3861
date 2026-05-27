#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cmsis_os2.h"
#include "MQTTPacket.h"

#ifdef SMART_SHED_ENABLE_LIGHT_INTENSITY
#include "light_intensity_task.h"
#endif
#include "lwip/sockets.h"
#include "smart_shed_mqtt.h"
#include "smart_shed_shared.h"
#include "smart_shed_wifi.h"

#ifdef SMART_SHED_ENABLE_SOIL_MOISTURE
#include "soil_moisture_task.h"
#endif
#ifdef SMART_SHED_ENABLE_TEMP_AND_HUM
#include "temp_and_hum_task.h"
#endif
#include "transport.h"

#define MQTT_HOST "broker.emqx.io"
#define MQTT_PORT 1883
#define SMART_SHED_MAX_SUB_TOPICS 3

static int mqtt_send_connect(int sock, const char *client_id)
{
    unsigned char buf[256];
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    unsigned char session_present = 0;
    unsigned char connack_rc = 0;

    data.clientID.cstring = (char *)client_id;
    data.keepAliveInterval = 20;
    data.cleansession = 1;

    int len = MQTTSerialize_connect(buf, sizeof(buf), &data);
    int rc = transport_sendPacketBuffer(sock, buf, len);
    printf("MQTT connect send rc=%d len=%d client=%s\n", rc, len, client_id);

    if (MQTTPacket_read(buf, sizeof(buf), transport_getdata) != CONNACK) {
        return -1;
    }
    if (MQTTDeserialize_connack(&session_present, &connack_rc, buf, sizeof(buf)) != 1 || connack_rc != 0) {
        printf("MQTT connack failed rc=%d\n", connack_rc);
        return -1;
    }
    return 0;
}

static int mqtt_publish_json(int sock, const char *topic, const char *payload)
{
    unsigned char buf[256];
    MQTTString topic_string = MQTTString_initializer;

    topic_string.cstring = (char *)topic;
    int len = MQTTSerialize_publish(buf, sizeof(buf), 0, 0, 0, 0, topic_string,
        (unsigned char *)payload, strlen(payload));
    return transport_sendPacketBuffer(sock, buf, len);
}

static void mqtt_apply_command(const char *topic, const char *payload)
{
    int level = atoi(payload);

#ifdef SMART_SHED_ENABLE_FAN
    if (strcmp(topic, "Rayawa/fan_1") == 0) {
        fan_level = level;
        return;
    }
#endif
#ifdef SMART_SHED_ENABLE_LED
    if (strcmp(topic, "Rayawa/led_1") == 0) {
        led_level = level;
        return;
    }
#endif
#ifdef SMART_SHED_ENABLE_WATER_PUMP
    if (strcmp(topic, "Rayawa/water_pump_1") == 0) {
        water_pump_level = level;
        return;
    }
#endif
}

static void smart_shed_mqtt_thread(void *arg)
{
    (void)arg;
    MQTTString sub_topics[SMART_SHED_MAX_SUB_TOPICS];
    int req_qos[SMART_SHED_MAX_SUB_TOPICS] = {0};
    int granted_qos[SMART_SHED_MAX_SUB_TOPICS] = {0};
    int sub_topic_count = 0;
    unsigned char buf[256];

#ifdef SMART_SHED_ENABLE_FAN
    sub_topics[sub_topic_count++].cstring = "Rayawa/fan_1";
#endif
#ifdef SMART_SHED_ENABLE_LED
    sub_topics[sub_topic_count++].cstring = "Rayawa/led_1";
#endif
#ifdef SMART_SHED_ENABLE_WATER_PUMP
    sub_topics[sub_topic_count++].cstring = "Rayawa/water_pump_1";
#endif

    while (1) {
        if (smart_shed_connect_wifi() != 0) {
            sleep(2);
            continue;
        }

        int sock = transport_open(MQTT_HOST, MQTT_PORT);
        if (sock < 0) {
            printf("transport_open failed: %d\n", sock);
            sleep(2);
            continue;
        }

        if (mqtt_send_connect(sock, "hi3861_smart_shed_all") != 0) {
            transport_close(sock);
            sleep(2);
            continue;
        }

        if (sub_topic_count > 0) {
            int len = MQTTSerialize_subscribe(buf, sizeof(buf), 0, 1, sub_topic_count, sub_topics, req_qos);
            int rc = transport_sendPacketBuffer(sock, buf, len);
            printf("MQTT subscribe send rc=%d len=%d\n", rc, len);

            if (MQTTPacket_read(buf, sizeof(buf), transport_getdata) != SUBACK) {
                transport_close(sock);
                sleep(2);
                continue;
            }

            unsigned short submsgid = 0;
            int subcount = 0;
            if (MQTTDeserialize_suback(&submsgid, sub_topic_count, &subcount, granted_qos, buf, sizeof(buf)) != 1) {
                transport_close(sock);
                sleep(2);
                continue;
            }
        }

        printf("MQTT connected and subscribed\n");

        while (1) {
            char payload[64];

#ifdef SMART_SHED_ENABLE_SOIL_MOISTURE
            snprintf(payload, sizeof(payload), "{\"moisture\":%d}", moisture);
            mqtt_publish_json(sock, "Rayawa/soil_moisture_1", payload);
#endif

#ifdef SMART_SHED_ENABLE_LIGHT_INTENSITY
            snprintf(payload, sizeof(payload), "{\"intensity\":%d}", intensity);
            mqtt_publish_json(sock, "Rayawa/light_intensity_1", payload);
#endif

#ifdef SMART_SHED_ENABLE_TEMP_AND_HUM
            snprintf(payload, sizeof(payload), "{\"temp\":%d,\"hum\":%d}", temp, hum);
            mqtt_publish_json(sock, "Rayawa/temp_and_hum_1", payload);
#endif

            int packet_type = MQTTPacket_read(buf, sizeof(buf), transport_getdata);
            if (packet_type == PUBLISH) {
                unsigned char dup = 0;
                int qos = 0;
                unsigned char retained = 0;
                unsigned short msgid = 0;
                int payload_len = 0;
                unsigned char *payload_in = NULL;
                MQTTString received_topic = MQTTString_initializer;
                char topic[32];

                if (MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &received_topic,
                        &payload_in, &payload_len, buf, sizeof(buf)) == 1 &&
                    payload_len > 0 && payload_len < (int)sizeof(payload)) {
                    int topic_len = received_topic.lenstring.len;
                    if (topic_len >= (int)sizeof(topic)) {
                        topic_len = sizeof(topic) - 1;
                    }
                    memcpy(topic, received_topic.lenstring.data, topic_len);
                    topic[topic_len] = '\0';
                    memcpy(payload, payload_in, payload_len);
                    payload[payload_len] = '\0';
                    printf("MQTT cmd topic=%s payload=%s\n", topic, payload);
                    mqtt_apply_command(topic, payload);
                }
            }
        }

        transport_close(sock);
        sleep(2);
    }
}

void smart_shed_mqtt_task(void)
{
    osThreadAttr_t attr = {0};
    attr.name = "smart_shed_mqtt";
    attr.stack_size = 8192;
    attr.priority = osPriorityNormal;

    if (osThreadNew(smart_shed_mqtt_thread, NULL, &attr) == NULL) {
        printf("[smart_shed_mqtt] Failed to create mqtt thread!\n");
    }
}
