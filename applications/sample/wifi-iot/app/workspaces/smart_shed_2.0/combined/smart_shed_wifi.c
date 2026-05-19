#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "lwip/api_shell.h"
#include "lwip/netifapi.h"
#include "smart_shed_wifi.h"
#include "wifi_device.h"

int smart_shed_connect_wifi(void)
{
    WifiErrorCode errCode;
    WifiDeviceConfig apConfig = {};
    int netId = -1;

    strcpy(apConfig.ssid, "Rayawa");
    strcpy(apConfig.preSharedKey, "dddddddd");
    apConfig.securityType = WIFI_SEC_TYPE_PSK;

    errCode = EnableWifi();
    printf("EnableWifi: %d\r\n", errCode);
    if (errCode != WIFI_SUCCESS) {
        return errCode;
    }

    errCode = AddDeviceConfig(&apConfig, &netId);
    printf("AddDeviceConfig: %d, netId=%d\r\n", errCode, netId);
    if (errCode != WIFI_SUCCESS) {
        return errCode;
    }

    errCode = ConnectTo(netId);
    printf("ConnectTo(%d): %d\r\n", netId, errCode);
    if (errCode != WIFI_SUCCESS) {
        return errCode;
    }

    usleep(1000 * 1000);
    struct netif *iface = netifapi_netif_find("wlan0");
    printf("netifapi_netif_find(\"wlan0\"): %p\r\n", iface);
    if (iface != NULL) {
        err_t ret = netifapi_dhcp_start(iface);
        printf("netifapi_dhcp_start: %d\r\n", ret);

        usleep(5000 * 1000);
        ret = netifapi_netif_common(iface, dhcp_clients_info_show, NULL);
        printf("netifapi_netif_common: %d\r\n", ret);
    }

    return WIFI_SUCCESS;
}
