This is the README for A1.

Code developed by Olivia Townsend:

   In classifier.c: lines 81 - 110
   In knn.c: lines 39 - 220

How to run program: 

   1. Compilation command: gcc -Wall -std=c99 -lm -o classifier classifier.c knn.c
   2. Decompress dataset into individual images: tar xvzf datasets.tgz
   3. You’re ready to go. 

   To run a quick test with 1k training and 1k testing images, K = 1:     
   ./classifier 1 lists training_1k.txt lists/testing_1k.txt

   To run a full evaluation with all images, K = 7 (Will take a while):
   ./classifier 7 lists/training_full.txt lists/testing_full.txt

   Play around with the value of K if you wish. Enjoy!
 
