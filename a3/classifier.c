#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>      
#include <sys/types.h>  
#include <sys/wait.h>  
#include <string.h>
#include "knn.h"
#include <math.h>

/*****************************************************************************/
/* Do not add anything outside the main function here. Any core logic other  */
/*   than what is described below should go in `knn.c`. You've been warned!  */
/*****************************************************************************/

/**
 * main() takes in the following command line arguments.
 *   -K <num>:  K value for kNN (default is 1)
 *   -d <distance metric>: a string for the distance function to use
 *          euclidean or cosine (or initial substring such as "eucl", or "cos")
 *   -p <num_procs>: The number of processes to use to test images
 *   -v : If this argument is provided, then print additional debugging information
 *        (You are welcome to add print statements that only print with the verbose
 *         option.  We will not be running tests with -v )
 *   training_data: A binary file containing training image / label data
 *   testing_data: A binary file containing testing image / label data
 *   (Note that the first three "option" arguments (-K <num>, -d <distance metric>,
 *   and -p <num_procs>) may appear in any order, but the two dataset files must
 *   be the last two arguments.
 * 
 * You need to do the following:
 *   - Parse the command line arguments, call `load_dataset()` appropriately.
 *   - Create the pipes to communicate to and from children
 *   - Fork and create children, close ends of pipes as needed
 *   - All child processes should call `child_handler()`, and exit after.
 *   - Parent distributes the test dataset among children by writing:
 *        (1) start_idx: The index of the image the child should start at
 *        (2)    N:      Number of images to process (starting at start_idx)
 *     Each child should get at most N = ceil(test_set_size / num_procs) images
 *      (The last child might get fewer if the numbers don't divide perfectly)
 *   - Parent waits for children to exit, reads results through pipes and keeps
 *      the total sum.
 *   - Print out (only) one integer to stdout representing the number of test 
 *      images that were correctly classified by all children.
 *   - Free all the data allocated and exit.
 *   - Handle all relevant errors, exiting as appropriate and printing error message to stderr
 */


/**
 * @param num_items: represents number of items in testing dataset
 * @param num_procs: represents number of children processes
 * @return an array representing the image distribution over child processes
 */
int* get_image_distribution(int num_items, int num_procs){

    int min_test_images = floor(num_items/num_procs);
    int leftover_images = num_items - (min_test_images * num_procs);
    int* image_distribution = malloc(sizeof(int) * num_procs);

    // every process gets at least min_test_images
    for (int i = 0; i < num_procs; i++){
        image_distribution[i] = min_test_images;
    }

    while (leftover_images > 0){
        for (int i = 0; i < num_procs; i++){
            image_distribution[i] = image_distribution[i] + 1;
            leftover_images--;
            if (leftover_images == 0){
                break;
            }
        }
    }

    return image_distribution;
}



void usage(char *name) {
    fprintf(stderr, "Usage: %s -v -K <num> -d <distance metric> -p <num_procs> training_list testing_list\n", name);
}

int main(int argc, char *argv[]) {

    int opt;
    int K = 1;             // default value for K
    char *dist_metric = "euclidean"; // default distant metric
    int num_procs = 1;     // default number of children to create
    int verbose = 0;       // if verbose is 1, print extra debugging statements
    int total_correct = 0; // Number of correct predictions
    double (*fptr)(Image*, Image*); // function pointer

    while((opt = getopt(argc, argv, "vK:d:p:")) != -1) {
        switch(opt) {
        case 'v':
            verbose = 1;
            break;
        case 'K':
            K = atoi(optarg);
            break;
        case 'd':
            dist_metric = optarg;
            break;
        case 'p':
            num_procs = atoi(optarg);
            break;
        default:
            usage(argv[0]);
            exit(1);
        }
    }

    if(optind >= argc) {
        fprintf(stderr, "Expecting training images file and test images file\n");
        exit(1);
    } 

    char *training_file = argv[optind];
    optind++;
    char *testing_file = argv[optind];

    // Set which distance function to use
    if (strncmp(dist_metric, "euclidean", strlen(dist_metric)) == 0){
        fptr = distance_euclidean;
    }
    else if(strncmp(dist_metric, "cosine", strlen(dist_metric)) == 0){
        fptr = distance_cosine;
    }
    else{
        fprintf(stderr, "Valid functions: euclidean, eucl, cosine, or cos\n");
        exit(1);
    }


    // Load data sets
    if(verbose) {
        fprintf(stderr,"- Loading datasets...\n");
    }
    
    Dataset *training = load_dataset(training_file);
    if ( training == NULL ) {
        fprintf(stderr, "The data set in %s could not be loaded\n", training_file);
        exit(1);
    }

    Dataset *testing = load_dataset(testing_file);
    if ( testing == NULL ) {
        fprintf(stderr, "The data set in %s could not be loaded\n", testing_file);
        exit(1);
    }

    // Create the pipes and child processes who will then call child_handler
    if(verbose) {
        printf("- Creating children ...\n");
    }

    // Used in parent > 0 code block
    int* image_distribution = get_image_distribution((*testing).num_items, num_procs);
    int img_distribution_index = 0;
    int index = 0;

    // create two pipes for every children
    // Distribute the work to the children by writing their starting index and
    // the number of test images to process to their write pipe
    int pipe_fd[num_procs * 2][2];

    for (int i = 0; i < num_procs*2; i+=2){
        if (pipe(pipe_fd[i]) == -1){
            perror("pipe");
            exit(1);
        }

        if (pipe(pipe_fd[i+1]) == -1){
            perror("pipe");
            exit(1);
        }

        // child starts here
        int result = fork();

        if(result == 0){
            // close writing end of pipe_fd[i]. only for reading.
            if(close(pipe_fd[i][1]) == -1){
                perror("close");
                exit(1);
            }

            // close reading end of pipe_fd[i+1]. only for writing.
            if(close(pipe_fd[i+1][0]) == -1){
                perror("close");
                exit(1);
            }

            // child inherits file descriptors of parent. close the un-needed ones.
            for (int pipe_no = 0; pipe_no < i; pipe_no+=2){
                close(pipe_fd[pipe_no+1][0]);
            }


            child_handler(training, testing, K, fptr, pipe_fd[i][0], pipe_fd[i+1][1]);

            free_dataset(training);
            free_dataset(testing);

            exit(0);
        }
        else if(result > 0){ // parent ends up here

            // close reading end of pipe_fd[i]
            if (close(pipe_fd[i][0]) == -1){
                perror("close");
                exit(1);
            }

            int arr_write[2];
            if (index < (*testing).num_items){

                arr_write[0] = index;
                arr_write[1] = image_distribution[img_distribution_index];
                index = index + image_distribution[img_distribution_index];
                img_distribution_index++;


                // write
                if (write(pipe_fd[i][1], arr_write, sizeof(int)*2) == -1){
                    perror("write");
                    exit(1);
                }

                // close write
                if (close(pipe_fd[i][1]) == -1){
                    perror("close");
                    exit(1);
                }
            }
            else{
                fprintf(stderr, "invalid index");
                exit(1);
            }
        }
        else{
            perror("fork");
            exit(1);
        }

    }


    // Wait for children to finish
    if(verbose) {
        printf("- Waiting for children...\n");
    }


    // When the children have finished, read their results from their pipe

    // Ensure children terminated normally
    for (int i = 0; i < num_procs; i++){
        int status;
        if (wait(&status) == -1) {
            perror("wait");
            exit(1);
        }
    }

    // Read results from pipe
    for (int j = 0; j < num_procs * 2; j += 2){
        int correct;
        int read_pipe = read(pipe_fd[j+1][0], &correct, sizeof(int));
        if (read_pipe > 0){
            total_correct += correct;
        }
        else if (read_pipe == 0){
            fprintf(stderr, "No bytes read");
            exit(1);
        }
        else{
            perror("read");
            exit(1);
        }

        // close reading end of pipe_fd[j+1]
        if (close(pipe_fd[j+1][0]) == -1){
            perror("close");
            exit(1);
        }
    }


    if(verbose) {
        printf("Number of correct predictions: %d\n", total_correct);
    }

    // This is the only print statement that can occur outside the verbose check
    printf("%d\n", total_correct);

    // Clean up any memory, open files, or open pipes
    // Note children datasets have already been freed at this point
    free(image_distribution);
    free_dataset(testing);
    free_dataset(training);


    return 0;
}



