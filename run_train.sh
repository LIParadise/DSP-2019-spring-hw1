(trap 'kill 0' SIGINT;
./train 100 model_init.txt seq_model_01.txt model_01.txt &
P1=$!
./train 100 model_init.txt seq_model_02.txt model_02.txt &
P2=$!
./train 100 model_init.txt seq_model_03.txt model_03.txt &
P3=$!
./train 100 model_init.txt seq_model_04.txt model_04.txt &
P4=$!
./train 100 model_init.txt seq_model_05.txt model_05.txt &
P5=$!
wait $P1 $P2 $P3 $P4 $P5
)
./test modellist.txt testing_data1.txt result1.txt
