import asyncio
from bleak import BleakClient, BleakScanner
import os


ESP32_UUID = "d804b643-6ce7-4e81-9f8a-ce0f699085eb"
OTA_SERVICE_UUID = "c8659210-af91-4ad3-a995-a58d6fd26145"
VERSION_CHAR_UUID = "c8659212-af91-4ad3-a995-a58d6fd26145"
FILE_CHAR_UUID = "c8659211-af91-4ad3-a995-a58d6fd26145"

async def _main(firmware_path):
    devices = await BleakScanner.discover()
    esp32 = next((d for d in devices if "ESP32" in (d.name or "")), None)

    if not esp32:
        print("ESP32 not found")
        return

    print(f"Found ESP32 at {esp32.address}")
    await asyncio.sleep(1)  # wait a moment before connecting

    async with BleakClient(esp32.address, timeout=20.0) as client:
        print("Connected")
        services = client.services
        for service in services:
            print(f"Service: {service.uuid}")

        # read version
        version = await client.read_gatt_char(VERSION_CHAR_UUID)
        print("Version bytes:", list(version))

        # load firmware
        with open(firmware_path, "rb") as f:
            data = f.read()
        mtu = client.mtu_size - 3
        print("MTU from esp, to do to try and find why its not 512")
        CHUNK_SIZE = 512
        total_bytes = len(data)
        total_chunks = (total_bytes + CHUNK_SIZE - 1) // CHUNK_SIZE
        print(f"Firmware size: {total_bytes} bytes")
        print(f"Sending {total_chunks} chunks (Chunk size: {CHUNK_SIZE})...")

        for i in range(0, total_bytes, CHUNK_SIZE):
            chunk = data[i:i+CHUNK_SIZE]
            chunk_number = (i // CHUNK_SIZE) + 1
            await client.write_gatt_char(FILE_CHAR_UUID, chunk, response=True)
            print(f"Sent chunk ({chunk_number} / {total_chunks})", end="\r")

        print("\nUpload Complete!")

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    firmware_path = os.path.join(script_dir, "..", "firmware", ".pio", "build", "esp32s3-wroom-1-n16r8", "firmware.bin")
    if not os.path.exists(firmware_path):
        print("Error: Firmware file not found!")
        exit(-1)
    asyncio.run(_main(firmware_path))

main()
