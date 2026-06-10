#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
// ==========================================
// CONFIGURATION & DEFINITIONS
// ==========================================
#define WIDTH 60
#define HEIGHT 20
#define MAX_OBJECTS 100
// Character representation
#define CHAR_BG '_'
#define CHAR_FG '*'
typedef enum {
    OBJ_LINE = 1,
    OBJ_RECTANGLE,
    OBJ_CIRCLE,
    OBJ_TRIANGLE
} ObjectType;
typedef struct {
    int x1, y1;
    int x2, y2;
} LineData;
typedef struct {
    int x, y;
    int w, h;
} RectData;
typedef struct {
    int cx, cy;
    int r;
} CircleData;
typedef struct {
    int x1, y1;
    int x2, y2;
    int x3, y3;
} TriangleData;
typedef struct {
    int id;
    ObjectType type;
    union {
        LineData line;
        RectData rect;
        CircleData circle;
        TriangleData triangle;
    } data;
} GraphicObject;
// Global Canvas
char canvas[HEIGHT][WIDTH];
// Global Shape Storage
static GraphicObject objects[MAX_OBJECTS];
static int object_count = 0;
static int next_id = 1;
// ==========================================
// FUNCTION DECLARATIONS
// ==========================================
void clear_canvas(char canvas[HEIGHT][WIDTH]);
void set_pixel(char canvas[HEIGHT][WIDTH], int x, int y);
void display_canvas(char canvas[HEIGHT][WIDTH]);
void draw_line(char canvas[HEIGHT][WIDTH], int x1, int y1, int x2, int y2);
void draw_rectangle(char canvas[HEIGHT][WIDTH], int x, int y, int w, int h);
void draw_circle(char canvas[HEIGHT][WIDTH], int cx, int cy, int r);
void draw_triangle(char canvas[HEIGHT][WIDTH], int x1, int y1, int x2, int y2, int x3, int y3);
int add_object(GraphicObject obj);
int delete_object(int id);
int modify_object(int id, GraphicObject obj);
void clear_all_objects(void);
void render_scene(char canvas[HEIGHT][WIDTH]);
GraphicObject* get_object_by_id(int id);
void list_objects(void);
int read_int(const char* prompt, int min, int max);
GraphicObject prompt_object_data(ObjectType type);
// ==========================================
// CANVAS IMPLEMENTATION
// ==========================================
void clear_canvas(char canvas[HEIGHT][WIDTH]) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            canvas[y][x] = CHAR_BG;
        }
    }
}
void set_pixel(char canvas[HEIGHT][WIDTH], int x, int y) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        canvas[y][x] = CHAR_FG;
    }
}
void display_canvas(char canvas[HEIGHT][WIDTH]) {
    // Print column header tens digits
    printf("   ");
    for (int x = 0; x < WIDTH; x++) {
        if (x % 10 == 0) {
            printf("%d", x / 10);
        } else {
            printf(" ");
        }
    }
    printf("\n");
    // Print column header ones digits
    printf("   ");
    for (int x = 0; x < WIDTH; x++) {
        printf("%d", x % 10);
    }
    printf("\n");
    // Print top border
    printf("  +");
    for (int x = 0; x < WIDTH; x++) printf("-");
    printf("+\n");
    // Print each row with index
    for (int y = 0; y < HEIGHT; y++) {
        printf("%02d|", y);
        for (int x = 0; x < WIDTH; x++) {
            putchar(canvas[y][x]);
        }
        printf("|\n");
    }
    // Print bottom border
    printf("  +");
    for (int x = 0; x < WIDTH; x++) printf("-");
    printf("+\n");
}
// ==========================================
// GEOMETRY ALGORITHMS
// ==========================================
void draw_line(char canvas[HEIGHT][WIDTH], int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = -abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    int e2;
    while (1) {
        set_pixel(canvas, x1, y1);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x1 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y1 += sy;
        }
    }
}
void draw_rectangle(char canvas[HEIGHT][WIDTH], int x, int y, int w, int h) {
    if (w <= 0 || h <= 0) return;
    
    // Top & Bottom lines
    draw_line(canvas, x, y, x + w - 1, y);
    draw_line(canvas, x, y + h - 1, x + w - 1, y + h - 1);
    
    // Left & Right lines
    draw_line(canvas, x, y, x, y + h - 1);
    draw_line(canvas, x + w - 1, y, x + w - 1, y + h - 1);
}
void draw_circle(char canvas[HEIGHT][WIDTH], int cx, int cy, int r) {
    if (r < 0) return;
    if (r == 0) {
        set_pixel(canvas, cx, cy);
        return;
    }
    
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    
    while (y >= x) {
        set_pixel(canvas, cx + x, cy + y);
        set_pixel(canvas, cx - x, cy + y);
        set_pixel(canvas, cx + x, cy - y);
        set_pixel(canvas, cx - x, cy - y);
        set_pixel(canvas, cx + y, cy + x);
        set_pixel(canvas, cx - y, cy + x);
        set_pixel(canvas, cx + y, cy - x);
        set_pixel(canvas, cx - y, cy - x);
        
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}
void draw_triangle(char canvas[HEIGHT][WIDTH], int x1, int y1, int x2, int y2, int x3, int y3) {
    draw_line(canvas, x1, y1, x2, y2);
    draw_line(canvas, x2, y2, x3, y3);
    draw_line(canvas, x3, y3, x1, y1);
}
// ==========================================
// EDITOR DATABASE OPERATIONS
// ==========================================
int add_object(GraphicObject obj) {
    if (object_count >= MAX_OBJECTS) {
        return -1;
    }
    obj.id = next_id++;
    objects[object_count] = obj;
    object_count++;
    return obj.id;
}
int delete_object(int id) {
    int found_idx = -1;
    for (int i = 0; i < object_count; i++) {
        if (objects[i].id == id) {
            found_idx = i;
            break;
        }
    }
    
    if (found_idx == -1) {
        return 0;
    }
    // Shift remaining elements
    for (int i = found_idx; i < object_count - 1; i++) {
        objects[i] = objects[i + 1];
    }
    object_count--;
    return 1;
}
int modify_object(int id, GraphicObject new_obj) {
    for (int i = 0; i < object_count; i++) {
        if (objects[i].id == id) {
            new_obj.id = id; // Keep original ID
            objects[i] = new_obj;
            return 1;
        }
    }
    return 0;
}
void clear_all_objects(void) {
    object_count = 0;
}
GraphicObject* get_object_by_id(int id) {
    for (int i = 0; i < object_count; i++) {
        if (objects[i].id == id) {
            return &objects[i];
        }
    }
    return NULL;
}
void render_scene(char canvas[HEIGHT][WIDTH]) {
    clear_canvas(canvas);
    
    for (int i = 0; i < object_count; i++) {
        GraphicObject obj = objects[i];
        switch (obj.type) {
            case OBJ_LINE:
                draw_line(canvas, obj.data.line.x1, obj.data.line.y1, 
                                  obj.data.line.x2, obj.data.line.y2);
                break;
            case OBJ_RECTANGLE:
                draw_rectangle(canvas, obj.data.rect.x, obj.data.rect.y, 
                                       obj.data.rect.w, obj.data.rect.h);
                break;
            case OBJ_CIRCLE:
                draw_circle(canvas, obj.data.circle.cx, obj.data.circle.cy, 
                                     obj.data.circle.r);
                break;
            case OBJ_TRIANGLE:
                draw_triangle(canvas, obj.data.triangle.x1, obj.data.triangle.y1, 
                                      obj.data.triangle.x2, obj.data.triangle.y2, 
                                      obj.data.triangle.x3, obj.data.triangle.y3);
                break;
        }
    }
}
void list_objects(void) {
    if (object_count == 0) {
        printf("  (No objects in scene)\n");
        return;
    }
    for (int i = 0; i < object_count; i++) {
        GraphicObject obj = objects[i];
        printf("  [%d] ", obj.id);
        switch (obj.type) {
            case OBJ_LINE:
                printf("Line: start=(%d, %d), end=(%d, %d)\n", 
                       obj.data.line.x1, obj.data.line.y1, 
                       obj.data.line.x2, obj.data.line.y2);
                break;
            case OBJ_RECTANGLE:
                printf("Rectangle: top-left=(%d, %d), size=%dx%d\n", 
                       obj.data.rect.x, obj.data.rect.y, 
                       obj.data.rect.w, obj.data.rect.h);
                break;
            case OBJ_CIRCLE:
                printf("Circle: center=(%d, %d), radius=%d\n", 
                       obj.data.circle.cx, obj.data.circle.cy, 
                       obj.data.circle.r);
                break;
            case OBJ_TRIANGLE:
                printf("Triangle: A=(%d, %d), B=(%d, %d), C=(%d, %d)\n", 
                       obj.data.triangle.x1, obj.data.triangle.y1, 
                       obj.data.triangle.x2, obj.data.triangle.y2, 
                       obj.data.triangle.x3, obj.data.triangle.y3);
                break;
        }
    }
}
// ==========================================
// INPUT HELPERS
// ==========================================
int read_int(const char* prompt, int min, int max) {
    char buf[128];
    while (1) {
        printf("%s", prompt);
        if (fgets(buf, sizeof(buf), stdin) != NULL) {
            char* endptr;
            buf[strcspn(buf, "\n")] = 0;
            if (strlen(buf) == 0) {
                printf("Input cannot be empty. Please try again.\n");
                continue;
            }
            long parsed = strtol(buf, &endptr, 10);
            if (endptr != buf && *endptr == '\0' && parsed >= min && parsed <= max) {
                return (int)parsed;
            }
        }
        printf("Invalid input. Please enter an integer between %d and %d.\n", min, max);
    }
}
GraphicObject prompt_object_data(ObjectType type) {
    GraphicObject obj;
    obj.type = type;
    obj.id = 0;
    
    switch (type) {
        case OBJ_LINE:
            printf("\n--- Draw Line ---\n");
            obj.data.line.x1 = read_int("Enter X1 (start col, 0-59): ", 0, WIDTH - 1);
            obj.data.line.y1 = read_int("Enter Y1 (start row, 0-19): ", 0, HEIGHT - 1);
            obj.data.line.x2 = read_int("Enter X2 (end col, 0-59):   ", 0, WIDTH - 1);
            obj.data.line.y2 = read_int("Enter Y2 (end row, 0-19):   ", 0, HEIGHT - 1);
            break;
            
        case OBJ_RECTANGLE:
            printf("\n--- Draw Rectangle ---\n");
            obj.data.rect.x = read_int("Enter Top-Left X (0-59):  ", 0, WIDTH - 1);
            obj.data.rect.y = read_int("Enter Top-Left Y (0-19):  ", 0, HEIGHT - 1);
            obj.data.rect.w = read_int("Enter Width (1-60):       ", 1, WIDTH);
            obj.data.rect.h = read_int("Enter Height (1-20):      ", 1, HEIGHT);
            break;
            
        case OBJ_CIRCLE:
            printf("\n--- Draw Circle ---\n");
            obj.data.circle.cx = read_int("Enter Center X (0-59): ", 0, WIDTH - 1);
            obj.data.circle.cy = read_int("Enter Center Y (0-19): ", 0, HEIGHT - 1);
            obj.data.circle.r  = read_int("Enter Radius (0-30):   ", 0, 30);
            break;
            
        case OBJ_TRIANGLE:
            printf("\n--- Draw Triangle ---\n");
            obj.data.triangle.x1 = read_int("Enter X1 (0-59): ", 0, WIDTH - 1);
            obj.data.triangle.y1 = read_int("Enter Y1 (0-19): ", 0, HEIGHT - 1);
            obj.data.triangle.x2 = read_int("Enter X2 (0-59): ", 0, WIDTH - 1);
            obj.data.triangle.y2 = read_int("Enter Y2 (0-19): ", 0, HEIGHT - 1);
            obj.data.triangle.x3 = read_int("Enter X3 (0-59): ", 0, WIDTH - 1);
            obj.data.triangle.y3 = read_int("Enter Y3 (0-19): ", 0, HEIGHT - 1);
            break;
    }
    return obj;
}
// ==========================================
// ENTRY POINT
// ==========================================
int main(void) {
    int choice = 0;
    clear_canvas(canvas);
    
    printf("=========================================\n");
    printf("      WELCOME TO THE 2D GRAPHICS EDITOR   \n");
    printf("=========================================\n");
    printf("Draw vector shapes on a %dx%d canvas.\n", WIDTH, HEIGHT);
    printf("Shapes are rendered as '%c' on a '%c' background.\n", CHAR_FG, CHAR_BG);
    
    while (1) {
        render_scene(canvas);
        
        printf("\n==================== WORKSPACE ====================\n");
        display_canvas(canvas);
        printf("===================================================\n");
        
        printf("Active Objects:\n");
        list_objects();
        printf("---------------------------------------------------\n");
        
        printf("Menu Options:\n");
        printf("  1. Add Line\n");
        printf("  2. Add Rectangle\n");
        printf("  3. Add Circle\n");
        printf("  4. Add Triangle\n");
        printf("  5. Modify Object\n");
        printf("  6. Delete Object\n");
        printf("  7. Clear Canvas (Delete All)\n");
        printf("  8. Exit\n");
        
        choice = read_int("Select an option (1-8): ", 1, 8);
        
        if (choice == 8) {
            printf("\nExiting editor. Thank you!\n");
            break;
        }
        
        if (choice >= 1 && choice <= 4) {
            ObjectType type = (ObjectType)choice;
            GraphicObject obj = prompt_object_data(type);
            int id = add_object(obj);
            if (id != -1) {
                printf("\n>>> Successfully added shape! (Assigned ID: %d)\n", id);
            } else {
                printf("\n>>> Error: Canvas is full of objects (max %d).\n", MAX_OBJECTS);
            }
        } 
        else if (choice == 5) {
            printf("\n--- Modify Object ---\n");
            int id = read_int("Enter Object ID to modify: ", 1, 10000);
            GraphicObject* existing = get_object_by_id(id);
            if (existing == NULL) {
                printf("\n>>> Error: Object with ID %d not found.\n", id);
            } else {
                printf("Selected object is of type: ");
                switch (existing->type) {
                    case OBJ_LINE: printf("Line\n"); break;
                    case OBJ_RECTANGLE: printf("Rectangle\n"); break;
                    case OBJ_CIRCLE: printf("Circle\n"); break;
                    case OBJ_TRIANGLE: printf("Triangle\n"); break;
                }
                
                printf("Choose new shape type:\n");
                printf("  1. Line\n");
                printf("  2. Rectangle\n");
                printf("  3. Circle\n");
                printf("  4. Triangle\n");
                int new_type_choice = read_int("Select type (1-4): ", 1, 4);
                
                GraphicObject new_obj = prompt_object_data((ObjectType)new_type_choice);
                if (modify_object(id, new_obj)) {
                    printf("\n>>> Successfully modified Object %d!\n", id);
                } else {
                    printf("\n>>> Error: Failed to modify Object %d.\n", id);
                }
            }
        } 
        else if (choice == 6) {
            printf("\n--- Delete Object ---\n");
            int id = read_int("Enter Object ID to delete: ", 1, 10000);
            if (delete_object(id)) {
                printf("\n>>> Successfully deleted Object %d!\n", id);
            } else {
                printf("\n>>> Error: Object with ID %d not found.\n", id);
            }
        } 
        else if (choice == 7) {
            clear_all_objects();
            printf("\n>>> Cleared all objects from the canvas.\n");
        }
    }
    
    return 0;
}

