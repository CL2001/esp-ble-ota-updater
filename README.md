# Bluetooth Over the air updater using react

## Compile the firmware after modifications
### First time set up
Download pipx and add to path
```bash
sudo apt install pipx
pipx ensurepath
```
Install plaform io
```bash
pipx install platformio
```

Open a new terminal and verify the install is successful.
You should see the PlatformIO Core version printed, e.g., 'PlatformIO Core, version 6.x.x'.
```bash
pio --version
```

### Compile
Compile the Firmware
Compile using the following command
```bash
pio run --project-dir ./firmware
```

### Flash firmware (first time)
An initial flash of the firmware must be made
```bash
pio run --project-dir ./firmware -t upload
```


## Python
### Set up env
```bash
poetry install --no-root
poetry shell
```


### Run python script
```bash
poetry run python local_uploader.py
```


### Reset (if needed)
Run bluetoothctl in your terminal.
Type remove 30:ED:A0:A8:AE:0D to clear the cache.



## Run webapp TO DO
### First time set up
```bash
npm install
```

### Run the web app
```bash
export NODE_OPTIONS=--openssl-legacy-provider
npm start
```

brave://flags/
Web Bluetooth API
Web Bluetooth

### Open web app
Open [http://localhost:3000](http://localhost:3000) to view it in the browser.



## Inspiration
Code was inspired and based on this following project. Credit goes to them:
https://learn.sparkfun.com/tutorials/esp32-ota-updates-over-ble-from-a-react-web-application/all