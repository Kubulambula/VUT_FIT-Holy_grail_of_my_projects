/********************/
/**  Jakub Jansta  **/
/**    xjanst02    **/
/********************/


#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
//#include <unistd.h>


#define MAX_LINE_LEN 100
#define MAX_CONTACTS 50


typedef struct {
    char name[MAX_LINE_LEN+1];
    char number[MAX_LINE_LEN+1];
}Contact;


bool contact_matches_filter(Contact ct, char *filter, bool continuous);
bool name_matches_filter(char *name, char *filter, bool continuous);
bool number_matches_filter(char *number, char *filter, bool continuous);

bool is_valid_number(char supposed_number[]);
bool is_valid_filter(char *filter);

int load_contacts(Contact contacts[]);


int main(int argc, char *argv[]){

    char *filter = NULL;
    bool continuous = true;
    
     
    /* Disabled because it might break the automated tests
    
    // stdin is coming from terminal and not from a file
    if (isatty(0)){
        errno = ENOTSUP; // Operation not supported
        perror("stdin is not a file ");
        return errno;
    }
    */

    switch (argc){
        case 1: // No args
            break;
        
        case 2: // 1 arg (filter)
            if (!is_valid_filter(argv[1])){
                errno = EINVAL; // Invalid argument
                perror("Filter is not a not a numeral value ");
                return errno;
            }

            filter = argv[1];
            break;
        
        case 3: // 2 args (-s + filter)
            if (strcmp(argv[1], "-s")){
                errno = EINVAL; // Invalid argument
                perror("Unknown argument ");
                return errno;
            }

            if (!is_valid_filter(argv[2])){
                errno = EINVAL; // Invalid argument
                perror("Filter is not a not a numeral value ");
                return errno;
            }

            continuous = false;
            filter = argv[2];
            break;
        
        default:
            errno = E2BIG; // Argument list too long
            char err[64];
            sprintf(err, "Too many arguments (0, 1 or 2 expected, but %i were given) ", argc-1);
            perror(err);
            return errno;
            break;
    }

    // Load the contacts
    Contact contacts[MAX_CONTACTS];
    memset(contacts, '\0', sizeof(contacts));
    int contact_count = load_contacts(contacts);
    if (!contact_count){
        errno = EINVAL; // Invalid argument
        perror("Invalid contact data ");
        return errno;
    }

    if (filter){
        // print filtered
        bool found_at_least_one = false;
        for (int i = 0; i < contact_count; i++){
            if (contact_matches_filter(contacts[i], filter, continuous)){
                printf("%s, %s\n", contacts[i].name, contacts[i].number);
                found_at_least_one = true;
            }
        }
        if (!found_at_least_one){
            printf("Not found\n");
        }
    }else{
        // print all
        for (int i = 0; i < contact_count; i++){
            printf("%s, %s\n", contacts[i].name, contacts[i].number);
        }
    }

    return 0;
}


bool contact_matches_filter(Contact ct, char *filter, bool continuous){
    return name_matches_filter(ct.name, filter, continuous) || number_matches_filter(ct.number, filter, continuous);
}


bool name_matches_filter(char *name, char *filter, bool continuous){
    // convets the name to the key sequence that would have to be pressed so that the same algorithm can be used
    char name_as_number[MAX_LINE_LEN+1] = {'\0'};
    for (int i = 0; i < (int)strlen(name); i++){
        switch (name[i]){
            case '+': case '0':
                name_as_number[i] = '0';
                break;
            case '1':
                name_as_number[i] = '1';
                break;
            case 'A': case 'a': case 'B': case 'b': case 'C': case 'c': case '2':
                name_as_number[i] = '2';
                break;
            case 'D': case 'd': case 'E': case 'e': case 'F': case 'f': case '3':
                name_as_number[i] = '3';
                break;
            case 'G': case 'g': case 'H': case 'h': case 'I': case 'i': case '4':
                name_as_number[i] = '4';
                break;
            case 'J': case 'j': case 'K': case 'k': case 'L': case 'l': case '5':
                name_as_number[i] = '5';
                break;
            case 'M': case 'm': case 'N': case 'n': case 'O': case 'o': case '6':
                name_as_number[i] = '6';
                break;
            case 'P': case 'p': case 'Q': case 'q': case 'R': case 'r': case 'S': case 's': case '7':
                name_as_number[i] = '7';
                break;
            case 'T': case 't': case 'U': case 'u': case 'V': case 'v': case '8':
                name_as_number[i] = '8';
                break;
            case 'W': case 'w': case 'X': case 'x': case 'Y': case 'y': case 'Z': case 'z': case '9':
                name_as_number[i] = '9';
                break;
            default:
                name_as_number[i] = ' ';
        }
    }
    return number_matches_filter(name_as_number, filter, continuous);
}


bool number_matches_filter(char *number, char *filter, bool continuous){
    if (continuous){
        for (int i = 0; i < (int)strlen(number); i++){
            for (int ii = i, consumed = 0; ii < (int)strlen(number); ii++){
                if (number[ii] == ' '){ // skip spaces ' ' 
                    continue;
                }
                if (number[ii] == filter[consumed] || (number[ii] == '+' && filter[consumed] == '0')){
                    consumed++;
                    if (consumed == (int)strlen(filter)){
                        return true;
                    }
                    continue;
                }
                break;
            }
        }
    }else{ // -s argument
        for (int i = 0, consumed = 0; i < (int)strlen(number); i++){
            if (number[i] == ' '){ // skip spaces ' ' 
                continue;
            }
            if (number[i] == filter[consumed] || (number[i] == '+' && filter[consumed] == '0')){
                consumed++;
                if (consumed == (int)strlen(filter)){
                    return true;
                }
            }
        }
    }

    return false;
}


bool is_valid_number(char supposed_number[]){
    if (!strlen(supposed_number)){
        return false;
    }
    bool contains_a_number = false;
    for (int i = supposed_number[0] == '+' ? 1 : 0; i < (int)strlen(supposed_number); i++){ // skips the 1st char if it is '+'
        if (supposed_number[i] >= '0' && supposed_number[i] <= '9'){
            contains_a_number = true;
        }else if (supposed_number[i] != ' '){ // allows ' '
            return false;
        }
    }
    return contains_a_number;
}


bool is_valid_filter(char *filter){
    for (int i = 0; i < (int)strlen(filter); i++){
        if (!(filter[i] >= '0' && filter[i] <= '9')){
            return false;
        }
    }
    return true;
}


// Returns the number of the loaded contacts if successful or 0 if failed
int load_contacts(Contact contacts[]){
    char c;
    int current_line = 0;
    int current_line_char = 0;
    int loaded_contact_count = 0;
    char current_line_char_buffer[MAX_LINE_LEN+1] = {'\0'};
    Contact ct;

    while ((c = getchar()) != EOF){
        if (current_line_char == MAX_LINE_LEN){
            return 0; // Line is exceeds MAX_LINE_LEN
        }

        if(c == '\n'){
            if (!strlen(current_line_char_buffer)){
                return 0; // Empty line
            }

            if (current_line % 2 == 0){ // odd line (name)
                strcpy(ct.name, current_line_char_buffer);
            }else{ // even line (number)
                if (!is_valid_number(current_line_char_buffer)){
                    return 0; // line is not a valid number
                }

                strcpy(ct.number, current_line_char_buffer);

                // Push the new contact
                contacts[(current_line-1)/2] = ct;
                loaded_contact_count++;

                // If MAX_CONTACTS is reached, the rest of the stdin will be ignored
                if ((current_line+1)/2 > MAX_CONTACTS-1){
                    return loaded_contact_count;
                }
            }

            // Reset the temp variables for the next line
            current_line++;
            current_line_char = 0;
            memset(current_line_char_buffer, '\0', sizeof(current_line_char_buffer));
        }else{
            current_line_char_buffer[current_line_char++] = c;
        }
    }
    return loaded_contact_count;
}
