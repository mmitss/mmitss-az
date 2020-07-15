This directory provides tools to test the RSU4.1 interface.
1. 'sendMsgToSourceDevice.py' is a script that sends the specified message (stored in messages directory) to an IP and port specified in the script. Instructions on how to edit the script are provided in the beginning of the script.
2. 'receiveMsgsFromSinkDevice.py' is a script that receives messages forwarded from a sink device and stores in 'received_messages' directory. Instructions on how to use the script are provided in the beginning of the script.
3. 'messages' directory stores samples of messages that may be forwarded to the source dsrc device. These messeages are formatted as per the guidelines of RSU4.1 specifications. 
NOTE: For all messages, the psid is kept the same (0x8003) for the purpose of experiments and demo. During implementation, actual psids need to be defined.
4. 'received_messages' directory stores the messages received from the sink dsrc devices.
5. 
