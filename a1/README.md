This is the README for a1.

Code developed by Olivia Townsend:

   - In classifier.c: lines 81 - 110
   - In knn.c: lines 39 - 220

How to run program (in terminal): 

   1. Compilation command: gcc -Wall -std=c99 -lm -o classifier classifier.c knn.c
   2. Decompress dataset into individual images: tar xvzf datasets.tgz
   3. You’re ready to go. 

   To run a quick test with 1k training and 1k testing images, with 2 nearest neighbours: ./classifier 2 lists training_1k.txt lists/testing_1k.txt

   To run a full evaluation with all images, with 7 nearest neighbours (Will take a while): ./classifier 7 lists/training_full.txt lists/testing_full.txt
   
   Expected output will be the number of correct predictions. 
  
   Please view the datasets file for all the different testing and training image set sizes allowed. You may also adjust the number of nearest neighbours. Enjoy!
 
