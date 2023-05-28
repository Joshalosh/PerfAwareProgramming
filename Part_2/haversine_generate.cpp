#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "haversine_generate.h"
#include "haversine_math.h"
#include "haversine_calculate.cpp"

#define EARTH_RADIUS 6372.8f

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Need to call exectuable with arguments: -- harversine_generate.exe (int)Seed (int)Pairs\n");
    }
    else {
        
        FILE *json_file;

        json_file = fopen("points.json", "w");

        s32 seed = atoi(argv[1]);
        s32 pair_count = atoi(argv[2]);
        srand(seed);

        f64 average_haversine = 0;

        fprintf(json_file, "{\"pairs\":[\n");
        for(s32 index = 0; index < pair_count; index++) {
            f32 x_max =  180.0f;
            f32 x_min = -180.0f;
            f32 y_max =  90.0f;
            f32 y_min = -90.0f;

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
            f32 x0 = x_min + ((f32)rand() / RAND_MAX * (2*x_max));
            f32 y0 = y_min + ((f32)rand() / RAND_MAX * (2*y_max));
            f32 x1 = x_min + ((f32)rand() / RAND_MAX * (2*x_max));
            f32 y1 = y_min + ((f32)rand() / RAND_MAX * (2*y_max));

            fprintf(json_file, "\t{\"x0\":%f, \"y0\":%f, ", x0, y0);
            fprintf(json_file, "\"x1\":%f, \"y1\":%f}", x1, y1);

            if (index < pair_count - 1) {
                fprintf(json_file, ",\n");
            }

            average_haversine += ReferenceHaversine(x0, y0, x1, y1, EARTH_RADIUS);
        }

        average_haversine /= pair_count;
        printf("\n\tThe Average sum is: %f\n", average_haversine);

        fprintf(json_file, "\n]}");
        fclose(json_file);
    }
}
