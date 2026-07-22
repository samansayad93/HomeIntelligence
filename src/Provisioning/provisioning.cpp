#include <Provisioning/provisioning.h>

#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <config.h>

ProvisioningConfig provisioningConfig;

static const char PROVISIONING_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Device Setup</title>
<style>
  *{box-sizing:border-box;margin:0;padding:0}
  body{font-family:Arial,sans-serif;background:#f0f4f8;display:flex;justify-content:center;align-items:center;min-height:100vh;padding:16px}
  .card{background:#fff;border-radius:12px;box-shadow:0 4px 20px rgba(0,0,0,.12);padding:32px;max-width:420px;width:100%}
  h1{font-size:1.4rem;color:#1a202c;margin-bottom:8px}
  p.sub{font-size:.875rem;color:#718096;margin-bottom:24px}
  h2{font-size:.95rem;color:#4a5568;text-transform:uppercase;letter-spacing:.05em;margin:20px 0 12px}
  label{display:block;font-size:.85rem;color:#4a5568;margin-bottom:4px}
  input{width:100%;padding:10px 12px;border:1px solid #e2e8f0;border-radius:8px;font-size:.95rem;outline:none;transition:border .2s}
  input:focus{border-color:#4299e1;box-shadow:0 0 0 3px rgba(66,153,225,.2)}
  .row{margin-bottom:14px}
  .hint{font-size:.78rem;color:#a0aec0;margin-top:3px}
  button{width:100%;padding:12px;background:#4299e1;color:#fff;border:none;border-radius:8px;font-size:1rem;cursor:pointer;margin-top:24px;transition:background .2s}
  button:hover{background:#3182ce}
  .divider{height:1px;background:#e2e8f0;margin:20px 0}
</style>
</head>
<body>
<div class="card">
  <h1>&#128279; Device Setup</h1>
  <p class="sub">Configure your Wi-Fi and MQTT connection.</p>
  <form method="POST" action="/save">
    <h2>Wi-Fi</h2>
    <div class="row">
      <label for="ssid">SSID</label>
      <input type="text" id="ssid" name="ssid" required placeholder="Network name" autocomplete="off">
    </div>
    <div class="row">
      <label for="wpass">Password</label>
      <input type="password" id="wpass" name="wpass" placeholder="Leave blank if open network">
    </div>
    <div class="divider"></div>
    <h2>MQTT Broker</h2>
    <div class="row">
      <label for="mhost">Host / IP</label>
      <input type="text" id="mhost" name="mhost" required placeholder="192.168.1.10" autocomplete="off">
    </div>
    <div class="row">
      <label for="mport">Port</label>
      <input type="number" id="mport" name="mport" value="1883" min="1" max="65535">
    </div>
    <div class="row">
      <label for="muser">Username</label>
      <input type="text" id="muser" name="muser" placeholder="Optional" autocomplete="off">
    </div>
    <div class="row">
      <label for="mpass">Password</label>
      <input type="password" id="mpass" name="mpass" placeholder="Optional">
    </div>
    <div class="divider"></div>
    <h2>Client</h2>
    <div class="row">
        <label for="cid">Device</label>
        <input type="text" id="cid" name="cid" placeholder="Device name">
    </div>
    <button type="submit">Save &amp; Restart</button>
  </form>
</div>
</body>
</html>
)rawliteral";

static const char SAVED_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Saved</title>
<style>
  *{box-sizing:border-box;margin:0;padding:0}
  body{font-family:Arial,sans-serif;background:#f0f4f8;display:flex;justify-content:center;align-items:center;min-height:100vh}
  .card{background:#fff;border-radius:12px;box-shadow:0 4px 20px rgba(0,0,0,.12);padding:40px;max-width:380px;width:100%;text-align:center}
  .icon{font-size:3rem;margin-bottom:16px}
  h1{font-size:1.3rem;color:#2d3748;margin-bottom:8px}
  p{color:#718096;font-size:.9rem}
</style>
</head>
<body>
<div class="card">
  <div class="icon">&#10004;&#65039;</div>
  <h1>Settings Saved</h1>
  <p>The device will restart and connect to your network.</p>
</div>
</body>
</html>
)rawliteral";

static WebServer server(80);
static DNSServer dnsServer;
static bool provisioningDone = false;

static bool ensureLittleFS()
{
    if (LittleFS.begin(false))
    {
        return true;
    }

    Serial.println("Provisioning: formatting LittleFS...");
    if (!LittleFS.begin(true))
    {
        Serial.println("Provisioning: LittleFS mount failed");
        return false;
    }

    return true;
}

bool loadProvisioningConfig()
{
    if (!ensureLittleFS())
    {
        return false;
    }

    if (!LittleFS.exists(PROVISIONING_FILE_PATH))
    {
        return false;
    }

    File f = LittleFS.open(PROVISIONING_FILE_PATH, "r");
    if (!f)
    {
        Serial.println("Provisioning: failed to open config file");
        return false;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err)
    {
        Serial.print("Provisioning: JSON parse error: ");
        Serial.println(err.c_str());
        return false;
    }

    provisioningConfig.wifiSSID = doc["ssid"] | "";
    provisioningConfig.wifiPassword = doc["wpass"] | "";
    provisioningConfig.mqttHost = doc["mhost"] | "";
    provisioningConfig.mqttPort = doc["mport"] | 1883;
    provisioningConfig.mqttUser = doc["muser"] | "";
    provisioningConfig.mqttPassword = doc["mpass"] | "";
    provisioningConfig.client = doc["cid"] | "";

    return provisioningConfig.wifiSSID.length() > 0 && provisioningConfig.mqttHost.length() > 0;
}

bool saveProvisioningConfig(const ProvisioningConfig &cfg)
{
    if (!ensureLittleFS())
    {
        return false;
    }

    File f = LittleFS.open(PROVISIONING_FILE_PATH, "w");
    if (!f)
    {
        Serial.println("Provisioning: failed to open config file for writing");
        return false;
    }

    JsonDocument doc;
    doc["ssid"] = cfg.wifiSSID;
    doc["wpass"] = cfg.wifiPassword;
    doc["mhost"] = cfg.mqttHost;
    doc["mport"] = cfg.mqttPort;
    doc["muser"] = cfg.mqttUser;
    doc["mpass"] = cfg.mqttPassword;
    doc["cid"] = cfg.client;

    bool ok = serializeJson(doc, f) > 0;
    f.close();
    return ok;
}

bool isProvisioned()
{
    return loadProvisioningConfig();
}

static void handleRoot()
{
    server.send_P(200, "text/html", PROVISIONING_HTML);
}

static void redirectToPortal()
{
    // 3xx redirect to the absolute AP URL. iOS / Android / Windows auto-present
    // their captive-portal browser when the connectivity probe is redirected; a
    // plain 200 page is usually treated as "no internet" and never opens the page.
    String url = "http://";
    url += WiFi.softAPIP().toString();
    url += "/";
    server.sendHeader("Location", url, true);
    server.send(302, "text/plain", "");
}

static void handleSave()
{
    ProvisioningConfig cfg;
    cfg.wifiSSID = server.arg("ssid");
    cfg.wifiPassword = server.arg("wpass");
    cfg.mqttHost = server.arg("mhost");

    String portStr = server.arg("mport");
    cfg.mqttPort = portStr.length() > 0 ? (uint16_t)portStr.toInt() : 1883;
    if (cfg.mqttPort == 0)
    {
        cfg.mqttPort = 1883;
    }

    cfg.mqttUser = server.arg("muser");
    cfg.mqttPassword = server.arg("mpass");
    cfg.client = server.arg("cid");

    if (cfg.wifiSSID.length() == 0 || cfg.mqttHost.length() == 0 || cfg.client.length() == 0)
    {
        server.send(400, "text/plain", "SSID and MQTT host and Device name are required.");
        return;
    }

    if (saveProvisioningConfig(cfg))
    {
        provisioningConfig = cfg;
        server.send_P(200, "text/html", SAVED_HTML);
        provisioningDone = true;
    }
    else
    {
        server.send(500, "text/plain", "Failed to save configuration.");
    }
}

static void handleNotFound()
{
    redirectToPortal();
}

void runProvisioningPortal()
{
    Serial.println("Provisioning: starting AP...");

    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(PROVISIONING_AP_SSID, PROVISIONING_AP_PASSWORD))
    {
        Serial.println("Provisioning: softAP failed — verify AP password is 8-63 chars");
    }

    IPAddress apIP = WiFi.softAPIP();
    Serial.print("Provisioning AP: ");
    Serial.println(PROVISIONING_AP_SSID);
    Serial.print("Provisioning URL: http://");
    Serial.println(apIP);

    // Captive portal: capture every DNS query and point it at the AP IP so that
    // connecting clients auto-open the setup page (iOS / Android / Windows).
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", apIP);
    Serial.println("Provisioning: captive portal active");

    server.on("/", HTTP_GET, handleRoot);
    // OS connectivity-check probes — redirecting them (instead of returning the
    // expected "success"/204 response) is what makes the captive portal pop up
    // automatically on iOS / Android / Windows.
    server.on("/generate_204", HTTP_GET, redirectToPortal);        // Android
    server.on("/gen_204", HTTP_GET, redirectToPortal);             // Android (alt)
    server.on("/hotspot-detect.html", HTTP_GET, redirectToPortal); // Apple
    server.on("/ncsi.txt", HTTP_GET, redirectToPortal);            // Windows
    server.on("/connecttest.txt", HTTP_GET, redirectToPortal);     // Windows
    server.on("/redirect", HTTP_GET, redirectToPortal);            // Windows
    server.on("/save", HTTP_POST, handleSave);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("Provisioning: web server started");

    while (!provisioningDone)
    {
        dnsServer.processNextRequest();
        server.handleClient();
        delay(2);
    }

    dnsServer.stop();
    server.stop();
    Serial.println("Provisioning: complete, restarting...");
    delay(1500);
    ESP.restart();
}
