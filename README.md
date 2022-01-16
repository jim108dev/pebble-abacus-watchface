# Pebble Abacus Watch

This is a learning project in order to create a Pebble app which displays date and time with [abacus/soroban](https://en.wikipedia.org/wiki/Soroban) beads.

## Install

1. Install repository 's software

    ```sh
    git clone https://github.com/jim108dev/pebble-abacus-watchface.git
    ```

1. Install Pebble: See [Native SDK install on Linux](https://willow.systems/blog/pebble-sdk-installation-guide/).

1. Run on the emulator

    ```sh
    cd pebble_app

    pebble build && pebble install --logs --emulator aplite
    ```

1. Run on the watch

    ```sh
    cd pebble_app

    sudo rfcomm bind 0 <pebble bluetooth id> 1
    pebble build && pebble install --serial /dev/rfcomm0
    ```
