import asyncio
from bleak import BleakClient, BleakScanner
import aiohttp


ESP32_UUID = "d804b643-6ce7-4e81-9f8a-ce0f699085eb"
OTA_SERVICE_UUID = "c8659210-af91-4ad3-a995-a58d6fd26145"
VERSION_CHAR_UUID = "c8659212-af91-4ad3-a995-a58d6fd26145"
FILE_CHAR_UUID = "c8659211-af91-4ad3-a995-a58d6fd26145"

# 👇 Replace this with your GitHub release asset URL
FIRMWARE_URL = "https://github.com/Interactive-Chessboard/ICB-Embedded/releases/latest/download/icb-embedded-firmware.bin"


async def download_firmware(url: str) -> bytes:
    print(f"Downloading firmware from:\n{url}")
    async with aiohttp.ClientSession() as session:
        async with session.get(url) as response:
            response.raise_for_status()
            data = await response.read()
            print(f"Downloaded {len(data)} bytes")
            return data


async def _main():
    # download firmware
    firmware_data = await download_firmware(FIRMWARE_URL)

    devices = await BleakScanner.discover()
    esp32 = next((d for d in devices if "ESP32" in (d.name or "")), None)

    if not esp32:
        print("ESP32 not found")
        return

    print(f"Found ESP32 at {esp32.address}")
    await asyncio.sleep(1)

    async with BleakClient(esp32.address, timeout=20.0) as client:
        print("Connected")

        services = client.services
        for service in services:
            print(f"Service: {service.uuid}")

        # read version
        version = await client.read_gatt_char(VERSION_CHAR_UUID)
        print("Version bytes:", list(version))

        mtu = client.mtu_size - 3
        print(f"MTU size: {client.mtu_size}")

        CHUNK_SIZE = 512
        total_bytes = len(firmware_data)
        total_chunks = (total_bytes + CHUNK_SIZE - 1) // CHUNK_SIZE

        print(f"Firmware size: {total_bytes} bytes")
        print(f"Sending {total_chunks} chunks (Chunk size: {CHUNK_SIZE})...")

        for i in range(0, total_bytes, CHUNK_SIZE):
            chunk = firmware_data[i:i + CHUNK_SIZE]
            chunk_number = (i // CHUNK_SIZE) + 1
            await client.write_gatt_char(FILE_CHAR_UUID, chunk, response=True)
            print(f"Sent chunk ({chunk_number} / {total_chunks})", end="\r")

        print("\nUpload Complete!")


def main():
    asyncio.run(_main())


main()
