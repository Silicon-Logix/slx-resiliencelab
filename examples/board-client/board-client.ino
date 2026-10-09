// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// Project board client implementation.

// USB CDC example for Arduino-compatible boards with at least 8 KiB RAM.
// Polls host measurements; the host or customer DLL still calculates motor drive.
#include <Arduino.h>
#include <string.h>
constexpr int requestButton = 2; // Button to ground requests 80% opening.
char reply[1536];
size_t received = 0;
bool waiting = false, dropping = false, queuedTarget = false, previousButton = false;
unsigned long sentAt = 0;
unsigned long requestId = 0;
void setup()
{
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(requestButton, INPUT_PULLUP);
}
void loop()
{
    const bool pressed = digitalRead(requestButton) == LOW;
    if (pressed && !previousButton)
        queuedTarget = true;
    previousButton = pressed;
    while (Serial.available())
    {
        const char value = char(Serial.read());
        if (value == '\n')
        {
            reply[received] = '\0';
            // Compact response matching is limited to the fields used by this example.
            // Product firmware should parse/validate the complete contract and response ID.
            if (!dropping && strstr(reply, "\"schema\":\"slx.bench.v1\"") && strstr(reply, "\"ok\":true") &&
                strstr(reply, "\"stopped\":"))
                digitalWrite(LED_BUILTIN, strstr(reply, "\"stopped\":true") ? HIGH : LOW);
            received = 0;
            dropping = false;
            waiting = false;
        }
        else if (!dropping)
        {
            if (received < sizeof(reply) - 1)
                reply[received++] = value;
            else
                dropping = true;
        }
    }
    if (waiting && millis() - sentAt > 1000)
    {
        waiting = false;
        received = 0;
        dropping = false;
        digitalWrite(LED_BUILTIN, HIGH);
    }
    if (Serial && !waiting && millis() - sentAt >= 200)
    {
        Serial.print("{\"id\":");
        Serial.print(++requestId);
        Serial.println(queuedTarget ? ",\"op\":\"target\",\"value\":80}" : ",\"op\":\"measurements\"}");
        queuedTarget = false;
        waiting = true;
        sentAt = millis();
    }
}
