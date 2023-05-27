#include<stdio.h>
#include<stdlib.h>

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Need to call exectuable with arguments: -- harversine_generate.exe (int)Seed (int)Pairs\n");
    }
    else {
        
        FILE *json_file;

        json_file = fopen("points.json", "w");

        int seed = atoi(argv[1]);
        int pair_count = atoi(argv[2]);
        srand(seed);

        fprintf(json_file, "{\"pairs\":[\n");
        for(int index = 0; index < pair_count; index++) {
            float range = 180.0f;

            float x0 = ((float)rand()/(float)(RAND_MAX)) * range;
            float y0 = ((float)rand()/(float)(RAND_MAX)) * range;
            float x1 = ((float)rand()/(float)(RAND_MAX)) * range;
            float y1 = ((float)rand()/(float)(RAND_MAX)) * range;

            fprintf(json_file, "\t{\"x0\":%f, \"y0\":%f,\n", x0, y0);
            fprintf(json_file, "{\"x1\":%f, \"y1\":%f},\n", x1, y1);
        }

        fclose(json_file);
    }
}
