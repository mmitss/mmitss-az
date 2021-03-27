# Structure of the Simulated BSM Blob
| **Bytes** | **Data** | **Data Type** | **Description** |
| --- | --- | --- | --- |
| 0-3 | Message Count | uint_32 | Cumulative count of messages from the start of simulation |
| 4-11 | Temporary Id | uint_64 | Vehicle's unique identifier |
| 12-13 | SecMark | uint_16 | SecMark |
| 14-17 | Latitude | int_32 | 10000000 * Latitude in decimal degree |
| 18-21 | Longitude | int_32 | 10000000 * Longitude in decimal degree |
| 22-25 | Elevation | int_32 | 100 * Elevation in meter |
| 26-27 | Speed | uint_16 | 100 * Speed in meter per second |
| 28-29 | Heading | uint_16 | 100 * Heading in degrees from North (clockwise) |
| 30-31 | Length | uint_16 | 10 * Length in centimeter |
| 32-33 | Width | uint_16 | 10 * Width in centimeter |
| 34 | Vehicle Type | uint_8 | 0:"unknown", 2: "EmergencyVehicle", 4: "Car", 6: "Transit", 9: "Truck" |
