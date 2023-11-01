import serial
import paho.mqtt.client as mqtt

ser = serial.Serial('/dev/ttyACM0', 115200)

# ThingsBoard MQTT broker settings
THINGSBOARD_HOST = "mqtt.thingsboard.cloud"
ACCESS_TOKEN = "az3ddvh4mflz5yny54ia"
TOPIC = "v1/devices/me/telemetry"

client = mqtt.Client()
client.username_pw_set(ACCESS_TOKEN)
client.connect(THINGSBOARD_HOST, 1883, 60)

while True:
    if ser.in_waiting > 0:
        line = ser.readline().decode('utf-8').strip()
        parts = line.split(',')[:-1]
        data = {}
        if len(parts) > 3:
            for part in parts:
                key, value = part.split(': ')
                data[key] = float(value)
            data["V_BAT"] = data["V_BAT"]/9 * 100 #Change range to 0-100
            print(data)
            # Send data to ThingsBoard
            client.publish(TOPIC, str(data))
