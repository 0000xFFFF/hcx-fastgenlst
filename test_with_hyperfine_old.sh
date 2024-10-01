#!/bin/bash -x

hyperfine \
    "./hcx-fastgenlst -s hello -lut123" \
    "./hcx-fastgenlst-old -s hello -lut123" \
