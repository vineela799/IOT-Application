#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "Lavanya12";       // Replace with your WiFi SSID
const char* password = "no password"; // Replace with your WiFi Password

#define MQ2_SENSOR A0   // Analog pin for MQ2 sensor
#define BUZZER_PIN D2   // Buzzer pin (GPIO4)
#define THRESHOLD 45    // Updated threshold on a 0-100 scale

#define SENSOR_MIN 300  // Adjust based on calibration (low value in clean air)
#define SENSOR_MAX 1024 // Maximum sensor value in high gas concentration

ESP8266WebServer server(80);

// Function to get stable gas readings
int getStableGasValue() {
    int total = 0;
    for (int i = 0; i < 10; i++) {  // Take 10 readings
        total += analogRead(MQ2_SENSOR);
        delay(100);
    }
    return total / 10;  // Return the average value
}

// Function to map raw MQ2 value to a 0-100 scale
int mapGasValue(int rawValue) {
    return constrain(map(rawValue, SENSOR_MIN, SENSOR_MAX, 0, 100), 0, 100);
}

// Function to get current timestamp
String getTimeStamp() {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    char buffer[30];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return String(buffer);
}

// Handle Web Portal Display
void handleRoot() {
    int rawGasValue = getStableGasValue();
    int gasPercentage = mapGasValue(rawGasValue);
    String status = (gasPercentage > THRESHOLD) ? "⚠ Warning! High Gas Detected!" : "✅ Normal";
    String timestamp = getTimeStamp();

    String html = "<html><head><title>MQ2 Sensor</title>";
    html += "<style>body { font-family: Arial, sans-serif; text-align: center; } ";
    html += "table { width: 60%; margin: 20px auto; border-collapse: collapse; } ";
    html += "th, td { border: 1px solid #ddd; padding: 10px; } ";
    html += "th { background: #333; color: #fff; } </style></head><body>";

    // Title
    html += "<h1>Team - Batch41</h1>";

    // Table Format
    html += "<table>";
    html += "<tr><th>Parameter</th><th>Value</th></tr>";
    html += "<tr><td>Gas Level</td><td>" + String(gasPercentage) + "%</td></tr>";
    html += "<tr><td>Status</td><td>" + status + "</td></tr>";
    html += "<tr><td>Timestamp</td><td>" + timestamp + "</td></tr>";
    html += "</table>";

    // Prevention Measures
    if (gasPercentage > THRESHOLD) {
        html += "<h3>🚨 Prevention Measures:</h3>";
        html += "<p>1️⃣ Open windows and ensure proper ventilation.</p>";
        html += "<p>2️⃣ Avoid open flames and turn off electrical appliances.</p>";
    }

    // Footer
    html += "<footer style='margin-top:20px; font-size:14px;'>";
    html += "Copyright 2025 | Developed by Batch41 | Spirit of AI</footer>";
    html += "</body></html>";

    server.send(200, "text/html", html);
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(MQ2_SENSOR, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);  // Ensure buzzer is off initially

    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20) {
        delay(500);
        Serial.print(".");
        timeout++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
        Serial.print("Web Portal: http://");
        Serial.println(WiFi.localIP());  // Print the Web Portal link
    } else {
        Serial.println("\nError: WiFi Connection Failed!");
    }

    server.on("/", handleRoot);
    server.begin();
    Serial.println("Web Server Started.");
}

void loop() {
    int rawGasValue = getStableGasValue();
    int gasPercentage = mapGasValue(rawGasValue);
    
    Serial.print("Gas Sensor Value: ");
    Serial.print(rawGasValue);
    Serial.print(" -> ");
    Serial.print(gasPercentage);
    Serial.println("%");

    // Buzzer Control
    if (gasPercentage > THRESHOLD) {
        Serial.println(" Warning: High Gas Detected! Turning on Buzzer...");
        digitalWrite(BUZZER_PIN, HIGH); // Turn on buzzer continuously
    } else {
        Serial.println("Normal Air Quality. Turning off Buzzer...");
        digitalWrite(BUZZER_PIN, LOW);  // Turn off buzzer
    }

    server.handleClient();
    delay(1000);
}
