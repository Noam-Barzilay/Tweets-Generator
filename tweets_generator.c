#include "markov_chain.h"
#define MAX_LINE_LENGTH 1000
#define MIN_ARGS_NUM 4
#define MAX_ARGS_NUM 5
#define MAX_WORDS_IN_TWEET 20
#define MAX_WORD_LENGTH 100


int fill_database_all_file(FILE *fp, MarkovChain *markov_chain){
    bool beginning_of_line = true;
    int cur_num_of_words = 0;
    char buffer[MAX_LINE_LENGTH + 1];
    char *current_word = NULL;
    while (fgets(buffer, MAX_LINE_LENGTH + 1, fp)){
        current_word = strtok(buffer, " "); // going over each line
        char *prev_word = NULL;
        while (current_word != NULL){
            if (beginning_of_line == true){
                // previous word is NULL, current word is first tok
                if (!add_to_database(markov_chain, current_word)){
                    return EXIT_FAILURE;}
                cur_num_of_words++; beginning_of_line = false;
                prev_word = current_word;
                continue;
            }// current word is the next word
            current_word = strtok(NULL, " ");
            if (!current_word){break;} // in case of next new line word
            char new_current_word_buffer[MAX_WORD_LENGTH + 1];
            char *new_current_word = NULL;
            // if current word is an ending word, we remove the \r and \n
            if (is_next_word_new_line(current_word) == 0){
                new_current_word = remove_nline_ch
                        (new_current_word_buffer, current_word);
                if (!add_to_database(markov_chain, new_current_word)){
                    return EXIT_FAILURE;
                }
            }
            else {
                if (!add_to_database(markov_chain, current_word)){
                    return EXIT_FAILURE;
                }
            }// Nodes corresponding to current and previous words
            Node *current_node = NULL;
            if (new_current_word){
                current_node = get_node_from_database
                        (markov_chain, new_current_word);}
            else{
              current_node=get_node_from_database(markov_chain,current_word);
            }
            Node *prev_node = get_node_from_database(markov_chain, prev_word);
        // if previous word doesn't end with dot, we add to it's counter list
            if (ends_with_dot(prev_word) == 1){
                if (!add_node_to_counter_list
                (prev_node->data, current_node->data)){
                    return EXIT_FAILURE;
                }
            }// check if you're in the last word, go to a new line
            if (is_next_word_new_line(current_word) == 0){
                beginning_of_line = true;
                cur_num_of_words++;
                break;
            }
            prev_word = current_word;
            cur_num_of_words++;
        }
    }
    return EXIT_SUCCESS;
}


int fill_database (FILE *fp, int words_to_read, MarkovChain
*markov_chain){
    bool beginning_of_line = true;
    int cur_num_of_words = 0;
    char buffer[MAX_LINE_LENGTH + 1];
    char *current_word = NULL;
    while(fgets(buffer,MAX_LINE_LENGTH+1,fp)&&cur_num_of_words<words_to_read){
        current_word = strtok(buffer, " \r\n"); // going over each line
        char *prev_word = NULL;
        while (current_word != NULL && cur_num_of_words < words_to_read){
            if (beginning_of_line == true){
                // previous word is NULL, current word is first tok
                if (!add_to_database(markov_chain, current_word)){
                    return EXIT_FAILURE;}
                cur_num_of_words++; beginning_of_line = false;
                prev_word = current_word; continue;
            } // current word is the next word
            current_word = strtok(NULL, " \r\n");
            if (!current_word){break;}
            char new_current_word_buffer[MAX_WORD_LENGTH + 1];
            char *new_current_word = NULL;
            // if current word is an ending word, we remove the \r and \n
            if (is_next_word_new_line(current_word) == 0){
                new_current_word = remove_nline_ch
                        (new_current_word_buffer, current_word);
                if (!add_to_database(markov_chain, new_current_word)){
                    return EXIT_FAILURE;
                }
            }
            else {
                if (!add_to_database(markov_chain, current_word)){
                    return EXIT_FAILURE;
                }
            } // Nodes corresponding to current and previous words
            Node *current_node = NULL;
            if (new_current_word){
                current_node = get_node_from_database(
                        markov_chain, new_current_word);
            }
            else{
                current_node = get_node_from_database
                        (markov_chain, current_word);}
            Node *prev_node = get_node_from_database(markov_chain, prev_word);
            // if previous word doesn't end with dot, add to it's counter list
            if (ends_with_dot(prev_word) == 1){
                if (!add_node_to_counter_list
                (prev_node->data, current_node->data)){
                    return EXIT_FAILURE;
                }} // check if you're in the last word, go to a new line
            if (is_next_word_new_line(current_word) == 0){
                beginning_of_line = true;
                cur_num_of_words++;
                break;
            }
            prev_word = current_word;
            cur_num_of_words++;
        }
    }
    return EXIT_SUCCESS;
}


int main(int argc, char *argv[]){
    if (argc != MIN_ARGS_NUM && argc != MAX_ARGS_NUM){
        fprintf(stdout, "Usage: seed, number of tweets, path to file and "
                        "(optionally) number of words to read is required\n");
        return EXIT_FAILURE;
    } // initialize the seed
    unsigned int seed = (unsigned int) strtol(argv[1], NULL, 10);
    srand(seed);
    bool read_all_file = false;
    int num_of_tweets_to_generate = (int) strtol(argv[2], NULL, 10);
    int num_of_words_to_read = 0,
    tweet_num = 1;
    FILE *fp = fopen(argv[3], "r");
    if (!fp){
        fprintf(stdout, "Error: failed to open file\n");
        return EXIT_FAILURE;}
    // initializing the structs
    MarkovChain *markov_chain = malloc(sizeof(MarkovChain));
    if (!markov_chain){
        fprintf(stdout, ALLOCATION_ERROR_MASSAGE);
        fclose(fp);
        return EXIT_FAILURE;
    }
    LinkedList *linked_list = malloc(sizeof(LinkedList));
    if (!linked_list){
        free(markov_chain);
        markov_chain = NULL;
        fprintf(stdout, ALLOCATION_ERROR_MASSAGE);
        fclose(fp);
        return EXIT_FAILURE;
    }
    linked_list->first = NULL;
    linked_list->last = NULL;
    linked_list->size = 0;
    markov_chain->database = linked_list;
    if (argc == MAX_ARGS_NUM){
        num_of_words_to_read = (int) strtol(argv[4], NULL, 10);
    }
    else {  // argc == 4
        read_all_file = true;
    }
    if (read_all_file == false){
        fill_database(fp, num_of_words_to_read, markov_chain);
    }
    else {  // read_all_file == true
        fill_database_all_file(fp, markov_chain);
    }
    //generate the tweets
    for (int i = 0; i < num_of_tweets_to_generate; i++){
        fprintf(stdout, "Tweet %d: ", tweet_num);
        MarkovNode *first_node = get_first_random_node(markov_chain);
        generate_random_sequence(markov_chain, first_node,MAX_WORDS_IN_TWEET);
        fprintf(stdout, "\n");
        tweet_num++;
    }
    free_markov_chain(&markov_chain);
    fclose(fp);
    return EXIT_SUCCESS;
}

