#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
            float x_max =  180.0f;
            float x_min = -180.0f;
            float y_max =  90.0f;
            float y_min = -90.0f;

            // To get a random floating point number in the correct range first
            // I need to get a normalised random number between the ranges of
            // [0, 1] and then multiple it by double the maximum range amount.
            // Then I add the minimum range amount to that value to shift the entire
            // range down by that minimum value. This results in a random float
            // that spans the correct range I'm looking for. 
            //
            // RAND_MAX is the maximum random value I can get which is 32767.
            // so by dividing my random value by that amount and casting it to
            // a float I can normalise my random value between [0, 1].
            float x0 = x_min + ((float)rand() / RAND_MAX * (2*x_max));
            float y0 = y_min + ((float)rand() / RAND_MAX * (2*y_max));
            float x1 = x_min + ((float)rand() / RAND_MAX * (2*x_max));
            float y1 = y_min + ((float)rand() / RAND_MAX * (2*y_max));

            fprintf(json_file, "\t{\"x0\":%f, \"y0\":%f, ", x0, y0);
            fprintf(json_file, "\"x1\":%f, \"y1\":%f},\n", x1, y1);
        }

        fclose(json_file);
    }
}
