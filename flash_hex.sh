#!/usr/bin/env bash

# nrfjprog -e -f nrf52 
# https://devzone.nordicsemi.com/f/nordic-q-a/10134/flashing-softdevice-with-the-latest-nrfjprog
nrfjprog --program $1 --sectoranduicrerase -f nrf52 --reset --verify # e.g. particle_xenon_bootloader-0.4.0_s140_6.1.1.hex OR s140_nrf52_7.2.0_softdevice.hex
# nrfjprog --program $1 --chiperase -f nrf52 --reset --verify # This erases all chip, not sure what effect will happen for soft device etc.

# Soft devices are located here: https://github.com/NordicSemiconductor/nRF5-SDK-for-Mesh/tree/master/bin/softdevice
# nRF52832 - S132
# nRF52833/nRF52840 - S140