import asyncio
import sys
from bleak.exc import BleakDBusError    
from bleak import BleakClient
import paho.mqtt.client as mqtt
import time 

def publish(client,data,uuid):
    if uuid.find("2a1e") != -1:
        topic = '/I102/CoudraisGirardin/data'
        client.publish(topic,data)
    if uuid.find("2a1c") != -1:
        topic = '/I102/CoudraisGirardin/temp'
        client.publish(topic,data)
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
            characteristics = service.characteristics

            for char in characteristics:
                try:
                    data = await client.read_gatt_char(char.uuid)
                    cnt = int.from_bytes(data, "little")

                    print("\nValue of last characteristic : {val}".format(val=cnt))

                    publish(clientMQTT, cnt, char.uuid)
                except BleakDBusError:
                    print("error")
            time.sleep(1)   


if __name__ == "__main__":
  address = "24:62:AB:F4:F5:32"
  print('address:', address)
  client = mqtt.Client()
  client.on_connect = on_connect
  client.enable_bridge_mode()

  client.connect("192.168.1.66", 1884, 60)
  asyncio.run(main(address, client))




