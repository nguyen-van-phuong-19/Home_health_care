import paho.mqtt.client as mqtt
from mqtt import mqtt_server


if __name__ == "__main__":
    """Main function to run the MQTT server"""
    # Setup Firebase
    root_ref = mqtt_server.setup_firebase()
    
    # Setup MQTT client
    client = mqtt.Client()
    client.user_data_set({"db": root_ref})
    client.on_connect = mqtt_server.on_connect
    client.on_message = mqtt_server.on_message
    
    # Connect to MQTT broker
    # In production, use secure connection parameters
    mqtt_broker = "broker.emqx.io"  # Replace with your MQTT broker address
    mqtt_port = 1883
    
    try:
        client.connect(mqtt_broker, mqtt_port, 60)
        print(f"Connected to MQTT broker at {mqtt_broker}:{mqtt_port}")
        
        # Start the MQTT loop
        client.loop_forever()
    except Exception as e:
        print(f"Error: {e}")
