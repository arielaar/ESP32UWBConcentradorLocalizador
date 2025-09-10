#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "DataUtils.h"
#include "PositioningManager.h"
#include "PortalWeb.h"

// --- CONFIGURACIÓN ---
const char* pmk_key_str = "pmk-123456789012";
#define AP_SSID "ESP32-Concentrador"
#define AP_PASSWORD "123456789"
#define MIN_ANCHORS_FOR_CALCULATION 4

// --- OBJETOS GLOBALES ---
PortalWeb portal(AP_SSID, AP_PASSWORD);
PositioningManager manager(MIN_ANCHORS_FOR_CALCULATION);

// FUNCIÓN CALLBACK: Se ejecuta cuando se recibe un mensaje por ESP-NOW
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
    if (len == sizeof(AnchorRangeReport_t)) {
        AnchorRangeReport_t packedReport;
        memcpy(&packedReport, incomingData, sizeof(packedReport));
        
        DecodedAnchorReport_t decodedReport = unpack_anchor_report(packedReport);
        
        manager.addAnchorReport(decodedReport);
    } else {
        DEBUG_PRINTF("Error: Tamaño de paquete incorrecto. Esperado: %d, Recibido: %d\n", sizeof(AnchorRangeReport_t), len);
    }
}

void setup() {
    Serial.begin(115200);
    DEBUG_PRINTLN("\n== INICIANDO CONCENTRADOR TWR V4 ==");

    // !!! TAREA CRÍTICA: CONFIGURAR POSICIONES DE LAS ANCLAS !!!
    // Debes poner las coordenadas 3D REALES (en metros) y el ID CORTO de tus anclas.
    manager.setAnchorPosition(0x1001, 0.0, 0.0, 2.5);
    manager.setAnchorPosition(0x1002, 5.0, 0.0, 2.5);
    manager.setAnchorPosition(0x1003, 5.0, 5.0, 2.5);
    manager.setAnchorPosition(0x1004, 0.0, 5.0, 2.5);
    DEBUG_PRINTLN("[SETUP] Posiciones de anclas configuradas.");

    WiFi.mode(WIFI_AP_STA);
    String mac = WiFi.macAddress();
    portal.begin(mac, manager);

    if (esp_now_init() != ESP_OK) {
        DEBUG_PRINTLN("Error al inicializar ESP-NOW");
        return;
    }

    esp_now_set_pmk((const uint8_t *)pmk_key_str);
    esp_now_register_recv_cb(OnDataRecv);
    DEBUG_PRINTLN("ESP-NOW inicializado. Esperando reportes de rango...");
    DEBUG_PRINTLN("=================================================");
}

void loop() {
    delay(2000);
}