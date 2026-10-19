# MQTT API Reference

## Configuring MQTT

MQTT broker settings can be configured two ways:

### 1. At first boot (WiFi setup portal)

When the device boots for the first time and creates the `WateringSystemAP` access point, the captive portal includes fields for:

- **MQTT Broker address**
- **MQTT Port**
- **MQTT Username**
- **MQTT Password**

Fill these in before saving your WiFi credentials.

### 2. Via the web settings page

Once the device is on your network, navigate to:

```
GET /settings
```

This serves an HTML form pre-populated with the current broker address, port, and username. Submit the form to update settings without rebooting.

```
POST /settings
```

Accepts the following form fields:

| Field | Type | Description |
|---|---|---|
| `mqtt_server` | string | Broker hostname or IP (max 253 chars, required) |
| `mqtt_port` | number | Broker port 1–65535 (required) |
| `mqtt_username` | string | Username (max 99 chars, optional) |
| `mqtt_password` | string | New password (max 99 chars) — leave blank to keep existing |
| `clear_password` | checkbox | If checked, clears the saved password |
| `mqtt_enabled` | checkbox | If checked, enables the MQTT connection |

Returns `202 Accepted` when the update has been queued (applied within a few seconds). Reload `/settings` to verify.

> **Note:** Credentials are submitted over plain HTTP. Only use this page on a trusted local network.

---

## MQTT Topics

### Check-in status

```
hunter/<device_hostname>/checkIn
```

Publishes: `online` / `rebooted` / `reconnected` / `Dead Somewhere`

### Action result

```
hunter/<device_hostname>/result
```

Publishes the same result JSON as the [HTTP API](api.md).

### Start / stop a zone

```
hunter/<device_hostname>/zone/<ZONE_NUMBER>
```

Payload (JSON):

```json
{ "action": "start", "time": "1" }
{ "action": "stop" }
```

### Start a program

```
hunter/<device_hostname>/program/<PROGRAM_NUMBER>
```

Payload (JSON):

```json
{ "action": "start" }
```
