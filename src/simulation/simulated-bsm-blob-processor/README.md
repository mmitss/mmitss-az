# Simulated BSM Blob Processor
In the MMITSS simulation environment, simulated vehicles using the [MMITSS Driver Model](./../driver-model) pack their dynamic state information in a BLOB and send the BLOB to configured network node. The structure of the BLOB is defined [here](./../README.md)
Simulated-Bsm-Blob-Processor component unpacks these BLOBs and constructs the corresponding JSON strings with `MsgType="BSM"`. These JSON strings are forwarded to [Message-Distributor](./../../server/message-distributor) component for further processing and distribution. 
