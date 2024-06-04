#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <sys/file.h>
#include <string.h>
#include "monteCarloAB.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

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

void monteCarloAB(int argc, char *argv[]) {
    if (argc != 4) {
        write(STDERR_FILENO, "Uso: ", 5);
        write(STDERR_FILENO, argv[0], strlen(argv[0]));
        write(STDERR_FILENO, " <nome_ficheiro_texto> <numero_processos_filho> <numero_total_pontos>\n", 76);
        exit(EXIT_FAILURE);
    }

    const char *filename = argv[1];
    const int num_children = atoi(argv[2]);
    const int total_points = atoi(argv[3]);

    int MAX_POINTS, count = 0;
    srand(time(NULL));

    int file_fd = open(filename, O_RDONLY);
    if (file_fd == -1) {
        char error_msg[] = "Erro ao abrir o arquivo\n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        exit(EXIT_FAILURE);
    }

    //leitura do numero total pontos poligno
    MAX_POINTS = read_int(file_fd);
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

    int output_fd = open("RequisitoB.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (output_fd == -1) {
        char error_msg[] = "Erro ao abrir o arquivo de saída\n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        exit(EXIT_FAILURE);
    }

    int i, pid, total_inside = 0;;
    for (i = 0; i < num_children; i++) {
        pid = fork();
        if (pid == 0) {
            srand(time(NULL) ^ getpid());
            int points_inside = 0;
            for (int j = 0; j < total_points / num_children; j++) {
                Point p = {((double) rand() / RAND_MAX) * 2 - 1, ((double) rand() / RAND_MAX) * 2 - 1};
                if (isInsidePolygon(polygon, MAX_POINTS, p)) {
                    points_inside++;
                }
            }
            char msg[100];
            int msg_len = sprintf(msg, "%d;%d;%d\n", getpid(), total_points / num_children, points_inside);
            write(output_fd, msg, msg_len);
            exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            perror("Erro ao criar processo filho");
            exit(EXIT_FAILURE);
        }
    }

    int status;
    for (i = 0; i < num_children; i++) {
        wait(&status);
    }

    close(output_fd);

    // Abre o arquivo RequisitoB.txt para leitura
    int result_file = open("RequisitoB.txt", O_RDONLY);
    if (result_file == -1) {
        char error_msg[] = "Erro ao abrir o arquivo\n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        exit(EXIT_FAILURE);
    }

    char line[BUFFER_SIZE];
    while (read_line(result_file, line, BUFFER_SIZE) > 0) {
        char *last_column = strrchr(line, ';');
        if (last_column != NULL) {
            total_inside += atoi(last_column + 1);
        }
    }

    close(result_file);

    double squareArea = 4.0;
    double area = squareArea * ((double) total_inside / total_points);
    printf("Área estimada do polígono: %f\n", area);

    free(polygon);
    close(output_fd);
}