struct find_arr{
    int size;
    char **array;
    char *current_dir;
    char *searching_file;
};

struct find_arr* create(int size);

void set_searching_parameters(struct find_arr* find_arr, char* directiory, char* file);

void search(struct find_arr *find_arr, char* file_name);

int load_searching_result(struct find_arr* find_arr, char* file_name);

int delete(struct find_arr* find_arr, int index);


void print(struct find_arr* find_array);

int free_slots(struct find_arr* find_array);
