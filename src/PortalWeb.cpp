#include "PortalWeb.h"
#include <WiFi.h>
#include <ArduinoJson.h>

const char htmlContent[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Concentrador UWB - TWR</title>
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif; background-color: #f0f2f5; margin: 0; padding: 20px; color: #333; }
        .container { max-width: 1200px; margin: 0 auto; }
        h1, h2 { color: #0056b3; text-align: center; }
        .card { background-color: #fff; border: 1px solid #ddd; border-radius: 8px; padding: 20px; margin-bottom: 20px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
        .card-title { font-weight: bold; font-size: 1.2em; color: #0056b3; margin-bottom: 15px; border-bottom: 2px solid #0056b3; padding-bottom: 10px;}
        .position-display { text-align: center; font-size: 1.5em; font-weight: bold; margin: 20px 0; }
        table { width: 100%; border-collapse: collapse; font-size: 0.9em; } 
        th, td { padding: 8px 12px; border: 1px solid #ddd; text-align: left; } 
        th { background-color: #e9ecef; white-space: nowrap; } 
        tbody tr:nth-child(odd) { background-color: #f9f9f9; }
        td { white-space: nowrap; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Concentrador UWB - Trilateración</h1>
        <div class="card">
            <div class="card-title">Posición Calculada del Tag</div>
            <div id="tag-position" class="position-display">X: N/A, Y: N/A, Z: N/A</div>
        </div>
        <div class="card">
            <div class="card-title">Reportes de Rango de Anclas</div>
            <div style="overflow-x:auto;">
                <table id="anchor-table">
                    <thead>
                        <tr><th>Ancla SAddr</th><th>Tag UID</th><th>Seq</th><th>Rango (m)</th><th>Temp (°C)</th><th>Accel SQ (g)</th><th>RXPACC</th><th>Ruido Std</th><th>Potencia CIR</th></tr>
                    </thead>
                    <tbody>
                        <tr><td colspan="9" style="text-align:center;">Esperando datos...</td></tr>
                    </tbody>
                </table>
            </div>
        </div>
    </div>

    <script>
        function updateTable(anchorData) {
            const tableBody = document.querySelector("#anchor-table tbody");
            tableBody.innerHTML = '';
            if (Object.keys(anchorData).length === 0) {
                tableBody.innerHTML = '<tr><td colspan="9" style="text-align:center;">Esperando datos...</td></tr>';
                return;
            }
            for (const id in anchorData) {
                const data = anchorData[id];
                const row = `<tr>
                    <td>0x${data.anchor_saddr.toString(16).toUpperCase()}</td>
                    <td>0x${data.tag_uid.toString(16).toUpperCase()}</td>
                    <td>${data.seq}</td>
                    <td>${data.range_m.toFixed(3)}</td>
                    <td>${data.temp.toFixed(2)}</td>
                    <td>${data.aSQ.toFixed(3)}</td>
                    <td>${data.rxpacc}</td>
                    <td>${data.std_noise}</td>
                    <td>${data.cir_pwr}</td>
                </tr>`;
                tableBody.innerHTML += row;
            }
        }

        setInterval(() => {
            fetch('/data').then(r => r.json()).then(data => {
                const pos = data.tag_position;
                document.getElementById('tag-position').textContent = `X: ${pos.x.toFixed(2)}, Y: ${pos.y.toFixed(2)}, Z: ${pos.z.toFixed(2)}`;
                updateTable(data.anchors);
            }).catch(console.error);
        }, 2000);
    </script>
</body>
</html>
)rawliteral";

PortalWeb::PortalWeb(const char* ssid, const char* password) 
    : _server(80), _manager(nullptr), _ssid(ssid), _password(password) {}

void PortalWeb::begin(String mac, PositioningManager& manager) {
    _manager = &manager;

    WiFi.softAP(_ssid, _password);
    IPAddress apIP = WiFi.softAPIP();
    DEBUG_PRINTLN("Punto de Acceso iniciado.");
    DEBUG_PRINT("Conéctate a la red: "); DEBUG_PRINTLN(_ssid);
    DEBUG_PRINT("IP para acceder al portal: http://"); DEBUG_PRINTLN(apIP);
    DEBUG_PRINTLN("-----------------------------------");

    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", htmlContent);
    });

    _server.on("/data", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (!_manager) return request->send(500, "text/plain", "Manager no inicializado");

        StaticJsonDocument<2048> doc;
        
        Point pos = _manager->getLastTagPosition();
        doc["tag_position"]["x"] = pos.x;
        doc["tag_position"]["y"] = pos.y;
        doc["tag_position"]["z"] = pos.z;

        JsonVariant anchors = doc.createNestedObject("anchors");
        const auto& anchorMap = _manager->getAnchorDataMap();
        for (auto const& [saddr, data] : anchorMap) {
            JsonObject anchorObj = anchors.createNestedObject(String(saddr, HEX));
            anchorObj["anchor_saddr"] = data.anchor_saddr;
            anchorObj["tag_uid"] = data.tag_uid;
            anchorObj["seq"] = data.seq;
            anchorObj["range_m"] = data.range_m;
            // Añadir datos de diagnóstico y sensores al JSON
            anchorObj["temp"] = data.temp;
            anchorObj["aSQ"] = data.aSQ;
            anchorObj["rxpacc"] = data.rxpacc;
            anchorObj["std_noise"] = data.std_noise;
            anchorObj["cir_pwr"] = data.cir_pwr;
        }

        String jsonResponse;
        serializeJson(doc, jsonResponse);
        request->send(200, "application/json", jsonResponse);
    });

    _server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Página no encontrada");
    });

    _server.begin();
}