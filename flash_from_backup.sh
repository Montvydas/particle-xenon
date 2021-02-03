#!/usr/bin/env bash

nrfjprog -f NRF52 --program backupuicr.hex --chiperase
nrfjprog -f NRF52 --program backupcode.hex
