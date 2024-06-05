//
// Created by Cristiano Rocha on 02/06/2024.
//

#include "mcThreadC.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <sys/file.h>
#include <string.h>
#include <pthread.h>
#include <dispatch/dispatch.h>
#include <sys/semaphore.h>

/**
 * Estrutura para uso dentro das threads
 */
typedef struct {
    Point *polygon;
    int num_points;
    int points_inside;
    int N;
    int M;
    int counter;
    Point *points;
} ThreadArgsConsumer;

/**
 * Estrutura para uso dentro das threads
 */
typedef struct {
    int N;
    int M;
    int counter;
    Point *points;
} ThreadArgsProducer;

pthread_mutex_t trinco_p = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t trinco_c = PTHREAD_MUTEX_INITIALIZER;

int prodptr = 0, consptr = 0;

sem_t *pode_prod, *pode_cons;

/**
 * @brief Determines the orientation of an ordered triplet (p, q, r).
 * @param p First point of the triplet.
 * @param q Second point of the triplet.
 * @param r Third point of the triplet.
 * @return 0 if p, q, and r are colinear, 1 if clockwise, 2 if counterclockwise.
 */
int orientation(Point p, Point q, Point r) {
    double val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

    if (val == 0) return 0;
    return (val > 0) ? 1 : 2;
}

/**
 * @brief Checks if point q lies on line segment pr.
 * @param p First point of the line segment.
 * @param q Point to check.
 * @param r Second point of the line segment.
 * @return true if point q lies on line segment pr, else false.
 */
bool onSegment(Point p, Point q, Point r) {
    if (q.x <= fmax(p.x, r.x) && q.x >= fmin(p.x, r.x) &&
        q.y <= fmax(p.y, r.y) && q.y >= fmin(p.y, r.y))
        return true;

    return false;
}

/**
 * @brief Checks if line segments p1q1 and p2q2 intersect.
 * @param p1 First point of the first line segment.
 * @param q1 Second point of the first line segment.
 * @param p2 First point of the second line segment.
 * @param q2 Second point of the second line segment.
 * @return true if line segments p1q1 and p2q2 intersect, else false.
 */
bool doIntersect(Point p1, Point q1, Point p2, Point q2) {

    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    if (o1 != o2 && o3 != o4)
        return true;

    // p1, q1 and p2 are colinear and p2 lies on segment p1q1
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;

    // p1, q1 and q2 are colinear and q2 lies on segment p1q1
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;

    // p2, q2 and p1 are colinear and p1 lies on segment p2q2
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;

    // p2, q2 and q1 are colinear and q1 lies on segment p2q2
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;

    return false;
}

/**
 * @brief Checks if a point p is inside a polygon of n points.
 * @param polygon[] Array of points forming the polygon.
 * @param n Number of points in the polygon.
 * @param p Point to check.
 * @return true if the point p is inside the polygon, else false.
 */
bool isInsidePolygon(Point polygon[], int n, Point p) {

    if (n < 3) return false;

    Point extreme = {2.5, p.y};

    int count = 0, i = 0;
    do {
        int next = (i + 1) % n;

        if (doIntersect(polygon[i], polygon[next], p, extreme)) {
            if (orientation(polygon[i], p, polygon[next]) == 0)
                return onSegment(polygon[i], p, polygon[next]);
            count++;
        }
        i = next;
    } while (i != 0);

    return count & 1;
}

/**
 * @brief reads the line pointed to by the filepointer and stores the read value in a buffer
 * @param fd filepointer to line to be read
 * @param buffer location that will receive the characters read
 * @param n size of buffer
 * @return returns number of bytes read
 */
ssize_t read_line(int fd, char *buffer, size_t n) {
    ssize_t num_bytes = 0;
    char ch;
    while ((num_bytes < n - 1) && (read(fd, &ch, 1) > 0)) {
        buffer[num_bytes] = ch;
        num_bytes++;
        if (ch == '\n') {
            break;
        }
    }
    buffer[num_bytes] = '\0';
    return num_bytes;
}

/**
 * @brief reads the line pointed to by the filepointer and translates that to a number
 * @param fd - filepointer points to character to be read
 * @return read number
 */
int read_int(int fd) {
    char buffer[BUFFER_SIZE];
    read_line(fd, buffer, BUFFER_SIZE);
    return atoi(buffer);
}

/**
 * funcao para as threads Worker
 * @param args
 * @return
 */
void *consumer_thread(void *args) {

    ThreadArgsConsumer *argsC = (ThreadArgsConsumer *) args;

    Point p;

    while (argsC->counter < argsC->M) {

        sem_wait(pode_cons);
        pthread_mutex_lock(&trinco_c);

        if (argsC->counter > argsC->M) {
            pthread_mutex_unlock(&trinco_c);
            pthread_exit(NULL);
        }

        p = argsC->points[consptr];
        consptr = (consptr + 1) % argsC->N;
        argsC->counter++;

        if (isInsidePolygon(argsC->polygon, argsC->num_points, p)) {
            argsC->points_inside++;
        }

        pthread_mutex_unlock(&trinco_c);
        sem_post(pode_prod);
    }

    pthread_exit(NULL);
}

void *producer_thread(void *args) {

    ThreadArgsProducer *argsP = (ThreadArgsProducer *) args;

    while (argsP->counter < argsP->M) {
        Point p;
        p.x = ((double) rand() / RAND_MAX) * 2 - 1;
        p.y = ((double) rand() / RAND_MAX) * 2 - 1;

        sem_wait(pode_prod);
        pthread_mutex_lock(&trinco_p);

        if (argsP->counter > argsP->M) {
            pthread_mutex_unlock(&trinco_p);
            pthread_exit(NULL);
        }

        argsP->points[prodptr] = p;
        argsP->counter++;
        prodptr = (prodptr + 1) % argsP->N;

        pthread_mutex_unlock(&trinco_p);
        sem_post(pode_cons);
    }

    pthread_exit(NULL);
}


void mcThreadC(int argc, char *argv[]) {
    //verifica se tem os argumentos todos, e se o numero de produtores ou cosumidores nao é 0
    if (argc != 5 || argv[2] == "0" || argv[3] == "0") {
        write(STDERR_FILENO, "Uso: ", 5);
        write(STDERR_FILENO, argv[0], strlen(argv[0]));
        write(STDERR_FILENO,
              " <nome_ficheiro_texto> <numero_consumidores_criar> <numero_produtores_criar> <numero_total_pontos>\n",
              76);
        exit(EXIT_FAILURE);
    }

    const char *filename = argv[1];//nome do ficheiro de texto que contem poligon
    const int C = atoi(argv[2]);//numero de threads Produtoras
    const int P = atoi(argv[3]);//numero de threads Consumidoras
    const int M = atoi(argv[4]);//numero de pontos random a criar

    int MAX_POINTS, count = 0;
    srand(time(NULL));

    int file_fd = open(filename, O_RDONLY);
    if (file_fd == -1) {
        char error_msg[] = "Erro ao abrir o arquivo\n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        exit(EXIT_FAILURE);
    }

    MAX_POINTS = read_int(file_fd);//primeira linha do ficheiro contem a quantidade de pontos do ficheiro poligon
    Point *polygon = malloc(MAX_POINTS * sizeof(Point));
    if (polygon == NULL) {
        char error_msg[] = "Erro ao alocar memória\n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    while (count < MAX_POINTS && read_line(file_fd, buffer, BUFFER_SIZE) > 0) {
        sscanf(buffer, "%lf,%lf", &polygon[count].x, &polygon[count].y);
        count++;
    }

    if (count >= MAX_POINTS) {
        char error_msg[] = "Número máximo de pontos excedido.\n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
    }

    close(file_fd);

    /** *-----------------------------------------------//-----------------------------------------------* **/

    int N = M / C; //tamanho do buffer NUMERO DE PONTOS/ NUMERO DE CONSUMIDORES

    //nosso buffer, que terá os pontos produzidos
    Point *random_points = malloc(N * sizeof(Point));
    if (random_points == NULL) {
        perror("Erro ao alocar memória");
        exit(EXIT_FAILURE);
    }

    pthread_t producer[P], consumer[C];

    pode_prod = sem_open("/pode_prod", O_CREAT, 0644, N);
    pode_cons = sem_open("/pode_cons", O_CREAT, 0644, 0);

    ThreadArgsConsumer consumerArgs = {polygon, MAX_POINTS, 0, N, M, 0, random_points};
    ThreadArgsProducer producerArgs = {N, M, 0, random_points};


    for (int i = 0; i < P; i++) {
        if (pthread_create(&producer[i], NULL, producer_thread, &producerArgs) != 0) {
            perror("Erro ao criar thread");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < C; i++) {
        if (pthread_create(&consumer[i], NULL, consumer_thread, &consumerArgs) != 0) {
            perror("Erro ao criar thread");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < P; i++) {
        pthread_join(producer[i], NULL);
        printf("%i PRODUCER ACABOU\n", i);
    }
    for (int j = 0; j < C; j++) {
        pthread_join(consumer[j], NULL);
        printf("%i CONSUMER ACABOU\n", j);
    }

    double squareArea = 4.0;
    double area = squareArea * ((double) consumerArgs.points_inside / M);
    printf("Área estimada do polígono: %f\n", area);

    free(polygon);
    free(random_points);
    pthread_mutex_destroy(&trinco_p);
    pthread_mutex_destroy(&trinco_c);
}