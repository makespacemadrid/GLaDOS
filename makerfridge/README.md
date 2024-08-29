# MakerFridge

A fridge made by makers for makers.

## Operation

The basic mode of operation is to fill the fridge with cans and close it.
It should dispense products when firmly pressing the buttons.

You can optionally set the stock of the machine via MQTT by posting a JSON message to the topic `smartfridge/set-stock`.

```json
{
  "stats": {
    "p0_stock": 8,
    "p1_stock": 8,
    "p2_stock": 8,
    "p3_stock": 8,
    "p4_stock": 8
  }
}
```

After the stock is set, you will be able to monitor the current state of the stock of the machine as well as the initial IP given to it on WiFi connection at topic `smartfridge/current-stock`.

When a can is deliveres, the value of the stock will be updated and submitted over the MQTT queue.

## SecDevOps

### Credentials management

**Before adding changes, call `scripts/rm_credentials.sh` to remove credentials in `platformio.ino`, and when your commit is done you can restore them calling `scripts/set_credentials.sh` from the variables set in your environment.**

Credentials are stored in a `.env` file similar to `env.example` in your local repo that should never be uploaded to version control.

However, we need to put them in the `platformio.ino` file in order to use them during builds.
