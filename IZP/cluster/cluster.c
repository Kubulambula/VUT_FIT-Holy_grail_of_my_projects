/********************/
/**  Jakub Jansta  **/
/**    xjanst02    **/
/********************/


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h> // sqrtf()
#include <limits.h> // INT_MAX
#include <float.h> // FLT_MAX
#include <string.h> // strlen(), strcmp()


#ifdef NDEBUG

#define debug(s)
#define dfmt(s, ...)
#define dint(i)
#define dfloat(f)

#else

// vypise ladici retezec
#define debug(s) printf("- %s\n", s)
// vypise formatovany ladici vystup - pouziti podobne jako printf
#define dfmt(s, ...) printf(" - "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)
// vypise ladici informaci o promenne - pouziti dint(identifikator_promenne)
#define dint(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)
// vypise ladici informaci o promenne typu float - pouziti
// dfloat(identifikator_promenne)
#define dfloat(f) printf(" - " __FILE__ ":%u: " #f " = %g\n", __LINE__, f)

#endif


// Chunk of cluster objects. Value recommended for reallocation.
const int CLUSTER_CHUNK = 10;


struct obj_t {
    int id;
    float x;
    float y;
};

struct cluster_t {
    int size;
    int capacity;
    struct obj_t *obj;
};


void sort_cluster(struct cluster_t *c);


void init_cluster(struct cluster_t *c, int cap){
    assert(c != NULL);
    assert(cap >= 0);
    c->obj = calloc(sizeof(struct obj_t) * cap, sizeof(struct obj_t)); // used calloc() so that valgrind is happy
    c->capacity = c->obj ? cap : 0;
    c->size = 0;
}


void clear_cluster(struct cluster_t *c){
    free(c->obj);
    init_cluster(c, CLUSTER_CHUNK);
}


struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap){
    // TUTO FUNKCI NEMENTE
    assert(c);
    assert(c->capacity >= 0);
    assert(new_cap >= 0);

    if (c->capacity >= new_cap)
        return c;

    size_t size = sizeof(struct obj_t) * new_cap;

    void *arr = realloc(c->obj, size);
    if (arr == NULL)
        return NULL;

    c->obj = (struct obj_t*)arr;
    c->capacity = new_cap;
    return c;
}


void append_cluster(struct cluster_t *c, struct obj_t obj){
    assert(c != NULL);

    if (c->size >= c->capacity){
        resize_cluster(c, c->capacity + CLUSTER_CHUNK);
    }
    c->obj[c->size++] = obj;
}


void merge_clusters(struct cluster_t *c1, struct cluster_t *c2){
    assert(c1 != NULL);
    assert(c2 != NULL);

    for(int i = 0; i < c2->size; i++){
        append_cluster(c1, c2->obj[i]);
    }
    sort_cluster(c1);
}


int remove_cluster(struct cluster_t *carr, int narr, int idx){
    assert(idx < narr);
    assert(narr > 0);

    // The array does not have to be sorted ¯\_(ツ)_/¯
    struct cluster_t temp = carr[idx];
    carr[idx] = carr[--narr];
    carr[narr] = temp;
    return narr;
}


float obj_distance(struct obj_t *o1, struct obj_t *o2){
    assert(o1 != NULL);
    assert(o2 != NULL);

    return sqrtf(pow(o2->x - o1->x, 2) + pow(o2->y - o1->y, 2));
}


float cluster_distance(struct cluster_t *c1, struct cluster_t *c2){
    assert(c1 != NULL);
    assert(c1->size > 0);
    assert(c2 != NULL);
    assert(c2->size > 0);

    float smallest_distance = FLT_MAX;
    for (int i = 0; i < c1->size; i++){
        for (int ii = 0; i < c2->size; i++){
            float d = obj_distance(&(c1->obj[i]), &(c2->obj[ii]));
            if (d < smallest_distance){
                smallest_distance = d;
            }
        }
    }
    return smallest_distance;
}


void find_neighbours_with_single_linkage(struct cluster_t *carr, int narr, int *c1, int *c2){
    assert(narr > 0);

    float d = -1;
    float smallest_distance = FLT_MAX;
    for (int i = 0; i < narr; i++){
        for(int ii = 0; ii < narr; ii++){
            if (i == ii){
                continue;
            }
            d = cluster_distance(&carr[i], &carr[ii]);
            if (d < smallest_distance){
                smallest_distance = d;
                *c1 = i;
                *c2 = ii;
            }
        }
    }
}


void find_neighbours_with_centroids(struct cluster_t *carr, int narr, int *c1, int *c2){
    assert(narr > 0);

    // Get centers of all clusters
    struct obj_t centers[narr];
    for (int i = 0; i < narr; i++){
        centers[i].x = centers[i].y = 0;

        for (int i = 0; i < carr[i].size; i++){
            centers[i].x += carr[i].obj[i].x;
            centers[i].y += carr[i].obj[i].y;
        }
        centers[i].x /= carr[i].size;
        centers[i].y /= carr[i].size;
    }

    float d, smallest_distance = FLT_MAX;
    // Compares each path just once
    for (int i = 0; i < narr-1; i++){
        for(int ii = 1; ii < narr-i; ii++){
            d = obj_distance(&centers[i], &centers[i+ii]);
            if (d < smallest_distance){
                smallest_distance = d;
                *c1 = i;
                *c2 = i + ii;
            }
        }
    }
}


void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2){
    assert(narr > 0);
    find_neighbours_with_single_linkage(carr, narr, c1, c2);
}


static int obj_sort_compar(const void *a, const void *b){
    // TUTO FUNKCI NEMENTE
    const struct obj_t *o1 = (const struct obj_t *)a;
    const struct obj_t *o2 = (const struct obj_t *)b;
    if (o1->id < o2->id) return -1;
    if (o1->id > o2->id) return 1;
    return 0;
}


static int cluster_sort_compar(const void *a, const void *b){
    // ALE KOPIROVAT SI JI MUZEME
    const struct cluster_t *o1 = (const struct cluster_t *)a;
    const struct cluster_t *o2 = (const struct cluster_t *)b;
    if (o1->obj[0].id < o2->obj[0].id) return -1;
    if (o1->obj[0].id > o2->obj[0].id) return 1;
    return 0;
}


void sort_cluster(struct cluster_t *c){
    // TUTO FUNKCI NEMENTE
    qsort(c->obj, c->size, sizeof(struct obj_t), &obj_sort_compar);
}


void sort_clusters(struct cluster_t **clusters, int size){
    // ALE KOPIROVAT SI JI MUZEME
    qsort(*clusters, size, sizeof(struct cluster_t), &cluster_sort_compar);
}


int load_obj_line(FILE* *f, int ids[], int id_idx, int *id, float *x, float *y){
    char element[5] = {'\0'};
    short element_length = 0;
    short element_count = 0;
    while(1){
        char c = fgetc(*f);
        if (c == ' ' || c == '\n' || c == EOF){
            switch (element_count){
                case 0:
                    *id = atoi(element);
                    for(int i = 0; i < id_idx; i++){
                        if (ids[i] == *id){
                            fprintf(stderr, "Can't load clusters: Obj has non-unique id");
                            return 1;
                        }
                    }
                    ids[id_idx] = *id;
                    break;
                case 1:
                    *x = (float)atoi(element);
                    break;
                case 2:
                    *y = (float)atoi(element);
                    break;
                default:
                    fprintf(stderr, "Can't load clusters: Too many obj elements");
                    return 1;
            }
            if(strlen(element) == 0){
                fprintf(stderr, "Can't load clusters: Obj element has missing number");
                return 1;
            }
            // Reset working variables
            memset(&element, 0, sizeof(element));
            element_length = 0;
            element_count++;
            // Check if obj end
            if (c == EOF){
                return 0;
            } else if(c == '\n'){
                if(element_count < 3){
                    fprintf(stderr, "Can't load clusters: Not enough obj elements");
                    return 1;
                }
                return 0;
            }
            continue;
        } else if (c < '0' ||  c > '9'){
            fprintf(stderr, "Can't load clusters: Obj element is not a number");
            return 1;
        }

        if (element_length >= 4){
            fprintf(stderr, "Can't load clusters: Obj element out of range (0 <= X <= 1000)\n");
            return 1;
        }
        element[element_length++] = c;
    }
    return 0;
}


int load_clusters(char *filename, struct cluster_t **arr){
    assert(arr != NULL);
    // Open file
    FILE* f = fopen(filename, "r");
    if (f == NULL){
        perror("Can't open");
        // Cleanup after fail
        *arr = NULL;
        return 0;
    }
    // Get and check obj count that is supposed to be inside the file
    int obj_count = 0;
    if (fscanf(f, "count=%i\n", &obj_count) != 1){
        fprintf(stderr, "Can't load clusters: Bad file format\n");
        // Cleanup after fail
        *arr = NULL;
        fclose(f);
        return 0;
    }
    if(obj_count < 1){
        fprintf(stderr, "Can't load clusters: Invalid obj count: %i\n", obj_count);
        // Cleanup after fail
        *arr = NULL;
        fclose(f);
        return 0;
    }
    // Allocate and validate cluster array
    *arr = calloc(sizeof(struct cluster_t) * obj_count, sizeof(struct cluster_t)); // used calloc() so that valgrind is happy
    if(*arr == NULL){
        fprintf(stderr, "Can't load clusters: Can't allocate enough memory\n");
        // Cleanup after fail
        *arr = NULL;
        fclose(f);
        return 0;
    }

    // Scan all lines
    int loaded_lines = 0;
    int *ids = calloc(sizeof(int) * obj_count, sizeof(int)); // used calloc() so that valgrind is happy
    if (ids == NULL){
        fprintf(stderr, "Can't load clusters: Can't allocate enough memory\n");
        // Cleanup after fail
        for(int i = 0; i < loaded_lines; i++){
            free((*arr)[i].obj);
        }
        free(*arr);
        *arr = NULL;
        fclose(f);
        return 0;
    }
    for (int i = 0; i < obj_count; i++){
        struct obj_t obj;
        loaded_lines++;

        // Init and check new cluster
        init_cluster(&(*arr)[i], CLUSTER_CHUNK);
        if ((*arr)[i].obj == NULL){
            fprintf(stderr, "Can't load clusters: Can't allocate enough memory\n");
            // Cleanup after fail
            free(ids);
            for(int i = 0; i < loaded_lines; i++){
                free((*arr)[i].obj);
            }
            free(*arr);
            *arr = NULL;
            fclose(f);
            return 0;
        }
        // Load and check if line is loaded correctly
        if (load_obj_line(&f, ids, i, &obj.id, &obj.x, &obj.y)){
            fprintf(stderr, " on line %i\n", i+2);
            // Cleanup after fail
            free(ids);
            for(int i = 0; i < loaded_lines; i++){
                free((*arr)[i].obj);
            }
            free(*arr);
            *arr = NULL;
            fclose(f);
            return 0;
        }
        // Check if line has valid data
        if (obj.x < 0 || obj.x > 1000 || obj.y < 0 || obj.y > 1000){
            fprintf(stderr, "Can't load clusters: Object coordinates out of range on line %i\n", i+2);
            // Cleanup after fail
            free(ids);
            for(int i = 0; i < loaded_lines; i++){
                free((*arr)[i].obj);
            }
            free(*arr);
            *arr = NULL;
            fclose(f);
            return 0;
        }
        // Append obj from line to cluster
        append_cluster(&((*arr)[i]), obj);
    }
    // Cleanup
    free(ids);
    fclose(f);
    return loaded_lines;
}


void print_cluster(struct cluster_t *c){
    // TUTO FUNKCI NEMENTE
    for (int i = 0; i < c->size; i++){
        if (i) putchar(' ');
        printf("%d[%g,%g]", c->obj[i].id, c->obj[i].x, c->obj[i].y);
    }
    putchar('\n');
}


void print_clusters(struct cluster_t *carr, int narr){
    printf("Clusters:\n");
    for (int i = 0; i < narr; i++){
        printf("cluster %d: ", i);
        print_cluster(&carr[i]);
    }
}


// -l - single linkage
// -c - centroids
int load_args(int argc, char **argv[], int *desired_cluster_count, int *algorithm){
    switch (argc){
        case 1: // No args
            fprintf(stderr, "Not enought args.\nUsage: cluster [FILENAME](mandatory) [N](optional) [algorithm](optional)");
            return 1;
        
        case 2: // 1 arg (filename)
            *desired_cluster_count = 1;
            *algorithm = 0;
            return 0;
        
        case 3: // 2 args (filename, N)
            // Get N
            for (int i = 0; i < (int)strlen((*argv)[2]); i++){
                if ((*argv)[2][i] < '0' || (*argv)[2][i] > '9'){
                    fprintf(stderr, "Invalid argument N");
                    return 1;
                }
            }
            *desired_cluster_count = atoi((*argv)[2]);
            if (*desired_cluster_count < 1){
                fprintf(stderr, "Invalid argument N");
                return 1;
            }
            *algorithm = 0;
            return 0;
        
        case 4: // 3 args (filename, N, algorithm)
            // get N
            for (int i = 0; i < (int)strlen((*argv)[2]); i++){
                if ((*argv)[2][i] < '0' || (*argv)[2][i] > '9'){
                    fprintf(stderr, "Invalid argument N");
                    return 1;
                }
            }
            *desired_cluster_count = atoi((*argv)[2]);
            if (*desired_cluster_count < 1){
                fprintf(stderr, "Invalid argument N");
                return 1;
            }
            // Get algorithm
            if (strlen((*argv)[3]) != 2){
                fprintf(stderr, "Invalid algorithm");
                return 1;
            } else if (!strcmp((*argv)[3], "-l")){
                *algorithm = 0;
                return 0;
            } else if (!strcmp((*argv)[3], "-c")){
                *algorithm = 1;
                return 0;
            } else {
                fprintf(stderr, "Invalid algorithm");
                return 1;
            }
            return 0;
        
        default: // Too many args
            fprintf(stderr, "Too many arguments (1, 2 or 3 expected, but %i were given) ", argc-1);
            return 1;
    }
}


int solve(struct cluster_t **clusters, int cluster_count, int desired_cluster_count, int algorithm){
    if (desired_cluster_count > cluster_count){
        fprintf(stderr, "Can't solve for %i desired clusters with %i default clusters", desired_cluster_count, cluster_count);
        return 1;
    }

    int a, b;
    while (desired_cluster_count < cluster_count){
        // Get two closest clusters with the desired algorithm
        switch (algorithm){
            case 0:
                find_neighbours_with_single_linkage(*clusters, cluster_count, &a, &b);
                break;
            case 1:
                find_neighbours_with_centroids(*clusters, cluster_count, &a, &b);
                break;
            default:
                fprintf(stderr, "Invalid algorithm: %i", algorithm);
                return 1;
        }
        // Merge the two closest clusters
        merge_clusters(&(*clusters)[a], &(*clusters)[b]);
        cluster_count = remove_cluster(*clusters, cluster_count, b);
    }
    return 0;
}


int main(int argc, char *argv[]){
    struct cluster_t *clusters;
    int desired_cluster_count, algorithm, loaded_cluster_count;
    //                         ^^^^^^^^^ 0=single linkage ; 1=centroids

    // Load args
    if (load_args(argc, &argv, &desired_cluster_count, &algorithm)){
        return 1;
    }
    
    // Load clusters
    loaded_cluster_count = load_clusters(argv[1], &clusters);
    if (loaded_cluster_count == 0){
        return 1;
    }

    // Solve the clusters
    if(!solve(&clusters, loaded_cluster_count, desired_cluster_count, algorithm)){
        sort_clusters(&clusters, desired_cluster_count);
        print_clusters(&(*clusters), desired_cluster_count);
    }

    // Cleanup
    for(int i = 0; i < loaded_cluster_count; i++){
        free(clusters[i].obj);
    }
    free(clusters);

    return 0;
}