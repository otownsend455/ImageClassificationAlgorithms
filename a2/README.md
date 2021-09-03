This is the README for a2.

Code developed by Olivia Townsend:

- In classifier.c: lines 54 - 79
- In dectree.c: lines 31 - 81, 136 - 348 

How to run program (in terminal):
1. Unzip dataset: unzip datasets.zip
2. Compilation command: make
3. You’re ready to go.

To run a quick test with 1k training and 1k testing images: 
./classifier datasets/training_1000.bin datasets/testing_1000.bin

To run a full evaluation with all training and test images: 
./classifier datasets/training_data.bin datasets/testing_data.bin

Expected output will be the number of correct predictions.

Please view the datasets file for all the different testing and training image set sizes allowed. Enjoy!
