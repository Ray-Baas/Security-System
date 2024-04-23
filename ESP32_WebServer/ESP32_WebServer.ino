#include "WiFi.h"
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <WebSocketsClient.h>

// Replace with your network credentials
const char* ssid = "";
const char* password = "";

// Cambiar para configurar servidor
IPAddress local_IP(192, 168, 1, 98);
IPAddress gateway(192, 168, 1, 254);

// No cambiar
IPAddress subnet(255, 255, 255, 0);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">

<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Servidor UNAM 1</title>

    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet"
        integrity="sha384-QWTKZyjpPEjISv5WaRU9OFeRpok6YctnYmDr5pNlyT2bRjXh0JMhjY6hW+ALEwIH" crossorigin="anonymous">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.2/css/all.min.css" />
    <style>
        .camContainer,
        .card-img-top {
            width: 100%;
            min-height: 50vh;
            max-height: 50vh;
        }

        .sliders input {
            width: 100%;
        }
    </style>
</head>

<body>
    <header>
        <div class="navbar navbar-dark bg-dark shadow-sm">
            <div class="container">
                <a href="#" class="navbar-brand d-flex align-items-center">
                    <strong>Sistema de Seguridad</strong>
                </a>
            </div>
        </div>
    </header>

    <main>
        <div class="album py-5 bg-light">
            <div class="container">
                <div class="row">
                    <div class="col-12">
                        <h4 id="websocketStatus"></h4>
                    </div>
                </div>

                <div class="row row-cols-1 row-cols-sm-1 row-cols-md-2 g-3">
                    <div class="col">
                        <div class="card shadow-sm">
                            <div class="camContainer">
                                <img src="" alt="" class="card-img-top" id="img1">
                            </div>
                            <hr>
                            <div class="card-body">
                                <h5 class="card-title">ESP-CAM 1</h5>
                                <div class="d-flex justify-content-between align-items-center">
                                    <div class="">
                                        <button class="btn btn-primary" onclick="TomarFoto(1)">
                                            <i class="fa fa-camera"></i>
                                            Tomar foto
                                        </button>
                                        <button class="btn btn-success" onclick="EncenderFlash(1)">
                                            <i class="fa fa-lightbulb"></i>
                                            Encender/Apagar flash
                                        </button>
                                        <button class="btn btn-info" onclick="RefrescarImagen(1)">
                                            <i class="fa fa-refresh"></i>
                                            Recargar imagen
                                        </button>
                                    </div>
                                    <small class="text-muted" id="statusCam1"></small>
                                </div>
                                <hr>
                                <div class="d-flex sliders">
                                    <div class="col-5">
                                        <label for="pan">Pan</label>
                                        <br>
                                        <input id="pan" type="range" min="0" max="180" value="90">
                                        <p id="panValue">90</p>
                                    </div>
                                    <div class="col-5 offset-1">
                                        <label for="tilt">Tilt</label>
                                        <br>
                                        <input id="tilt" type="range" min="0" max="180" value="90">
                                        <p id="tiltValue">90</p>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                    <div class="col">
                        <div class="card shadow-sm">

                            <div class="camContainer">
                                <img src="" alt="" class="card-img-top" id="img2">
                            </div>
                            <hr>
                            <div class="card-body">
                                <h5 class="card-title">ESP-CAM 2</h5>
                                <div class="d-flex justify-content-between align-items-center">
                                    <div class="">
                                        <button class="btn btn-primary" onclick="TomarFoto(2)">
                                            <i class="fa fa-camera"></i>
                                            Tomar foto
                                        </button>
                                        <button class="btn btn-success" onclick="EncenderFlash(2)">
                                            <i class="fa fa-lightbulb"></i>
                                            Encender/Apagar flash
                                        </button>
                                        <button class="btn btn-info" onclick="RefrescarImagen(2)">
                                            <i class="fa fa-refresh"></i>
                                            Recargar imagen
                                        </button>
                                    </div>
                                    <small class="text-muted" id="statusCam2"></small>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </main>
    <footer class="text-muted py-5">
        <div class="container">
            <p class="mb-1">© 2024 Sistema de Seguridad UNAM. Todos los derechos reservados.</p>
        </div>
    </footer>

    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"
        integrity="sha384-YvpcrYf0tY3lHB60NNkmXc5s9fDVZLESaAA55NDzOxhy9GkcIdslK1eN7N6jIeHz"
        crossorigin="anonymous"></script>
    <script>
        const cam1Ip = 'http://192.168.1.99/';
        const cam1WebSocketIp = 'ws://192.168.1.99/ws';
        const cam2Ip = 'http://192.168.1.100';
        const cam2WebSocketIp = 'ws://192.168.1.100/ws';
        const statusText = 'Estado: ';
        var webSocketObj;
        var webSocketObj2;

        window.onload = () => {
            InitWebSocket();
            InitWebSocket2();
            LoadCamera(1);
            LoadCamera(2);
        }

        function LoadCamera(camera) {

            let id = 'img' + camera;
            var element = document.getElementById(id);
            if (camera == 1)
                ip = cam1Ip;
            else
                ip = cam2Ip;

            element.src = ip;
        }

        function RemoveCamera(camera) {
            let id = 'img' + camera;
            var element = document.getElementById(id);
            element.src = '';
        }

        function InitWebSocket() {
            webSocketObj = new WebSocket(cam1WebSocketIp);
            webSocketObj.onclose = (e) => {
                webSocketActive = false;
                AsignarEstadoWebSocket('Inactivo', 1);
                setTimeout(() => { }, 3000)
                AsignarEstadoWebSocket('Reconectando...', 1);
                InitWebSocket();
            };

            webSocketObj.onopen = (e) => {
                webSocketActive = true;
                AsignarEstadoWebSocket('Activo', 1);
            }

            webSocketObj.onerror = (e) => {
                AsignarEstadoWebSocket('Cerrado', 1);
            }

            webSocketObj.onmessage = (e) => {
                console.log(e.data);
            }
        }


        function InitWebSocket2() {
            webSocketObj2 = new WebSocket(cam2WebSocketIp);
            webSocketObj2.onclose = (e) => {
                webSocketActive = false;
                AsignarEstadoWebSocket('Inactivo', 2);
                setTimeout(() => { }, 3000)
                AsignarEstadoWebSocket('Reconectando...', 2);
                InitWebSocket2();
            };

            webSocketObj2.onopen = (e) => {
                webSocketActive = true;
                AsignarEstadoWebSocket('Activo', 2);
            }

            webSocketObj2.onerror = (e) => {
                AsignarEstadoWebSocket('Cerrado', 2);
            }

            webSocketObj2.onmessage = (e) => {
                console.log(e.data);
            }
        }

        function AsignarEstadoWebSocket(estado, camara) {
            var header = document.getElementById('statusCam' + camara);
            var txt = document.createTextNode(statusText + estado);
            var childs = header.childNodes;
            childs.forEach(element => {
                header.removeChild(element);
            });
            header.appendChild(txt);
        }

        function TomarFoto(camara) {
            setTimeout(() => {
                if (camara == 1)
                    webSocketObj.send("Foto");
                else
                    webSocketObj2.send("Foto");
            }, 100);
        }

        function EncenderFlash(camara) {
            setTimeout(() => {
                if (camara == 1)
                    webSocketObj.send("Flash");
                else
                    webSocketObj2.send("Flash");
            }, 100);
        }

        function RefrescarImagen(camara) {
            RemoveCamera(camara);
            setTimeout(() => {
                LoadCamera(camara);
            }, 500);
        }

        const panSlider = document.getElementById('pan');
        const tiltSlider = document.getElementById('tilt');

        panSlider.oninput = (e) => {
            let panValue = e.target.value;
            let tiltValue = tiltSlider.value;
            setTimeout(() => {
                webSocketObj.send("PanTilt," + panValue + "," + tiltValue);
            }, 100);
            document.getElementById("panValue").textContent = e.target.value;
        };

        tiltSlider.oninput = (e) => {
            let panValue = panSlider.value;
            let tiltValue = e.target.value;
            setTimeout(() => {
                webSocketObj.send("PanTilt," + panValue + "," + tiltValue);
            }, 100);
            document.getElementById("tiltValue").textContent = e.target.value;
        };
    </script>
</body>

</html>)rawliteral";

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  WiFi.config(local_IP,gateway,subnet);
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });

  // Start server
  server.begin();
}

void loop() {
}