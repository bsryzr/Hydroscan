#include <SoftwareSerial.h>

static char recv_buf[512];
static int led = 2;

SoftwareSerial LoRaSerial(10, 11); // RX TX

static int at_send_check_response(const char *p_ack, int timeout_ms, const char *p_cmd, ...)
{
    int ch;
    int index = 0;
    int startMillis = 0;
    va_list args;
    memset(recv_buf, 0, sizeof(recv_buf));
    va_start(args, p_cmd);
    LoRaSerial.print(p_cmd);
    Serial.print(p_cmd);
    va_end(args);
    delay(200);
    startMillis = millis();

    if (p_ack == NULL)
        return 0;

    do
    {
        while (LoRaSerial.available() > 0)
        {
            ch = LoRaSerial.read();
            recv_buf[index++] = ch;
            Serial.print((char)ch);
            delay(2);
        }

        if (strstr(recv_buf, p_ack) != NULL)
            return 1;

    } while (millis() - startMillis < timeout_ms);
    Serial.println();
    return 0;
}

static void recv_parse(char *p_msg)
{
    if (p_msg == NULL)
    {
        Serial.println("Received null");
        return;
    }

    char *p_start = NULL;
    char data[128]; // To hold the received bytes as characters
    p_start = strstr(p_msg, "RX");

    if (p_start && (1 == sscanf(p_start, "RX \"%[^\"]\"", data)))
    {
        // Parse the received data
        char tds_str[5], ph_str[3], temp_str[3], turb_str[5];
        strncpy(tds_str, data, 4);
        tds_str[4] = '\0';
        strncpy(ph_str, data + 4, 2);
        ph_str[2] = '\0';
        strncpy(temp_str, data + 6, 2);
        temp_str[2] = '\0';
        strncpy(turb_str, data + 8, 4);
        turb_str[4] = '\0';

        uint32_t tdsValue = atoi(tds_str);
        uint32_t phValue = atoi(ph_str);
        uint32_t temperature = atoi(temp_str);
        uint32_t turbidity = atoi(turb_str);

        // Print the received values to the Serial Monitor
        Serial.print("Received tdsValue: ");
        Serial.println(tdsValue);
        Serial.print("Received phValue: ");
        Serial.println(phValue / 10.0, 1); // Divide phValue by 10 and print with one decimal
        Serial.print("Received TemperatureValue: ");
        Serial.println(temperature);
        Serial.print("Received TurbidityValue: ");
        Serial.println(turbidity / 10000.0, 4); // Divide turbidity by 10000 and print with four decimals
    }
    else
    {
        Serial.println("Failed to parse the RX message");
    }
}

void setup(void)
{
    Serial.begin(9600);
    pinMode(led, OUTPUT);
    digitalWrite(led, LOW);
    LoRaSerial.begin(9600);
    Serial.print("Serial2 LOCAL TEST\r\n");
    at_send_check_response("+AT: OK", 100, "AT\r\n");
    at_send_check_response("+MODE: TEST", 1000, "AT+MODE=TEST\r\n");
    at_send_check_response("+TEST: RXLRPKT", 5000, "AT+TEST=RXLRPKT\r\n");
    delay(200);
    digitalWrite(led, HIGH);
}

void loop(void)
{
    char cmd[128];
    // Transmit HEX Value
    sprintf(cmd, "");
    int ret = at_send_check_response("+TEST: RX", 1000, "");
    if (ret)
        recv_parse(recv_buf);
    else
        Serial.println("Receive failed!\r\n\r\n");
    delay(5000);
}







