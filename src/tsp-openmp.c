#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <float.h>
#include <math.h>
#include <omp.h>

typedef struct coord {
  float x;
  float y;
} coord;

float dist(coord a, coord b) {
  // Calculate the squared L2 norm
  return (a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y);
}

float pathlen(coord *coords, unsigned short *path, size_t n_cities) {
  // Calculate length of the path through the cities.
  float len = 0.0;
  for (size_t i = 1; i < n_cities; ++i)
    len += dist(coords[path[i]], coords[path[i - 1]]);
  return len;
}

void swap(unsigned short *a, unsigned short *b) {
  // Swap two unsigned shorts in-place.
  unsigned short tmp = *a;
  *a = *b;
  *b = tmp;
}

void shuffle(unsigned short *arr, size_t n) {
  // Shuffle the values in arr in-place.
  for (size_t i = 0; i < n; ++i) {
    size_t j = rand() % (i + 1);
    swap(&arr[i], &arr[j]);
  }
}

void InitPath(size_t n_cities, unsigned short* path) {
  // Initialize a random path through the cities by shuffling their indices.
  for (size_t i = 0; i < n_cities; ++i)
    path[i] = i;
  shuffle(path, n_cities);
}

void mutate(unsigned short* path, size_t n_cities) {
  // Mutate the path by swapping the positions of two cities.
  size_t i = rand() % (n_cities);
  size_t j = i;

  // Must mutate
  while (i == j)
    j = rand() % (n_cities);
  swap(&path[i], &path[j]);
}

bool IsInPath(unsigned short *path, unsigned short city, size_t i_max) {
  // Check if the city has already been visited in the path before index i_max.
  for (size_t i = 0; i < i_max; ++i) {
    if (path[i] == city)
      return true;
  }
  return false;
}

void breed(unsigned short* a, unsigned short* b, coord* coords, size_t n_cities, unsigned short* child) {
  // Breed two paths by using the algorithm from the assignment notes.
  unsigned short next_city = ((rand() % 2) == 1 ? b[0] : a[0]);
  child[0] = next_city;
  for (size_t i = 1; i < n_cities; ++i) {
    // Choose the next city from either parent that is closer to the current city
    float dist_a = dist(coords[a[i]], coords[child[i - 1]]);
    float dist_b = dist(coords[b[i]], coords[child[i - 1]]);
    next_city = (dist_a < dist_b ? a[i] : b[i]);

    // If the chosen city has already been visited, choose the city from the other parent
    if (IsInPath(child, next_city, i)) {
      if (i != n_cities - 1) {
	next_city = (dist_a < dist_b ? b[i] : a[i]);
	size_t tmp_index = i;
	// If the city is still not eligible, try other cities further down the path.
	while (IsInPath(child, next_city, i) && ++tmp_index != n_cities) {
	  bool which_parent = (rand() % 2) == 1;
	  next_city = (which_parent ? a[tmp_index] : b[tmp_index]);
	  if (IsInPath(child, next_city, i)) {
	    next_city = (which_parent ? b[tmp_index] : a[tmp_index]);
	  }
	}
      }
    }
    // Perform a final check that the city has not been visited.
    if (IsInPath(child, next_city, i)) {
      // If the city has been visited, then find the first city in
      // ascending order that has not been visited
      bool *have_visited = (bool *)malloc(n_cities * sizeof(bool));
      for (size_t j = 0; j < n_cities; ++j) {
	      have_visited[j] = false;
      }
      for (size_t j = 0; j < i; ++j) {
	      have_visited[child[j]] = true;
      }
      size_t j = 0;
      while (have_visited[j])
	      ++j;
      next_city = j;
      free(have_visited);
    }
    child[i] = next_city;
  }
}

int CountLines(char* filename) {
  // Count the number of lines in a file.
  FILE *fp = fopen(filename, "r");
  int lines = 0;
  while(!feof(fp)) {
    int ch = fgetc(fp);
    if(ch == '\n') {
      lines++;
    }
  }
  fclose(fp);
  return lines;
}

void ReadCoords(char* filename, size_t n_cities, coord* coords) {
  // Read 2D coordinates from a space-separated file, where the rows contain the coordinates.
  FILE *fp;
  fp = fopen (filename, "r");
  int index = 0;
  while(fscanf(fp, "%f %f", &coords[index].x, &coords[index].y) == 2)
    index++;
  fclose(fp);
}

float rand_p() {
  // Draw a random float between 0 and 1.
  return (float)rand() / (float)RAND_MAX;
}

void FitnessStatus(unsigned short* pops[], coord *coords, size_t pop_size, size_t n_cities) {
  // Find the most fit individual in pops and print out its fitness and the chosen path.
  size_t best_index = 0;
  float best_fitness = FLT_MAX;
  for (size_t i = 0; i < pop_size; ++i) {
    float fitness = pathlen(coords, pops[i], n_cities);
    if (fitness < best_fitness) {
      best_fitness = fitness;
      best_index = i;
    }
  }

  printf("%s", "best path: ");
  for (size_t i = 0; i < n_cities; ++i) {
    printf("%d", pops[best_index][i]);
    if (i < n_cities - 1) {
      printf(",");
    }
  }
  
  printf(" fitness: %f\n", best_fitness);
}

float BestFit(unsigned short* pops[], coord *coords, size_t pop_size, size_t n_cities, size_t *best_index) {
  // Extract the best path from pops. Returns the fitness of the path and stores its index in best_path.
  float best_fitness = FLT_MAX;
  for (size_t i = 0; i < pop_size; ++i) {
    float fitness = pathlen(coords, pops[i], n_cities);
    if (fitness < best_fitness) {
      best_fitness = fitness;
      best_index[0] = i;
    }
  }
  return best_fitness;
}

int cmp_ptr(const void *a, const void *b) {
  // Comparison for two float pointers. Used in order_float(2)
  const float **left  = (const float **)a;
  const float **right = (const float **)b;
  return (**left < **right) - (**right < **left);
}

size_t * order_float(const float *a, size_t n) {
  // Find the (descending) order in an array of floats using qsorts and return the indices rather than the ordered values.
    const float **pointers = (const float **)malloc(n * sizeof(const float *));
    for (size_t i = 0; i < n; i++)
      pointers[i] = a + i;

    qsort(pointers, n, sizeof(const float *), cmp_ptr);
    size_t *indices = (size_t *)malloc(n * sizeof(size_t));
    for (size_t i = 0; i < n; i++)
      indices[i] = pointers[i] - a;

    free(pointers);
    return indices;
}

void selection(unsigned short* old_pops[], unsigned short* new_pops[], unsigned short pop_size, coord* coords, size_t n_cities) {
  // Select the most fit individuals from both the old and new pops to survive to the next generation.

  // First calculate fitness for both populationms
  float *fits = (float *) malloc(2 * pop_size * sizeof(float));
  for (size_t i = 0; i < pop_size; ++i) {
    fits[2*i] = pathlen(coords, old_pops[i], n_cities);
    fits[2*i + 1] = pathlen(coords, new_pops[i], n_cities);
  }

  // Select the pop_size most fit individuals from the combined old and new population
  size_t* indices = order_float(fits, 2*pop_size);
  for (size_t i = 0; i < pop_size; ++i) {
    size_t pos = 2 * pop_size - (i + 1);
    bool from_old = (indices[pos] % 2) == 0;
    if (from_old) {
      for (size_t k = 0; k < n_cities; ++k) {
	old_pops[i][k] = old_pops[indices[pos]/2][k];
      }
    } else {
      for (size_t k = 0; k < n_cities; ++k) {
	old_pops[i][k] = new_pops[(indices[pos] - 1)/2][k];
      }
    }
  }

  free(fits);
  free(indices);
}

void immigration(unsigned short* pops[], size_t migration_size, size_t n_cities, size_t pop_size, coord *coords, unsigned short *emigrants[]) {
  // Integrate the emigrants into pops by wiping out the 100 most unfit individuals.

  // Calculate fitness for the population
  float *fits = (float *) malloc(pop_size * sizeof(float));
  for (size_t i = 0; i < pop_size; ++i) {
    fits[i] = pathlen(coords, pops[i], n_cities);
  }
  size_t* indices = order_float(fits, pop_size);

  // Replace worst 100 individuals with the emigrants
  for (size_t i = 0; i < migration_size; ++i)
    pops[indices[i]] = emigrants[i];

  free(indices);
  free(fits);
}

int w_srand(float *weights, size_t n_weights) {
  // Sample random integers in rangee 0, n_weights by weighting the probability to sample the integere with the *inverse* of its weight in weights.
  float sum = 0.0;
  for (size_t i = 0; i < n_weights; ++i) {
    sum += 1.0/weights[i];
  }
  int max = ceil(sum);  
  int rn = rand() % max;
  for (size_t i = 0; i < n_weights; ++i) {
    if (rn < 1.0/weights[i])
      return i;
    rn -= 1.0/weights[i];
  }
  exit(1); // should never reach here
}

void w_select_emigrants(unsigned short *pops[], unsigned short pop_size, size_t migration_size, coord* coords, size_t n_cities, unsigned short *emigrants[]) {
  // Select pops for emigration, weighted by their inverse-fitness.

  // Calculate fitness
  float *fits = (float *) malloc(pop_size * sizeof(float));
  for (size_t i = 0; i < pop_size; ++i) {
    fits[i] = pathlen(coords, pops[i], n_cities);
  }

  for (size_t i = 0; i < migration_size; ++i) {
    size_t emigrant_index = w_srand(fits, pop_size);
    for (size_t k = 0; k < n_cities; ++k) {
      emigrants[i][k] = pops[emigrant_index][k];
    }
  }
  free(fits);
}

void check_input(float mutation_prob, size_t pop_size, float migration_prob, size_t migration_size) {
  // Validate that the input values are correct.
  if (mutation_prob < 0 || mutation_prob >= 1) {
    printf("Mutation probability must be between 0 and 1\n");
    exit(1);
  }
  if (migration_prob < 0 || migration_prob >= 1) {
    printf("Migration probability must be between 0 and 1\n");
    exit(1);
  }
  if (migration_size >= pop_size) {
    printf("Migration size must be less than population size\n");
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  int id=0,ntasks=1;  
  
  #ifdef PRINT
  double start_time_total,end_time_total,cpu_time_used_total;  
  start_time_total =omp_get_wtime(); 
  #endif  
  

  //shared points
  size_t *v_my_best_path;
  float *v_best_fit;


  // Input values
  float mutation_prob;
  size_t n_generations;
  size_t pop_size;
  float migration_prob;
  size_t migration_size;

  size_t n_cities;
  coord *coords;

  
  // Process 0 will handle reading the input.
  char* infile = argv[1];
  mutation_prob = atof(argv[2]);
  n_generations = atoi(argv[3]);
  pop_size = atoi(argv[4]);
  migration_prob = atof(argv[5]);
  migration_size = atof(argv[6]);

  if (argc < 7) {
    printf("Usage: tsp <mutation probability, float> <n generations, size_t> <population size, size_t> <migration probability, float> <migration size, size_t>\n");    
    exit(1);
  }
  check_input(mutation_prob, pop_size, migration_prob, migration_size);

  // Check how many cities there are and read in their coordinates
  n_cities = CountLines(infile);
  coords = (coord *)malloc(n_cities * sizeof(coord));
  ReadCoords(infile, n_cities, coords);  
  
  #pragma omp parallel default(none) firstprivate(id) shared(v_my_best_path,v_best_fit,pop_size,n_cities,n_generations,migration_size,coords,mutation_prob,migration_prob,ntasks) 
  {
    // Initialize a population from random paths
    #ifdef PRINT
    double start_time,end_time,cpu_time_used;
    start_time = omp_get_wtime();  
    #endif 
    id = omp_get_thread_num();
    if ( id == 0){
      ntasks=omp_get_num_threads();
      printf("Numero de thread %d\n", ntasks);
      v_my_best_path =  (size_t *) malloc(sizeof(size_t) * ntasks);
      v_best_fit = (float *) malloc(sizeof(float) * ntasks);
    }    

    unsigned short *pops[pop_size];
    for (size_t i = 0; i < pop_size; ++i)
    pops[i] = (unsigned short*)malloc(n_cities * sizeof(unsigned short));
    srand(time(0) + id); // different seed for each process
    for (size_t i = 0; i < pop_size; ++i)
      InitPath(n_cities, pops[i]);

    unsigned short *new_pops[pop_size];
    for (size_t j = 0; j < pop_size; ++j)
      new_pops[j] = (unsigned short*)malloc(n_cities * sizeof(unsigned short));

    int has_immigrants = 0;
    unsigned short **emigrants; 
    #ifdef PRINT
    end_time = omp_get_wtime();
    cpu_time_used = (((double)(end_time - start_time))) + cpu_time_used; 
    #endif
    #pragma omp for
    for (size_t i = 0; i < n_generations; ++i) 
    {
      #ifdef PRINT
      start_time = omp_get_wtime(); 
      #endif     
      // Check pops want to emigrate
      if (rand_p() < migration_prob) {
        // Emigrants and immigrants need to be allocated contiguously for MPI
        unsigned short *data = (unsigned short *)malloc(migration_size*n_cities*sizeof(unsigned short));
        emigrants= (unsigned short **)malloc(migration_size*sizeof(unsigned short*));
        for (size_t i = 0; i < migration_size; ++i)
          emigrants[i] = &(data[n_cities*i]);

        // Randomly select the emigrants (weight by inverse of the fitness)
        w_select_emigrants(pops, pop_size, migration_size, coords, n_cities, emigrants);

        // Send to either the next or the previous process
        bool prev = (rand() % 2) == 1;
        size_t target_id = (prev ? id - 1 : id + 1);
        if (prev && id == 0) {
          target_id = ntasks - 1;
        } else if (!prev && id == ntasks - 1) {
          target_id = 0;
        }
        has_immigrants = 1;
        // MPI_Request req;
        // MPI_Isend(&(emigrants[0][0]), n_cities * migration_size, MPI_UNSIGNED_SHORT, target_id, 42, MPI_COMM_WORLD, &req);
        
      }

     
      if (has_immigrants) {
        //unsigned short *data = (unsigned short *)malloc(migration_size*n_cities*sizeof(unsigned short));
        //unsigned short **immigrants= emigrants;
        // for (size_t i = 0; i < migration_size; ++i)
        //   immigrants[i] = &(data[n_cities*i]);

        // MPI_Recv(&(immigrants[0][0]), n_cities * migration_size, MPI_UNSIGNED_SHORT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);

        // // Replace the 100 most unfit pops with the immigrants
        immigration(pops, migration_size, n_cities, pop_size, coords, emigrants);
        free(emigrants);
      }

      
      for (size_t j = 0; j < pop_size; ++j) {
        // Cross self with a randomly selected pop
        size_t parent_a = j;
        // Can't mate with self.
        while (j == parent_a)
          parent_a = rand() % pop_size;
        breed(pops[parent_a], pops[j], coords, n_cities, new_pops[j]);

        // Introduce a mutation with probability mutation_p
        float mutate_p = rand_p();
        //      if (rand_p() < mutation_prob)
        while (mutate_p < mutation_prob) {
          mutate(new_pops[j], n_cities);
          mutate_p = rand_p();
        }
      }
      // Select the fit individuals to populate the next generation
      selection(pops, new_pops, pop_size, coords, n_cities);

      //Uncomment for periodic updates
      if ((i % 1000) == 0) {
        printf("Process %d generation %zu\n", id, i + 1);
        //FitnessStatus(pops, coords, pop_size, n_cities);
      }  
      #ifdef PRINT
      end_time = omp_get_wtime();
      cpu_time_used = (((double)(end_time - start_time))) + cpu_time_used;     
      #endif
    }
    #ifdef PRINT
    start_time = omp_get_wtime(); 
    #endif      
    size_t my_best_path[1];
    my_best_path[0] = 0;
    float best_fit = BestFit(pops, coords, pop_size, n_cities, my_best_path);    
    v_best_fit[id]= best_fit;   
    v_my_best_path[id]= my_best_path[0];  
    #ifdef PRINT    
    end_time = omp_get_wtime();
    cpu_time_used = (((double)(end_time - start_time)) ) + cpu_time_used;  
    printf("Total time for thread %d is %f seconds\n", id, cpu_time_used);    
    #endif   
  }   
  size_t my_best_path[1];
  my_best_path[0] = 0;
  int best_id=0;  
  for(int i=0;i<ntasks;i++){
    if ( v_best_fit[i] < v_best_fit[best_id]){
      best_id=i;     
    }
  }
 
  
  my_best_path[0] = v_my_best_path[best_id]; 
  printf("Process %d has a final best fitness of %f.\n", best_id, v_best_fit[best_id]);   
  //printf("Fitness of the best route found is %f.\n", v_best_fit[best_id]);
  //printf("The best route found is ");
  // for (size_t i = 0; i < n_cities; ++i) {
  //   printf("%d", pops[my_best_path[0]][i]);
  //   if (i < n_cities - 1)
  //     printf(",");
  // } 
  // printf(".\n"); 
  
  #ifdef PRINT  
  end_time_total = omp_get_wtime();
  cpu_time_used_total = ((double)(end_time_total - start_time_total));
  printf("Total time for the program is %f seconds\n", cpu_time_used_total);
  #endif

  return 0;
}
