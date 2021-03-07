# MMITSS Driver Model for VISSIM Microscopic Simulation

To simulate connected vehicles in VISSIM simulation, the MMITSS driver-model allows vehicles using this driver-model to send their dynamic state information to a configured network node at every simulation timestep (10 Hz). The vehicle state information is packed in a Binary Large Object (Blob) which has the following structure:
| Syntax      | Description |
| ----------- | ----------- |
| Header      | Title       |
| Paragraph   | Text        |
