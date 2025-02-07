import paho.mqtt.client as mqtt

# MQTT broker settings
BROKER = "broker.emqx.io"
PORT = 1883
KEEPALIVE = 60
USERNAME = "emqx"
PASSWORD = "public"

# Callback when connecting to broker
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    # Subscribe to topics after connecting
    client.subscribe("test/topic")

# Callback when receiving message
def on_message(client, userdata, msg):
    print(f"Received message '{msg.payload.decode()}' on topic '{msg.topic}'")

# Create MQTT client instance
client = mqtt.Client()

# Set callbacks
client.on_connect = on_connect
client.on_message = on_message

# Set username and password
client.username_pw_set(USERNAME, PASSWORD)

# Connect to broker
client.connect(BROKER, PORT, KEEPALIVE)

# Start network loop
client.loop_start()

# Example publish
client.publish("test/topic", "Hello EMQX!")
