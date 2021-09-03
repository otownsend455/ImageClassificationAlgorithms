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

/**
 * Load the binary file, filename into a Dataset and return a pointer to 
 * the Dataset. The binary file format is as follows:
 *
 *     -   4 bytes : `N`: Number of images / labels in the file
 *     -   1 byte  : Image 1 label
 *     - NUM_PIXELS bytes : Image 1 data (WIDTHxWIDTH)
 *          ...
 *     -   1 byte  : Image N label
 *     - NUM_PIXELS bytes : Image N data (WIDTHxWIDTH)
 *
 * You can set the `sx` and `sy` values for all the images to WIDTH. 
 * Use the NUM_PIXELS and WIDTH constants defined in dectree.h
 */
Dataset *load_dataset(const char *filename) {
    // TODO: Allocate data, read image data / labels, return

    FILE *f1 = fopen(filename, "rb");
    if (f1 == NULL) {
        perror("Error: could not open file\n");
        exit(1);
    }

    Dataset* dataset = malloc(sizeof(Dataset));
    int num_files;
    fread(&num_files, sizeof(int), 1, f1);
    (*dataset).num_items = num_files;
    (*dataset).images = malloc(sizeof(Image)*num_files);
    (*dataset).labels = malloc(sizeof(unsigned char)*num_files);

    // loop through f1 collecting image and label corresponding to image.
    int reading = 1;
    int i = 0;
    while(reading == 1){

        /**
         * READ LABEL
         */
        unsigned char label;
        if (fread(&label, sizeof(unsigned char), 1, f1) != 0){
            (*dataset).labels[i] = label;
        }
        else{ // EOF reached
            reading = 0;
            continue;
        }

        /**
        * READ IMAGE
        */
        (*dataset).images[i].sx = WIDTH;
        (*dataset).images[i].sy = WIDTH;
        (*dataset).images[i].data = malloc(sizeof(unsigned char) * NUM_PIXELS);

        for (int j = 0; j < NUM_PIXELS; j++){
            unsigned char pixel;
            fread(&pixel, sizeof(unsigned char), 1, f1);
            (*dataset).images[i].data[j] = pixel;
        }

        i++; // go to the next image in f1
    }

    return dataset;
}

/**
 * Compute and return the Gini impurity of M images at a given pixel
 * The M images to analyze are identified by the indices array. The M
 * elements of the indices array are indices into data.
 * This is the objective function that you will use to identify the best 
 * pixel on which to split the dataset when building the decision tree.
 *
 * Note that the gini_impurity implemented here can evaluate to NAN 
 * (Not A Number) and will return that value. Your implementation of the 
 * decision trees should ensure that a pixel whose gini_impurity evaluates 
 * to NAN is not used to split the data.  (see find_best_split)
 * 
 * DO NOT CHANGE THIS FUNCTION; It is already implemented for you.
 */
double gini_impurity(Dataset *data, int M, int *indices, int pixel) {
    int a_freq[10] = {0}, a_count = 0;
    int b_freq[10] = {0}, b_count = 0;

    for (int i = 0; i < M; i++) {
        int img_idx = indices[i];

        // The pixels are always either 0 or 255, but using < 128 for generality.
        if (data->images[img_idx].data[pixel] < 128) {
            a_freq[data->labels[img_idx]]++;
            a_count++;
        } else {
            b_freq[data->labels[img_idx]]++;
            b_count++;
        }
    }

    double a_gini = 0, b_gini = 0;
    for (int i = 0; i < 10; i++) {
        double a_i = ((double)a_freq[i]) / ((double)a_count);
        double b_i = ((double)b_freq[i]) / ((double)b_count);
        a_gini += a_i * (1 - a_i);
        b_gini += b_i * (1 - b_i);
    }

    // Weighted average of gini impurity of children
    return (a_gini * a_count + b_gini * b_count) / M;
}

/**
 * Given a subset of M images and the array of their corresponding indices, 
 * find and use the last two parameters (label and freq) to store the most
 * frequent label in the set and its frequency.
 *
 * - The most frequent label (between 0 and 9) will be stored in `*label`
 * - The frequency of this label within the subset will be stored in `*freq`
 * 
 * If multiple labels have the same maximal frequency, return the smallest one.
 */
void get_most_frequent(Dataset *data, int M, int *indices, int *label, int *freq) {
    // TODO: Set the correct values and return

    int frequencies[10] = {0,0,0,0,0,0,0,0,0,0};
    for(int i = 0; i < M; i++){
        int img_index = indices[i];
        unsigned char img_label = (*data).labels[img_index];
        frequencies[img_label] = frequencies[img_label] + 1;
    }

    // find max frequency
    int most_frequent_label = 0;
    for (int g = 0; g < 10; g++){
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

    *label = most_frequent_label;
    *freq = frequencies[most_frequent_label];

    return;
}

/**
 * Given a subset of M images as defined by their indices, find and return
 * the best pixel to split the data. The best pixel is the one which
 * has the minimum Gini impurity as computed by `gini_impurity()` and 
 * is not NAN. (See handout for more information)
 * 
 * The return value will be a number between 0-783 (inclusive), representing
 *  the pixel the M images should be split based on.
 * 
 * If multiple pixels have the same minimal Gini impurity, return the smallest.
 */
int find_best_split(Dataset *data, int M, int *indices) {
    // TODO: Return the correct pixel

    int split_pixel = 0;
    double split_pixel_gini = gini_impurity(data, M, indices, 0);

    for (int i = 1; i < NUM_PIXELS; i++){
        double curr_gini = gini_impurity(data, M, indices, i);
        // second OR-condition accounts for the first pixel returning nan, because all images at this pixel is 0
        if (curr_gini < split_pixel_gini || (isnan(split_pixel_gini) && !isnan(curr_gini))){
            split_pixel_gini = curr_gini;
            split_pixel = i;
        }
    }

    return split_pixel;
}

/**
 * Create the Decision tree. In each recursive call, we consider the subset of the
 * dataset that correspond to the new node. To represent the subset, we pass 
 * an array of indices of these images in the subset of the dataset, along with 
 * its length M. Be careful to allocate this indices array for any recursive 
 * calls made, and free it when you no longer need the array. In this function,
 * you need to:
 *
 *    - Compute ratio of most frequent image in indices, do not split if the
 *      ration is greater than THRESHOLD_RATIO
 *    - Find the best pixel to split on using `find_best_split`
 *    - Split the data based on whether pixel is less than 128, allocate 
 *      arrays of indices of training images and populate them with the 
 *      subset of indices from M that correspond to which side of the split
 *      they are on
 *    - Allocate a new node, set the correct values and return
 *       - If it is a leaf node set `classification`, and both children = NULL.
 *       - Otherwise, set `pixel` and `left`/`right` nodes 
 *         (using build_subtree recursively). 
 */
DTNode *build_subtree(Dataset *data, int M, int *indices) {
    // TODO: Construct and return the tree
    DTNode* subtree = malloc(sizeof(DTNode));
    int label;
    int freq;
    get_most_frequent(data, M, indices, &label, &freq);
    float ratio = freq/(float)M;


    if (ratio <= THRESHOLD_RATIO){
        int split_pixel = find_best_split(data, M, indices);
        (*subtree).pixel = split_pixel;
        int left_size = 0;
        int right_size = 0;
        int* temp_left_indices = malloc(sizeof(int)*M);
        int* temp_right_indices = malloc(sizeof(int)*M);

        for (int i = 0; i < M; i++){
            int img_index = indices[i]; // get corresp. img index
            Image img = (*data).images[img_index]; // get the image at index
            if (img.data[split_pixel] < 128){
                temp_left_indices[left_size] = img_index; // place image index at correct loc
                left_size++;
            }
            else{
                temp_right_indices[right_size] = img_index;
                right_size++;
            }
        }

        int* left_indices = malloc(sizeof(int)*left_size);
        for (int j = 0; j < left_size; j++){
            left_indices[j] = temp_left_indices[j];
        }

        int* right_indices = malloc(sizeof(int)*right_size);
        for (int j = 0; j < right_size; j++){
            right_indices[j] = temp_right_indices[j];
        }

        free(temp_left_indices);
        free(temp_right_indices);

        (*subtree).classification = -1; // as specified in dec_tree.pdf
        (*subtree).left = build_subtree(data, left_size, left_indices);
        (*subtree).right = build_subtree(data, right_size, right_indices);

        free(left_indices);
        free(right_indices);
    }
    else{ // Is a leaf node
        (*subtree).classification = label;
        (*subtree).pixel = -1; // as specified in dec_tree.pdf
        (*subtree).left = NULL;
        (*subtree).right = NULL;
    }

    return subtree;

}




/**
 * This is the function exposed to the user. All you should do here is set
 * up the `indices` array correctly for the entire dataset and call 
 * `build_subtree()` with the correct parameters.
 */
DTNode *build_dec_tree(Dataset *data) {
    // HINT: Make sure you free any data that is not needed anymore
    int size_img_arr = (*data).num_items;
    int indices[size_img_arr];

    for (int i = 0; i < size_img_arr; i++){
        indices[i] = i;
    }

    DTNode* root = build_subtree(data, size_img_arr, indices);

    return root;
}

/**
 * Given a decision tree and an image to classify, return the predicted label.
 */
int dec_tree_classify(DTNode *root, Image *img) {
    int curr_pixel = (*root).pixel;
    int classification;

    if((*root).left == NULL && (*root).right == NULL){ // at leaf
        classification = (*root).classification;
    }
    else if ((*img).data[curr_pixel] < 128){ // go to left subtree
        classification = dec_tree_classify((*root).left, img);
    }
    else{ // go to right subtree
        classification = dec_tree_classify((*root).right, img);
    }

    return classification;
}

/**
 * This function frees the Decision tree.
 */
void free_dec_tree(DTNode *node) {
    if ((*node).left == NULL && (*node).right == NULL){
        free(node);
    }
    else{
        free_dec_tree((*node).left);
        free_dec_tree((*node).right);
        free(node);
    }

    return;
}


/**
 * Free all the allocated memory for the dataset
 */
void free_dataset(Dataset *data) {
    free((*data).labels);
    int num_images = (*data).num_items;
    for (int i = 0; i < num_images; i++){
        free((*data).images[i].data);
    }
    free((*data).images);
    free(data);

    return;
}
