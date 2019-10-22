#!/bin/sh
# Run All Model training shell
# 2019.10.21
# run make train first
echo "Make train"
make ./train
echo "Do training for $1 epochs..."
for ((model=1; model<=5 ; model=model+1))
do
    echo "Model $model"
    ./train $1 ./model_init.txt ./data/train_seq_0$model.txt ./model_0$model.txt
done

exit 0