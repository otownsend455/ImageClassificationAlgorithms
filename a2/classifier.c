/*
 * This code is provided solely for the personal and private use of students
 * taking the CSC209H course at the University of Toronto. Copying for purposes
 * other than this use is expressly prohibited. All forms of distribution of
 * this code, including but not limited to public repositories on GitHub,
 * GitLab, Bitbucket, or any other online platform, whether as given or with
 * any changes, are expressly prohibited.
 *
 * Authors: Mustafa Quraish, Bianca Schroeder, Karen Reid
 *
 * All of the files in this directory and all subdirectories are:
 * Copyright (c) 2021 Karen Reid
 */

#include "dectree.h"

// Makefile included in starter:
//    To compile:               make
//    To decompress dataset:    make datasets
//
// Running decision tree generation / validation:
//    ./classifier datasets/training_data.bin datasets/testing_data.bin

/*****************************************************************************/
/* Do not add anything outside the main function here. Any core logic other  */
/* than what is described below should go in `dectree.c`. You've been warned!*/
/*****************************************************************************/

/**
 * main() takes in 2 command line arguments:
 *    - training_data: A binary file containing training image / label data
 *    - testing_data: A binary file containing testing image / label data
 *
 * You need to do the following:
 *    - Parse the command line arguments, call `load_dataset()` appropriately.
 *    - Call `make_dec_tree()` to build the decision tree with training data
 *    - For each test image, call `dec_tree_classify()` and compare the real 
 *        label with the predicted label
 *    - Print out (only) one integer to stdout representing the number of 
 *        test images that were correctly classified.
 *    - Free all the data dynamically allocated and exit.
 * 
 */
int main(int argc, char *argv[]) {
    int total_correct = 0;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s training_data testing_data\n", argv[0]);
        exit(1);
    }

    char* binary_train = argv[1];
    char* binary_test = argv[2];

    Dataset* training_dataset_ptr = load_dataset(binary_train);
    Dataset* testing_dataset_ptr = load_dataset(binary_test);

    DTNode* dec_tree_ptr = build_dec_tree(training_dataset_ptr);

    int num_test_images = (*testing_dataset_ptr).num_items;
    for (int i = 0; i < num_test_images; i++){
        Image curr_image = (*testing_dataset_ptr).images[i];
        int real_label = (*testing_dataset_ptr).labels[i];
        int predicted_label = dec_tree_classify(dec_tree_ptr, &curr_image);
        if (real_label == predicted_label){
            total_correct++;
        }
    }

    // Print out answer
    printf("%d\n", total_correct);

    // Free all dynamically allocated data
    free_dataset(training_dataset_ptr);
    free_dataset(testing_dataset_ptr);
    free_dec_tree(dec_tree_ptr);

    return 0;
}
