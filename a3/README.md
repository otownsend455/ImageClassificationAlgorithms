This is the README for a3.

Code developed by Olivia Townsend:

- In classifier.c: lines 54 - 76, 123 - 306
- In knn.c: lines 138 - 247

How to run program (in terminal):
1. Unzip dataset: unzip databases.zip
2. Compilation command: make
3. You’re ready to go.

To run a quick test with 1k training and 1k testing images, with 3 nearest neighbours, 8 processes, with euclidean distance function: 
./classifier -K 3 -d eucl -p 8 -v datasets/training_1000.bin datasets/testing_1000.bin

To run a full evaluation with all training and test image, with 3 nearest neighbours, and 8 processes (will take awhile): 
./classifier -K 3 -d eucl -p 8 -v datasets/training_data.bin datasets/testing_data.bin

Please view the datasets file for all the different testing and training image set sizes allowed. You may also adjust the number of nearest neighbours and number of processes. Can also switch to use cosine function by replacing the eucl argument with cos. Enjoy!
