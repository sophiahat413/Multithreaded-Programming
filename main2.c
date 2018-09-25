#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/syscall.h>


#define handle_error_en(en, msg) \
              do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#include "primitives.h"
#include "raytracing.h"

#define OUT_FILENAME "out2.ppm"

#define ROWS 512
#define COLS 512
#define TOTAL_THREAD 2

static void write_to_ppm(FILE *outfile, uint8_t *pixels,
                         int width, int height)
{
    fprintf(outfile, "P6\n%d %d\n%d\n", width, height, 255);
    fwrite(pixels, 1, height * width * 3, outfile);
}

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

struct input
{
  uint8_t *pixel;
  color background_color;
  rectangular_node rectangulars;
  sphere_node spheres;
  light_node lights;
  const viewpoint *view;
  int width;
  int height;
  int id ;
  int total;
};


int main()
{
    uint8_t *pixels;
    light_node lights = NULL;
    rectangular_node rectangulars = NULL;
    sphere_node spheres = NULL;
    color background = { 0.0, 0.1, 0.1 };

    struct timespec start, end;

    pixels = malloc(sizeof(unsigned char) * ROWS * COLS * 3);
    if (!pixels) exit(-1);

#include "use-models.h"

    int q,k;
    //char buf[256];
  //  struct input data;
    cpu_set_t cpu, mask;
    pthread_t thread[2];
    struct input* data[2];

    printf("# Rendering scene\n");
    /* do the ray tracing with the given geometry */
    clock_gettime(CLOCK_REALTIME, &start);

    for(q = 0; q < 2; q++){
      int s,j,current,cpu_id;
      /* allocate by the given resolution */
      data[q] = (struct input *) malloc(sizeof(struct input));
      data[q]->pixel = pixels;
      data[q]->background_color[0] = background[0];
      data[q]->background_color[1] = background[1];
      data[q]->background_color[2] = background[2];
      data[q]->rectangulars = rectangulars;
      data[q]->spheres = spheres;
      data[q]->lights = lights;
      data[q]->view = &view;
      data[q]->width = ROWS;
      data[q]->height = COLS;
      data[q]->id = q;
      data[q]->total = TOTAL_THREAD;
      //printf("q: %d\n", q);
      if(pthread_create(&thread[q], NULL, raytracing,(void*)data[q])){
          printf("Cannot create thread.");
          return -1;
      }

      cpu_id = q % 4;
      CPU_ZERO(&mask);
      CPU_SET(cpu_id, &mask);
      current = pthread_setaffinity_np(thread[q], sizeof(cpu_set_t), &mask);
      if (current != 0)
          handle_error_en(current, "pthread_setaffinity_np");
      /*Check the actual affinity mask assigned to the thread*/
      CPU_ZERO(&cpu);
      s = pthread_getaffinity_np(thread[q], sizeof(cpu_set_t), &cpu);
      if (s != 0) {
          handle_error_en(s, "pthread_getaffinity_np");
      }
      for (j = 0; j < CPU_SETSIZE; j++)
        if (CPU_ISSET(j, &cpu))
          printf("thread is running in cpu %d\n", j);
      //sleep(2);

    }

    //waut tiil other thread are complete

    for(k = 0; k < 2; k++){
      if(pthread_join(thread[k], NULL))
      {
          printf("Could not join thread\n");
          return -1;
      }
      else{
        printf("thread %d join\n", k);
      }
    }

    //TODO
    /*This is a domain function for this program.
     * Please trace the parameter of it and create your threads to do the function*/
    /*raytracing(pixels, background,
               rectangulars, spheres, lights, &view, ROWS, COLS);*/

    clock_gettime(CLOCK_REALTIME, &end);
    {
        printf("into clock_gettime");
        FILE *outfile = fopen(OUT_FILENAME, "wb");
        write_to_ppm(outfile, pixels, ROWS, COLS);
        fclose(outfile);
    }
    delete_rectangular_list(&rectangulars);
    delete_sphere_list(&spheres);
    delete_light_list(&lights);
    free(pixels);
    free(data[0]);
    free(data[1]);
    printf("Done!\n");
    printf("Execution time of raytracing() : %lf sec\n", diff_in_second(start, end));
    return 0;
}
