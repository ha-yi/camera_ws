#include <WiFi.h>
#include <WebSocketsServer.h>
#include "esp_http_server.h"
#include "esp_camera.h"
#include "Arduino.h"


#define MOTOR_A_FORWARD 12  // Motor A forward
#define MOTOR_A_BACKWARD 13 // Motor A backward
#define MOTOR_B_LEFT 14     // Motor B left (steering)
#define MOTOR_B_RIGHT 15    // Motor B right (steering)


// WiFi credentials
const char* ssid = "ESP32-CAM-AP";
const char* password = "12345678";

// Server and websocket initialization
httpd_handle_t server = NULL;
WebSocketsServer webSocket = WebSocketsServer(81);

bool isStreaming = false;

// Camera configuration
camera_config_t config;

// Task handle for camera streaming
TaskHandle_t streamTaskHandle = NULL;

/**
 * Initialize camera with required configuration
 * Returns: bool - Success status of camera initialization
 */
bool initCamera() {
    Serial.println("Initializing camera...");
    
    // Pin Kamera punya ESP32-CAM
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 5;
    config.pin_d1 = 18;
    config.pin_d2 = 19;
    config.pin_d3 = 21;
    config.pin_d4 = 36;
    config.pin_d5 = 39;
    config.pin_d6 = 34;
    config.pin_d7 = 35;
    config.pin_xclk = 0;
    config.pin_pclk = 22;
    config.pin_vsync = 25;
    config.pin_href = 23;
    config.pin_sscb_sda = 26;
    config.pin_sscb_scl = 27;
    config.pin_pwdn = 32;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera initialization failed with error 0x%x", err);
        return false;
    }
    
    Serial.println("Camera initialized successfully");
    return true;
}

/**
 * Set up motor control pins and initialize them to LOW state
 * Configures pins defined by MOTOR_A_FORWARD, MOTOR_A_BACKWARD, 
 * MOTOR_B_LEFT, and MOTOR_B_RIGHT as outputs
 */
void setup_motor() {
   // Initialize motor control pins
    pinMode(MOTOR_A_FORWARD, OUTPUT);
    pinMode(MOTOR_A_BACKWARD, OUTPUT);
    pinMode(MOTOR_B_LEFT, OUTPUT);
    pinMode(MOTOR_B_RIGHT, OUTPUT);
}

/**
 * Process motor control commands received from websocket
 * @param command String command to control motor movement
 *               Valid commands: "forward", "backward", "left", "right", "stop",
 *               "forward-left", "forward-right", "backward-left", "backward-right"
 */
void process_motor_command(const char *command) {
    Serial.printf("Processing command: %s\n", command);
    

    if (strcmp(command, "forward") == 0) {
        digitalWrite(MOTOR_A_FORWARD, HIGH);
        digitalWrite(MOTOR_A_BACKWARD, LOW);
    }
    else if (strcmp(command, "backward") == 0) {
        digitalWrite(MOTOR_A_FORWARD, LOW);
        digitalWrite(MOTOR_A_BACKWARD, HIGH);
    }
    else if (strcmp(command, "left") == 0) {
        digitalWrite(MOTOR_B_LEFT, HIGH);
        digitalWrite(MOTOR_B_RIGHT, LOW);
        delay(100); // butuh delay untuk steering
        digitalWrite(MOTOR_B_LEFT, LOW);
        digitalWrite(MOTOR_B_RIGHT, LOW);
    }
    else if (strcmp(command, "right") == 0) {
        digitalWrite(MOTOR_B_LEFT, LOW);
        digitalWrite(MOTOR_B_RIGHT, HIGH);
        delay(100); // butuh delay untuk steering
        digitalWrite(MOTOR_B_LEFT, LOW);
        digitalWrite(MOTOR_B_RIGHT, LOW);
    }
    
    else if (strcmp(command, "stop") == 0) {
        digitalWrite(MOTOR_A_FORWARD, LOW);
        digitalWrite(MOTOR_A_BACKWARD, LOW);
        digitalWrite(MOTOR_B_LEFT, LOW);
        digitalWrite(MOTOR_B_RIGHT, LOW);
    }
}

/**
 * Handle streaming commands received via websocket
 * Params: char* command - Command received from websocket
 */
void stream_commands(char* command) {
    Serial.printf("Received stream command: %s\n", command);
    
    if (strcmp(command, "P") == 0) {
        isStreaming = true;
        Serial.println("Streaming started");
    } else if (strcmp(command, "O") == 0) {
        isStreaming = false;
        Serial.println("Streaming stopped");
    }
}

/**
 * Handle motor control commands received via websocket
 * Params: char* command - Command received from websocket
 */
void motor_control(char* command) {
    Serial.printf("Received motor command: %s\n", command);
    
    // Single key commands
    switch(command[0]) {
        case 'w':
            process_motor_command("forward");
            break;
        case 's':
            process_motor_command("backward");
            break;
        case 'a':
            process_motor_command("left");
            break;
        case 'd':
            process_motor_command("right");
            break;
        case 'e':
            process_motor_command("stop");
            break;
    }
}

/**
 * Handle websocket events
 */
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            isStreaming = false;
            break;
            
        case WStype_CONNECTED:
            Serial.printf("[%u] Connected!\n", num);
            break;
            
        case WStype_TEXT:
            char command[3];  // Increased size to handle 2 chars + null terminator
            if (length > 2) length = 2;  // Limit to 2 chars
            memcpy(command, payload, length);
            command[length] = '\0';
            
            if (command[0] == 'P' || command[0] == 'O') {
                stream_commands(command);
            } else {
                motor_control(command);
            }
            break;
    }
}

/**
 * Stream camera frames via websocket - runs on Core 0
 */
void streamCameraTask(void * parameter) {
    for(;;) {
        if (isStreaming) {
            camera_fb_t * fb = esp_camera_fb_get();
            if (!fb) {
                Serial.println("Frame buffer capture failed");
                continue;
            }
            
            webSocket.broadcastBIN(fb->buf, fb->len);
            esp_camera_fb_return(fb);
        }
        vTaskDelay(1); // Small delay to prevent watchdog triggers
    }
}

/* URI handler for root path */
esp_err_t root_handler(httpd_req_t *req) {
    const char* html = "<html><head>"
        "<style>"
        "body { font-family: Arial, sans-serif; text-align: center; }"
        ".button { margin: 10px; padding: 10px 20px; }"
        ".controls { margin: 20px; }"
        "</style>"
        "<script>"
        "var ws = new WebSocket('ws://192.168.1.1:81/');"
        "ws.binaryType = 'arraybuffer';"
        "var activeKeys = new Set();"
        "var currentInterval = null;"
        "ws.onmessage = function(evt) {"
        "  if (typeof evt.data === 'string') return;"
        "  document.getElementById('stream').src = URL.createObjectURL(new Blob([evt.data], { type: 'image/jpeg' }));"
        "};"
        "function sendCommand(cmd) { ws.send(cmd); }"
        "function updateMovement() {"
        "  const keys = Array.from(activeKeys).sort().join('');"
        "  if (keys) sendCommand(keys);"
        "  else sendCommand('e');"
        "}"
        "document.addEventListener('keydown', function(e) {"
        "  const validKeys = {'w': true, 'a': true, 's': true, 'd': true, 'e': true};"
        "  if (validKeys[e.key] && !activeKeys.has(e.key)) {"
        "    activeKeys.add(e.key);"
        "    if (!currentInterval) {"
        "      updateMovement();"
        "      currentInterval = setInterval(updateMovement, 100);"
        "    }"
        "  }"
        "});"
        "document.addEventListener('keyup', function(e) {"
        "  if (activeKeys.has(e.key)) {"
        "    activeKeys.delete(e.key);"
        "    if (activeKeys.size === 0) {"
        "      clearInterval(currentInterval);"
        "      currentInterval = null;"
        "      sendCommand('e');"
        "    }"
        "  }"
        "});"
        "</script></head><body>"
        "<img id='stream' style='width:640px;height:480px;'>"
        "<div><button class='button' onclick='sendCommand(\"P\")'>Start Stream</button>"
        "<button class='button' onclick='sendCommand(\"O\")'>Stop Stream</button></div>"
        "<div class='controls'>"
        "<p>Use W,A,S,D keys to control movement. E to stop.</p>"
        "</div></body></html>";

    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, html, strlen(html));
}

void setup() {
    Serial.begin(115200);
    Serial.println("Initializing system...");

    setup_motor();

    // Initialize Camera
    if (!initCamera()) {
        Serial.println("Camera initialization failed");
        return;
    }

    // Configure AP Mode
    WiFi.softAP(ssid, password);
    IPAddress IP = IPAddress(192, 168, 1, 1);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(IP, gateway, subnet);
    
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Configure HTTP Server
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = root_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &root);
        Serial.println("HTTP server started");
    }

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    
    // Create camera streaming task on Core 0
    xTaskCreatePinnedToCore(
        streamCameraTask,    // Task function
        "StreamCamera",      // Task name
        10000,              // Stack size
        NULL,               // Task parameters
        1,                  // Priority
        &streamTaskHandle,  // Task handle
        0                   // Core ID (0)
    );

    Serial.println("System initialization completed");
}

void loop() {
    webSocket.loop();
}
