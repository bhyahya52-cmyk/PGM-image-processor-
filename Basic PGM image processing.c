#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


typedef struct {
    int rows;       
    int cols;       
    int max_gray;   
    int **pixels;  
} Image;


Image* initialize_image(int rows, int cols, int max_gray) {
    Image *img = (Image*)malloc(sizeof(Image));
    if (!img) {
        perror("Erreur d'allocation de la structure image");
        return NULL;
    }

    img->rows = rows;
    img->cols = cols;
    img->max_gray = max_gray;

    img->pixels = (int**)malloc(rows * sizeof(int*));
    if (!img->pixels) {
        free(img);
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        img->pixels[i] = (int*)calloc(cols, sizeof(int)); 
        if (!img->pixels[i]) {
            for (int j = 0; j < i; j++) free(img->pixels[j]);
            free(img->pixels);
            free(img);
            return NULL;
        }
    }
    return img;
}
void free_image(Image *img) {
    if (img != NULL) {
        if (img->pixels != NULL) {
            for (int i = 0; i < img->rows; i++) {
                free(img->pixels[i]);
            }
            free(img->pixels);
        }
        free(img);
    }
}

Image* copy_image(Image *src) {
    if (!src) return NULL;
    
    Image *dst = initialize_image(src->rows, src->cols, src->max_gray);
    if (!dst) return NULL;

    for (int i = 0; i < src->rows; i++) {
        for (int j = 0; j < src->cols; j++) {
            dst->pixels[i][j] = src->pixels[i][j];
        }
    }
    return dst;
}


void get_image_info(Image *img, int *rows, int *cols, int *max_gray) {
    if (img) {
        *rows = img->rows;
        *cols = img->cols;
        *max_gray = img->max_gray;
        printf("Info Image -> Rows: %d, Cols: %d, Max Gray: %d\n", *rows, *cols, *max_gray);
    }
}


int is_within_bounds(Image *img, int row, int col) {
    if (!img) return 0;
    return (row >= 0 && row < img->rows && col >= 0 && col < img->cols);
}


int get_pixel(Image *img, int row, int col) {
    if (is_within_bounds(img, row, col)) {
        return img->pixels[row][col];
    }
    return 0; 
	}

void set_pixel(Image *img, int row, int col, int value) {
    if (is_within_bounds(img, row, col)) {
    	
        if (value > img->max_gray) value = img->max_gray;
        if (value < 0) value = 0;
        img->pixels[row][col] = value;
    }
}

void generate_test_pattern(Image *img) {
    if (!img) return;

    int pattern[7][24] = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,3,3,3,3,0,0,7,7,7,7,0,0,11,11,11,11,0,0,15,15,15,15,0},
        {0,3,0,0,0,0,0,7,0,0,0,0,0,11,0,0,0,0,0,15,0,0,15,0},
        {0,3,3,3,0,0,0,7,7,7,0,0,0,11,11,11,0,0,0,15,15,15,15,0},
        {0,3,0,0,0,0,0,7,0,0,0,0,0,11,0,0,0,0,0,15,0,0,0,0},
        {0,3,0,0,0,0,0,7,7,7,7,0,0,11,11,11,11,0,0,15,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    };

    for (int i = 0; i < img->rows; i++) {
        for (int j = 0; j < img->cols; j++) {
            set_pixel(img, i, j, pattern[i][j]);
        }
    }
}



Image* negate_image(Image *img) {
    if (!img) return NULL;
    Image *neg_img = copy_image(img); 

    for (int i = 0; i < neg_img->rows; i++) {
        for (int j = 0; j < neg_img->cols; j++) {
            int original_val = neg_img->pixels[i][j];
            neg_img->pixels[i][j] = neg_img->max_gray - original_val;
        }
    }
    return neg_img;
}


Image* rotate_image(Image *img, double theta) {
    if (!img) return NULL;
    
    Image *new_img = initialize_image(img->rows, img->cols, img->max_gray);
    
    int cx = img->cols / 2; 
    int cy = img->rows / 2; 

    for (int y = 0; y < new_img->rows; y++) {
        for (int x = 0; x < new_img->cols; x++) {
        	
            int src_x = (int)((x - cx) * cos(theta) - (y - cy) * sin(theta) + cx);
            int src_y = (int)((x - cx) * sin(theta) + (y - cy) * cos(theta) + cy);

            if (is_within_bounds(img, src_y, src_x)) {
                new_img->pixels[y][x] = img->pixels[src_y][src_x];
            } else {
            
                new_img->pixels[y][x] = 0; 
            }
        }
    }
    return new_img;
}

void embed_text(Image *img, const char *text) {
    int len = strlen(text) + 1; 
    int pixel_idx = 0;
    int total_pixels = img->rows * img->cols;

    if (len * 8 > total_pixels) {
        printf("Erreur: Image trop petite pour contenir le message.\n");
        return;
    }

    for (int i = 0; i < len; i++) {
        char c = text[i];
        for (int bit = 0; bit < 8; bit++) {
            int bit_val = (c >> bit) & 1;

            int row = pixel_idx / img->cols;
            int col = pixel_idx % img->cols;

            if (bit_val) {
                img->pixels[row][col] |= 1;  // Force le bit à 1
            } else {
                img->pixels[row][col] &= ~1; // Force le bit à 0
            }
            pixel_idx++;
        }
    }
    printf("Message encode avec succes.\n");
}

void decode_text(Image *img) {
    char buffer[1024] = {0}; 
    int char_idx = 0;
    int pixel_idx = 0;
    int total_pixels = img->rows * img->cols;

    while (pixel_idx + 8 <= total_pixels && char_idx < 1023) {
        char c = 0;
        for (int bit = 0; bit < 8; bit++) {
            int row = pixel_idx / img->cols;
            int col = pixel_idx % img->cols;


            int val = img->pixels[row][col] & 1;
            
            if (val) {
                c |= (1 << bit);
            }
            pixel_idx++;
        }
        
        if (c == '\0') {
            break; 
        }
        buffer[char_idx++] = c;
    }
    printf("Message decode: %s\n", buffer);
}


Image* load_pgm_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return NULL;
    
    char buffer[16];
    int cols, rows, max;
    
    
    fscanf(f, "%s", buffer);
    if (strcmp(buffer, "P2") != 0) { fclose(f); return NULL; }
    
    
    int c = getc(f);
    while (c == '#') { while (getc(f) != '\n'); c = getc(f); }
    ungetc(c, f);

    fscanf(f, "%d %d", &cols, &rows);
    fscanf(f, "%d", &max);
    
    Image *img = initialize_image(rows, cols, max);
    for(int i=0; i<rows; i++) {
        for(int j=0; j<cols; j++) {
            fscanf(f, "%d", &img->pixels[i][j]);
        }
    }
    fclose(f);
    return img;
}

void save_pgm_file(Image *img, const char *filename) {
    if (!img) return;
    FILE *f = fopen(filename, "w");
    if (!f) return;
    
    fprintf(f, "P2\n");
    fprintf(f, "%d %d\n", img->cols, img->rows); 
    fprintf(f, "%d\n", img->max_gray);
    
    for(int i=0; i<img->rows; i++) {
        for(int j=0; j<img->cols; j++) {
            fprintf(f, "%d ", img->pixels[i][j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

int main() {
    Image *test_img = initialize_image(7, 24, 15);
    generate_test_pattern(test_img);
    save_pgm_file(test_img, "input.pgm");
    free_image(test_img);

    Image *img = load_pgm_file("apollonian_gasket.ascii.pgm");
    if (!img) { printf("Erreur chargement.\n"); return 1; }
    
    int r, c, m;
    get_image_info(img, &r, &c, &m);


    Image *rotated = rotate_image(img, 1.5708); // 1.5708 rad ~ 90 deg
    save_pgm_file(rotated, "rotated.pgm");
    printf("Image rotation.\n");

 
    Image *negated = negate_image(img);
    save_pgm_file(negated, "negated.pgm");
    printf("Image negatif .\n");


    printf("\n--- Test Steganographie ---\n");
    Image *stego_img = copy_image(img); 
    const char *secret = "Bonjour monsieur  Khalid Bouzhdir";
    printf("Message a cacher: %s\n", secret);
    
    embed_text(stego_img, secret);
    save_pgm_file(stego_img, "secret.pgm");


    printf("\n");
    decode_text(stego_img);


    free_image(img);
    free_image(rotated);
    free_image(negated);
    free_image(stego_img);
    
    return 0;
}


