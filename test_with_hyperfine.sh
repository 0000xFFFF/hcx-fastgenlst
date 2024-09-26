#!/bin/bash -x

hyperfine \
    "./hcx-fastgenlst -s hello -lut123" \
    "./hcx-fastgenlst-o1 -s hello -lut123" \
    "./hcx-fastgenlst-o2 -s hello -lut123" \
    "./hcx-fastgenlst-o3 -s hello -lut123" \
