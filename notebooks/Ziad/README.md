# Ziad's Notebooks

## Week 1-2: September 24-28, 2024
### API Setup and Initial Testing

**Date: 09/24/2024**

**Objective:** Get MTD API and validate data structure

**Tasks Completed:**
- Registered and Received API key, from MTD and completed verification process.
- Set up Insomnia REST client for API testing
- Tested endpoints:
  - `GetVehicles()` - Returns real-time bus locations
  - `getRoutes()` - Returns list of all bus routes with colors
  - `getShape()` - Returns GPS path points defining route shapes
 
<img width="756" height="689" alt="Screenshot 2025-09-28 at 3 54 33 PM" src="https://github.com/user-attachments/assets/df7dcad6-0a2c-4683-b217-5d3e5157887f" />

**Results:**
- Successfully retrieved JSON responses from all endpoints
- Confirmed `GetVehicles` returns ~15-25KB payload with active bus locations.

**API Response Structure:**
```json
{
  "vehicles": [
    {
      "vehicle_id": "...",
      "trip": {
        "route_id": "ILLINI",
        "direction": "East"
      },
      "location": {
        "lat": 40.1092,
        "lon": -88.2272
      }
    }
  ]
}
```

## Week 3-4: October 1-7, 2024
### ESP32 Integration and Breadboard Demo

**Date: 10/05/2024**

**Objective:** Connect ESP32-S3 to WiFi and establish HTTPS connection to MTD API
**Software Libraries:**
- `WiFi.h` - ESP32 WiFi connectivity
- `HTTPClient.h` - HTTP request handling
- `WiFiClientSecure.h` - HTTPS/TLS support
- `ArduinoJson.h`  - JSON parsing

1. **TLS Certificate Validation:** Initially failed HTTPS connection
 Used `client.setInsecure()` to bypass certificate validation (acceptable for read-only public API)
2. **Using IllinoisNet**
  - Failed to connect to IllinoisNet due to Captive Portal to log in to student account, opted to using phone hotspot.

![B54E72DD-15F0-4063-979C-1600B46E0ABB_1_105_c](https://github.com/user-attachments/assets/d0a41dbf-c6ee-4eb9-86be-93b8a833e3a3)



## Week 5-6: October 13-20, 2024
### GPS Mapping Algorithm and LED Integration
**Date: 10/13/2024**
**Objective:** Test basic LED control and display live bus data for Illini Union Bus Stop Mock Demo 2

- Connected to MTD API and fetched live vehicle data
- Filtered JSON response for Illini Union route buses only
- Extracted latitude and longitude for each bus
- Applied simple linear scaling to approximate LED positions, based on (176 LEDs) small LED strip we have
- Updated LEDs every 60 seconds with new API data
**JSON Parsing Code:**
```cpp
for (JsonObject vehicle : doc["vehicles"].as()) {
    const char* route_id = vehicle["trip"]["route_id"];
    
    // Filter for Illini Union only
    if (strcmp(route_id, "ILLINI") == 0) {
        double lat = vehicle["location"]["lat"];
        double lon = vehicle["location"]["lon"];
        
        // Basic position estimation (simplified)
        int led_position = estimatePosition(lat, lon);
        strip.setPixelColor(led_position, strip.Color(255, 255, 255)); 
    }
}
strip.show();
```

https://github.com/user-attachments/assets/0354b601-5e5d-42b3-8681-6bb49f9fb9fd






## Week 7-9: October 27 - November 2, 2024
**Task:** Get Machine Shop Stand

### Machine Shop Coordination

**Objective:** Coordinate with machine shop for custom display stand

**Items Received (11/02/2024):**
- Board to glue 3D printed segments of hte map on

Items from Machine Shop
![651D2E2C-DB3E-4684-9555-0176D12105F5_4_5005_c](https://github.com/user-attachments/assets/8f355cf8-3e9c-444e-8b34-a8c762bd1154)



## Week 10: November 3-9, 2024
**Task:** Finalize Code

### GPS Mapping Algorithm Development

**Objective:** Develop accurate GPS-to-LED coordinate transformation algorithm

**Problem Statement:**
Convert GPS coordinates (latitude/longitude) to discrete LED indices (0-873) representing exact physical positions on the map.

### Two-Step Algorithm

**Segment Identification**
- Defined 16 street segments with GPS bounding boxes
- Each segment has `minLat, maxLat, minLon, maxLat` boundaries

**Data Structure:**
```cpp
struct GPStoLED {
    const char* segment_name;
    std::vector ranges;
    double minLon, maxLon;
    double minLat, maxLat;
    bool reverse;
};
```

**Linear Interpolation**

For **East-West streets**:
```
t = (current_lon - minLon) / (maxLon - minLon)
LED = start_LED + (t × LED_range)
```

For **North-South streets**:
```
t = (maxLat - current_lat) / (maxLat - minLat)
LED = start_LED + (t × LED_range)
```

### Animation System (removed from final product)
```
// 40 FPS smooth motion
void animateBuses() {
    for (int i = 0; i < busCount; i++) {
        if (buses[i].current_led < buses[i].target_led) {
            buses[i].current_led++;
        } else if (buses[i].current_led > buses[i].target_led) {
            buses[i].current_led--;
        }
    }
}
```

