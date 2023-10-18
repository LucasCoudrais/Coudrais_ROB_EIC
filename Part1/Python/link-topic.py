import paho.mqtt.client as mqtt

oldData = 2
countdown = 0

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("/I102/CoudraisGirardin/temperature")


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    data = msg.payload.decode()
    print(data)
    publish(client, data)
    

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
    print('SEND')
    
    
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.enable_bridge_mode()

client.connect("192.168.1.113", 1884, 60)

client.loop_forever()
