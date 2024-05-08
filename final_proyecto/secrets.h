#include <pgmspace.h>
#define SECRET
 
const char WIFI_SSID[] = "SSID";
const char WIFI_PASSWORD[] = "Passw";
 
#define THINGNAME "ESP8266"
 
int8_t TIME_ZONE = -5; //NYC(USA): -5 UTC
const char MQTT_HOST[] = "HOST_MQTT";
 
 
static const char cacert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
CADENA DE CERT
-----END CERTIFICATE-----
)EOF";
 
 
// Copy contents from XXXXXXXX-certificate.pem.crt here ▼
static const char client_cert[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
CADENA DE CERT
-----END CERTIFICATE-----
 
)KEY";
 
 
// Copy contents from  XXXXXXXX-private.pem.key here ▼
static const char privkey[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
CADENA DE CERT
-----END RSA PRIVATE KEY-----
 
)KEY";
