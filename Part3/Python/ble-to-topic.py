import asyncio
import sys
from bleak import BleakClient
import paho.mqtt.client as mqtt
import time 

oldData = 2
countdown = 0

def publish(client,data):
    global oldData
    global countdown
    topic = '/I102/CoudraisGirardin/air_flow'
    if(float(data) < 118) :
        if(oldData != 0) : 
            oldData = 0
            client.publish(topic,0)
        else : 
            countdown = countdown + 1
            if(countdown >= 5) : 
                countdown = 0
                client.publish(topic,0)
    elif(float(data) > 138) :
        if(oldData != 1) : 
            oldData = 1
            client.publish(topic,1)
        else : 
            countdown = countdown + 1
            if(countdown >= 5) : 
                countdown = 0
                client.publish(topic,1)
    print(float(data))
    print('SEND')

  
# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))  
    


async def main(address, clientMQTT):
  async with BleakClient(address) as client:
    while True : 
        if (not client.is_connected):
            raise "client not connected"

        services = await client.get_services()

        for service in services:
            print('\nservice', service.handle, service.uuid, service.description)
            print(service)
            characteristics = service.characteristics

            for char in characteristics:
                print('  characteristic', char.handle, char.uuid, char.description, char.properties)
                
                descriptors = char.descriptors

                for desc in descriptors:
                    print('    descriptor', desc)

        
        data = await client.read_gatt_char(char.uuid)
        cnt = int.from_bytes(data, "little")

        print("\nPrint value of last characteristic : {val}".format(val=cnt))

        publish(clientMQTT, cnt)
        time.sleep(1)



if __name__ == "__main__":
  address = "24:62:AB:F5:02:D6"
  print('address:', address)
  client = mqtt.Client()
  client.on_connect = on_connect
  client.enable_bridge_mode()

  client.connect("192.168.1.39", 1884, 60)
  asyncio.run(main(address, client))




