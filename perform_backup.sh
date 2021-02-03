#!/usr/bin/env bash

nrfjprog -f NRF52 --readcode backupcode.hex
nrfjprog -f NRF52 --readuicr backupuicr.hex
