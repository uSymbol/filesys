#include <stdio.h>
#include "file.h"


int main() {
    printf("Hello, World! start\n");

    storage_file_s *storage = storage_file_init("./storage", 10, sizeof(int), 256);

    for (int i = 0; i < 10; i++)
    {
        printf("storage_insert! start\n");
        char str[128] = "hello world abc";
        storage_insert(storage, &i, str, (int)strlen(str));
        printf("storage_insert! end\n");
    }

    for (int i = 0; i < 10; i++)
    {
        printf("storage_find! start\n");
        char str[128] = {0};
        storage_find(storage, &i, str, sizeof(str));
        printf("ret %s \n",str);
        printf("storage_find! end\n");
    }


    printf("Hello, World! end\n");
    return 0;
}
