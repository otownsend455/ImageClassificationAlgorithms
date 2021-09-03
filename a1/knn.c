#include <stdio.h>
#include <math.h>    // Need this for sqrt()
#include <stdlib.h>
#include <string.h>

#include "knn.h"

int k_global; // need for find_max function

/* Print the image to standard output in the pgmformat.  
 * (Use diff -w to compare the printed output to the original image)
 */
void print_image(unsigned char *image) {
    printf("P2\n %d %d\n 255\n", WIDTH, HEIGHT);
    for (int i = 0; i < NUM_PIXELS; i++) {
        printf("%d ", image[i]);
        if ((i + 1) % WIDTH == 0) {
            printf("\n");
        }
    }
}

/* Return the label from an image filename.
 * The image filenames all have the same format:
 *    <image number>-<label>.pgm
 * The label in the file name is the digit that the image represents
 */
unsigned char get_label(char *filename) {
    // Find the dash in the filename
    char *dash_char = strstr(filename, "-");
    // Convert the number after the dash into a integer
    return (char) atoi(dash_char + 1);
}

/* ******************************************************************
 * Complete the remaining functions below
 * ******************************************************************/


/* Read a pgm image from filename, storing its pixels
 * in the array img.
 * It is recommended to use fscanf to read the values from the file. First, 
 * consume the header information.  You should declare two temporary variables
 * to hold the width and height fields. This allows you to use the format string
 * "P2 %d %d 255 "
 * 
 * To read in each pixel value, use the format string "%hhu " which will store
 * the number it reads in an an unsigned character.
 * (Note that the img array is a 1D array of length WIDTH*HEIGHT.)
 */
void load_image(char *filename, unsigned char *img) {
    // Open corresponding image file, read in header (which we will discard)
    FILE *f2 = fopen(filename, "r");
    if (f2 == NULL) {
        perror("fopen");
        exit(1);
    }

    // parsing header
    int columns, rows;
    fscanf(f2, "P2 %d %d 255", &columns, &rows);

    // going through image body
    unsigned char pixel;
    int index = 0;
    while(fscanf(f2, "%hhu", &pixel) == 1){
        img[index] = pixel;
        index++;
    }

    fclose(f2);
}


/**
 * Load a full dataset into a 2D array called dataset.
 *
 * The image files to load are given in filename where
 * each image file name is on a separate line.
 * 
 * For each image i:
 *  - read the pixels into row i (using load_image)
 *  - set the image label in labels[i] (using get_label)
 * 
 * Return number of images read.
 */
int load_dataset(char *filename, 
                 unsigned char dataset[MAX_SIZE][NUM_PIXELS],
                 unsigned char *labels) {
    // We expect the file to hold up to MAX_SIZE number of file names
    FILE *f1 = fopen(filename, "r");
    if (f1 == NULL) {
        perror("fopen");
        exit(1);
    }

    int index = 0;
    char single_filename[40];
    while(fscanf(f1, "%s", single_filename) == 1){
        labels[index] = get_label(single_filename);
        load_image(single_filename, dataset[index]);
        index++;
    }

    fclose(f1);
    return index;
}

/** 
 * Return the euclidean distance between the image pixels in the image
 * a and b.  (See handout for the euclidean distance function)
 */
double distance(unsigned char *a, unsigned char *b) {

    int i ;
    int difference_sum = 0;
    for (i = 0; i < NUM_PIXELS; i++){
        int difference_exponent = pow(a[i] - b[i], 2);

        difference_sum = difference_sum + difference_exponent;
    }

    return sqrt(difference_sum);
}

int find_max(double k_most_similar_set[k_global][2]){
    int i;
    int index_largest = 0;

    for(i = 1; i < k_global; i++){
        if (k_most_similar_set[index_largest][1] < k_most_similar_set[i][1]) {
            index_largest = i;
        }
    }

    return index_largest;
}


/**
 * Return the most frequent label of the K most similar images to "input"
 * in the dataset
 * Parameters:
 *  - input - an array of NUM_PIXELS unsigned chars containing the image to test
 *  - K - an int that determines how many training images are in the 
 *        K-most-similar set
 *  - dataset - a 2D array containing the set of training images.
 *  - labels - the array of labels that correspond to the training dataset
 *  - training_size - the number of images that are actually stored in dataset
 * 
 * Steps
 *   (1) Find the K images in dataset that have the smallest distance to input
 *         When evaluating an image to decide whether it belongs in the set of 
 *         K closest images, it will only replace an image in the set if its
 *         distance to the test image is strictly less than all of the images in 
 *         the current K closest images.
 *   (2) Count the frequencies of the labels in the K images
 *   (3) Return the most frequent label of these K images
 *         In the case of a tie, return the smallest value digit.
 */ 

int knn_predict(unsigned char *input, int K,
                unsigned char dataset[MAX_SIZE][NUM_PIXELS],
                unsigned char *labels,
                int training_size) {

    double k_most_similar_set[K][2]; // type double because item_distance is.
    int largest_dist_index = 0; // index of item in k_most_similar_set with largest distance.
    double item_distance; // type double because distance func returns double.
    k_global = K;

    // arbitrarily add first K images
    int i; // index in training set
    for (i = 0; i < K; i++){
        item_distance = distance(input, dataset[i]);
        k_most_similar_set[i][0] = i;
        k_most_similar_set[i][1] = item_distance;
    }

    // identify index with largest distance
    largest_dist_index = find_max(k_most_similar_set);

    for (i = K; i < training_size; i++){
        item_distance = distance(input, dataset[i]);
        if (item_distance < k_most_similar_set[largest_dist_index][1]){
            // replace
            k_most_similar_set[largest_dist_index][0] = i;
            k_most_similar_set[largest_dist_index][1] = item_distance;
            largest_dist_index = find_max(k_most_similar_set);
        }
    }

    // count the frequencies of the labels in the K images
    int frequencies[10] = {0,0,0,0,0,0,0,0,0,0};

    int g;
    for(g = 0; g < K; g++){
        int dataset_index = k_most_similar_set[g][0];
        int label = labels[dataset_index];
        frequencies[label] = frequencies[label] + 1;
    }

    // find max frequency
    int most_frequent_label = 0;
    for (g = 1; g < 10; g++){
      if (frequencies[most_frequent_label] < frequencies[g]){
          most_frequent_label = g;
      }
      else if (frequencies[most_frequent_label] == frequencies[g]){
          // if tie, return smallest value digit
          if (most_frequent_label > g){
              most_frequent_label = g;
          }
      }
    }

    return most_frequent_label;
}
