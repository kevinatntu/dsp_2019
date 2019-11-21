#!/bin/bash

bash ./clean_auto.sh y
bash ./1-preprocess.sh
bash ./2-extract-feat.sh
bash ./3-train.sh
bash ./4-test.sh