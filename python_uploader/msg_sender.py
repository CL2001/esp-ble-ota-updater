import asyncio
from bleak import BleakClient, BleakScanner

ESP32_NAME_SUBSTR = "ESP32"
MSG_CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"

async def connect_esp32(timeout: float = 10.0):
    devices = await BleakScanner.discover()
    esp32 = next((d for d in devices if ESP32_NAME_SUBSTR in (d.name or "")), None)

    if not esp32:
        raise RuntimeError("ESP32 not found")

    print(f"Found ESP32: {esp32.address}")

    client = BleakClient(esp32.address, timeout=timeout)
    await client.connect()
    print("Connected")
    return client

async def send_and_receive(client: BleakClient, message: str, response_future, timeout: float = 5.0) -> str:
    # Send message
    print(f"Sending: {message}")
    await client.write_gatt_char(MSG_CHAR_UUID, message.encode("utf-8"), response=True)

    # Wait for response
    try:
        response = await asyncio.wait_for(response_future, timeout)
    except asyncio.TimeoutError:
        response = None

    # Reset the future for the next message
    response_future.set_result(None) if not response_future.done() else response_future.cancel()
    return response

async def main():
    client = await connect_esp32()

    response_future = asyncio.get_event_loop().create_future()

    def notification_handler(_, data: bytearray):
        if not response_future.done():
            response_future.set_result(data.decode(errors="ignore"))

    try:
        # Start notifications **once**
        await client.start_notify(MSG_CHAR_UUID, notification_handler)

        messages = ["Hello ESP32", "How are you?", "Another message"]
        for msg in messages:
            # Reset future for each message
            response_future = asyncio.get_event_loop().create_future()
            reply = await send_and_receive(client, msg, response_future)
            if reply is None:
                print(f"No response for: {msg}")
            else:
                print(f"Response: {reply}")

    finally:
        await client.stop_notify(MSG_CHAR_UUID)
        await client.disconnect()
        print("Disconnected")

if __name__ == "__main__":
    asyncio.run(main())
