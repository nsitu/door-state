/*
  Door Switch by Harold Sikkema
  Adapted from: https://RandomNerdTutorials.com/esp8266-nodemcu-https-requests/
  Based on the example created by Ivan Grokhotkov, 2015 (File > Examples > ESP8266WiFi > HTTPSRequests)
*/ 
 
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

// Root certificate (via Let's Encrypt)
// This makes it possible to make HTTPS requests
const char IRG_Root_X1 [] PROGMEM = R"CERT(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)CERT";

// Replace with your own WIFI network credentials
const char* ssid = "";
const char* password = "";
// this is the URL endpoint where our server will listen for updates
String endpoint =  "https://***.com/update/"; 

// My Door Switch is wired into this GPIO pin on my Feather board
const int doorSwitch = 5;
// Assume an initial state for the switch
bool lastSwitchState = HIGH;  
// Create a list of certificates with the server certificate
X509List cert(IRG_Root_X1);
// set a variable to hold the wifi client
WiFiClientSecure wifi_client;
// set up a variable to hold the https client
HTTPClient https; 

void setup() {
  // INPUT_PULLUP sets the doorSwitch pin to behave as an input. 
  // additionally it enables the internal pull-up resistor. 
  // When a button or switch is open, the pull-up resistor pulls the pin high. 
  // When the button is closed, the pin is pulled low. 
  pinMode(doorSwitch, INPUT_PULLUP);   

  // initialize serial communication between the Feather board and your computer
  Serial.begin(115200); 

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.'); 
    delay(1000);
  }
  Serial.println("Connected to WiFi"); 

  // Set the above Root Certificate as a trust anchors so that we can make HTTPS requests
  wifi_client.setTrustAnchors(&cert);
  Serial.println("Trust Anchors have been set using available Root Certificate.");
  
  // configTime ensures the system time is accurately set 
  // this is important for certificate validation.
  // the first parameter here is a timezone offset
  configTime(-5 * 3600, 0, "pool.ntp.org", "time.nist.gov"); 
  // wait for system time to be set
  while (time(nullptr) < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
  }
 
  
}


// ======================
// The loop continuously monitors the state of the door switch
// if a state change is detected we will send an HTTPS request
// to the relevant endpoint on our server. (i.e.  /close or /open)
// It also sends as part of the url, the current timestamp in ISO format 
// this timestamp includes milliseconds thanks to the ezTime library.
void loop() {
  // check the current state of the switch
  bool currentSwitchState = digitalRead(doorSwitch);  
  // Check if the button state has changed 
  if (currentSwitchState != lastSwitchState) {  
    String body;
    if (currentSwitchState == LOW) {  
      body = "{\"state\":\"closed\"}";
      Serial.println("Door is Closed");
    } else {  
      body = "{\"state\":\"open\"}";
      Serial.println("Door is Open");
    }
    // Send the HTTP request
    sendRequest(body);
    // update the switch state 
    lastSwitchState = currentSwitchState;
  }
  delay(100); // Small delay to prevent bouncing issues
}

// ========================
// This function sends a POST request to the API endpoint 
// The body will be set as a JSON string 
void sendRequest(const String& body) {  
  // wait for WiFi connection
  if (WiFi.status() == WL_CONNECTED) {  
    Serial.print("[HTTPS] begin...\n");
    if (https.begin(wifi_client, endpoint)) {  // HTTPS
      // tell the server to expect a JSON body
      https.addHeader("Content-Type", "application/json");
      Serial.print("[HTTPS] POST... " + endpoint + "\n"); 
      Serial.print( body + "\n"); 
      // start connection and send HTTP header
      int httpCode = https.POST(body); 
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] POST... code: %d\n", httpCode); 
      } else {
        Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
      } 
      https.end(); 
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  // TODO: what if the Wifi Goes down? 
}


