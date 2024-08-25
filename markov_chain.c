#include "markov_chain.h"


// helpers
int ends_with_dot(char *str){
    //! checks if word ends with a dot
    if (str[strlen(str) - 1] == '.'){
        return 0;
    }
    else{
        return 1;
    }
}

char *remove_nline_ch(char s[], char *str){
    //! removes the '\r\n' characters from a line ending word
    memcpy(s, str, strlen(str) + 1);
    char *tok = strtok(s, " \r\n");
    return tok;
}

MarkovNode *create_markov_node(char *data){
    //! allocates a new markov node containing the given data
    MarkovNode *markov = malloc(sizeof (MarkovNode));
    if (!markov){
        return NULL;
    }
    markov->data = malloc(strlen(data) + 1);
    if (!markov->data){
        free(markov);
        return NULL;
    }
    strcpy(markov->data, data);
    markov->counter_list = NULL;
    markov->counter_list_size = 0;
    markov->frequencies_sum = 0;
    return markov;
}

int is_next_word_new_line(char *word){
    //! checks if current word is the last word in the line
    if (word[strlen(word) - 1] == '\n'){
        return 0;
    }
    else{
        return 1;
    }
}

void free_markov_node(MarkovNode **markov_node){
    //! frees the given markov node
    // free the counter list
    free((*markov_node)->counter_list);
    // free the data
    free((*markov_node)->data);
    (*markov_node)->data = NULL;
    free(*markov_node);
    *markov_node = NULL;
}

void free_node(Node **node){
    //! frees the given node
    free_markov_node(&(*node)->data);
    free(*node);
    *node = NULL;
}

/**
* Get random number between 0 and max_number [0, max_number).
* @param max_number maximal number to return (not including)
* @return Random number
*/
int get_random_number(int max_number)
{
    return rand() % max_number;
}


// base functions

MarkovNode* get_first_random_node(MarkovChain *markov_chain){
    if (!markov_chain){
        return NULL;
    }
    Node *temp = NULL;
    //generate a new number,and get the corresponding indexed node in the list
    // if the word is a tweet's ending word, we redo the process
    while(true) {
        temp = markov_chain->database->first;
        int number = get_random_number(markov_chain->database->size);
        for (int i = 0; i < number; i++) {
            temp = temp->next;
        }
        if (ends_with_dot(temp->data->data) == 1) {
            break;
        }
    }
    return temp->data;
}


MarkovNode* get_next_random_node(MarkovNode *state_struct_ptr){
    if (!state_struct_ptr){
        return NULL;
    }
    int number = get_random_number(state_struct_ptr->frequencies_sum);
    // variables to help choose the node based on the probability
    NextNodeCounter *temp_counter_lst = state_struct_ptr->counter_list;
    if (number == 0){
        return temp_counter_lst[0].markov_node;
    }
    while (number>=0){
        number-= temp_counter_lst->frequency;
        if(number< 0){
            return temp_counter_lst->markov_node;
        }
        temp_counter_lst++;
    }
    return temp_counter_lst->markov_node;
}


void generate_random_sequence(MarkovChain *markov_chain, MarkovNode *
first_node, int max_length){
    if (!markov_chain){
        return;
    }
    int cur_length = 0;
    // if first node is invalid, we generate a new one
    if (!first_node || ends_with_dot(first_node->data) == 0){
        first_node = get_first_random_node(markov_chain);
    }
    printf("%s ", first_node->data);
    MarkovNode *cur_node = get_next_random_node(first_node);
// we stop generating if we reach maximum length or if we get an ending word
    while (cur_length < max_length && ends_with_dot(cur_node->data) == 1){
        printf("%s ", cur_node->data);
        cur_node = get_next_random_node(cur_node);
        cur_length++;
    }
    printf("%s", cur_node->data);
}


void free_markov_chain(MarkovChain ** ptr_chain){
    if (!ptr_chain || !(*ptr_chain)){
        return;
    }
    Node *node = (*ptr_chain)->database->first;
    // going over the entire linked list, freeing its nodes
    for (int i = 0; i < (*ptr_chain)->database->size; i++) {
        Node *temp = node->next;
        free_node(&node);
        node = temp;
    }
    // free the linked list
    free((*ptr_chain)->database);
    (*ptr_chain)->database = NULL;
    // free the markov chain pointer
    free(*ptr_chain);
    *ptr_chain = NULL;
}


bool add_node_to_counter_list(MarkovNode *first_node, MarkovNode *second_node){
    if (!first_node || !second_node){
        return false;
    }
    if (!first_node->counter_list){  // first time, we do malloc
        first_node->counter_list = malloc(sizeof(NextNodeCounter));
        if (!first_node->counter_list){
            return false;
        }
        first_node->counter_list[0]= (NextNodeCounter){1,second_node};
        first_node->counter_list_size = 1;
        first_node->frequencies_sum = 1;
        return true;
    }
    NextNodeCounter *temp_counter_lst = first_node->counter_list;
    // going over the first node's counter list
    for (int i = 0; i < first_node->counter_list_size; i++) {
        // if the two strings are the same,
        // we update the frequency and return true
       if(strcmp(temp_counter_lst[i].markov_node->data,second_node->data)==0){
            temp_counter_lst[i].frequency++;
            first_node->frequencies_sum++;
            return true;
        }
    }
    // if second NOT in first, we create and add to the counter list
    NextNodeCounter next_node_to_add = {1, second_node};
    first_node->counter_list = realloc(first_node->counter_list,
           (first_node->counter_list_size + 1) * sizeof(NextNodeCounter));
    if (!first_node->counter_list){
        return false;
    }
    first_node->counter_list[first_node->counter_list_size] = next_node_to_add;
    first_node->counter_list_size++;
    first_node->frequencies_sum++;
    return true;
}


Node* get_node_from_database(MarkovChain *markov_chain, char *data_ptr){
    if (!markov_chain){
        return NULL;
    }
    Node *res = NULL;
    Node* temp = markov_chain->database->first;
    for (int i = 0; i < markov_chain->database->size; i++) {
        // if current node's data is the same as given data_ptr
        if (strcmp(temp->data->data, data_ptr) == 0){
            res = temp;
            break;
        }
        temp = temp->next;
    }
    return res;
}


Node* add_to_database(MarkovChain *markov_chain, char *data_ptr){
    if (!markov_chain){
        return NULL;
    }
    LinkedList *linked_lst = markov_chain->database;
    Node *node = get_node_from_database(markov_chain, data_ptr);
    // if data_ptr is not in the chain, we add it
    if (!node){
        MarkovNode *new_markov_node = create_markov_node(data_ptr);
        // if addition to list was NOT successful
        if (add(linked_lst, new_markov_node) == 1){
            free_markov_node(&new_markov_node);
            return NULL;
        }
//if addition was successful, we will return it at the end of the function
    }
    // if data_ptr is already in the chain, we return its node
    return get_node_from_database(markov_chain, data_ptr);
}
