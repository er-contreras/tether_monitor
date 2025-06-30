// Compile with:
// gcc tether_hud_cyberpunk.c -lglfw -lGL -lm -o tether_hud_cyberpunk

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// =======================
// Begin: stb_easy_font implementation
// (Public Domain, minimal text renderer)
// -----------------------
#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"
// =======================
// End: stb_easy_font implementation

// =======================
// Configuration & Paths
// =======================
#define IFACE "enx9a10e8587884"
#define RX_PATH "/sys/class/net/" IFACE "/statistics/rx_bytes"
#define TX_PATH "/sys/class/net/" IFACE "/statistics/tx_bytes"

#define WIDTH 800
#define HEIGHT 600

// =======================
// Utility: Read Bytes from a File
// =======================
unsigned long long read_bytes(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("fopen");
        exit(1);
    }
    unsigned long long bytes = 0;
    fscanf(fp, "%llu", &bytes);
    fclose(fp);
    return bytes;
}

// =======================
// Utility: Draw a filled bar.
// (x, y) is the top-left corner; "width" is full bar width.
void draw_bar(float x, float y, float width, float height,
              float value, float max_value, float r, float g, float b) {
    float bar_width = (value / max_value) * width;
    if (bar_width > width) bar_width = width;
    
    // Draw a neon glow-like effect (simple double pass):
    glColor3f(r * 0.5f, g * 0.5f, b * 0.5f); // darker glow underneath
    glBegin(GL_QUADS);
      glVertex2f(x - 2, y - 2);
      glVertex2f(x + bar_width + 2, y - 2);
      glVertex2f(x + bar_width + 2, y + height + 2);
      glVertex2f(x - 2, y + height + 2);
    glEnd();
    
    // Draw the bright bar
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
      glVertex2f(x, y);
      glVertex2f(x + bar_width, y);
      glVertex2f(x + bar_width, y + height);
      glVertex2f(x, y + height);
    glEnd();
}

// =======================
// Utility: Render text using stb_easy_font
// =======================
void draw_text(float x, float y, const char *text, float r, float g, float b) {
    char buffer[99999]; // large enough for our text
    int num_quads = stb_easy_font_print(x, y, (char *)text, NULL, buffer, sizeof(buffer));
    
    // Set text color
    glColor3f(r, g, b);
    
    // Draw quads
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 16, buffer);
    glDrawArrays(GL_QUADS, 0, num_quads * 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

// =======================
// Main Function
// =======================
int main() {
    if (!glfwInit())
        return -1;
    
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Tether HUD - Cyberpunk Edition", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    // Set up orthographic projection: (0,0) top-left, (WIDTH,HEIGHT) bottom-right.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    
    // Initial readings
    unsigned long long prev_rx = read_bytes(RX_PATH);
    unsigned long long prev_tx = read_bytes(TX_PATH);
    
    // Total usage counters (in MB)
    double total_rx_mb = prev_rx / (1024.0 * 1024.0);
    double total_tx_mb = prev_tx / (1024.0 * 1024.0);
    
    while (!glfwWindowShouldClose(window)) {
        // Delay ~0.5 seconds for a visible update.
        usleep(500000);
        
        unsigned long long curr_rx = read_bytes(RX_PATH);
        unsigned long long curr_tx = read_bytes(TX_PATH);
        
        double new_total_rx_mb = curr_rx / (1024.0 * 1024.0);
        double new_total_tx_mb = curr_tx / (1024.0 * 1024.0);
        
        double delta_rx_mb = new_total_rx_mb - total_rx_mb;
        double delta_tx_mb = new_total_tx_mb - total_tx_mb;
        
        // Update total counters.
        total_rx_mb = new_total_rx_mb;
        total_tx_mb = new_total_tx_mb;
        
        // Clear with a dark, cyberpunk background.
        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Define maximum for bar scaling (e.g., 2 MB per 0.5 sec = full bar).
        float bar_max = 2.0f;
        
        // Draw RX and TX bars with neon colors.
        // RX bar (greenish)
        draw_bar(100, 200, 600, 40, delta_rx_mb, bar_max, 0.1f, 1.0f, 0.3f);
        // TX bar (bluish)
        draw_bar(100, 300, 600, 40, delta_tx_mb, bar_max, 0.3f, 0.7f, 1.0f);
        
        // Draw outlines for bars (optional, for extra cyberpunk look).
        glColor3f(0.8f, 0.8f, 0.8f);
        glLineWidth(2);
        glBegin(GL_LINE_LOOP);
            glVertex2f(100, 200);
            glVertex2f(700, 200);
            glVertex2f(700, 240);
            glVertex2f(100, 240);
        glEnd();
        glBegin(GL_LINE_LOOP);
            glVertex2f(100, 300);
            glVertex2f(700, 300);
            glVertex2f(700, 340);
            glVertex2f(100, 340);
        glEnd();
        
        // Prepare text strings (cyberpunk neon text).
        char rx_text[128], tx_text[128], total_text[128];
        snprintf(rx_text, sizeof(rx_text), "RX: %.2f MB/s", delta_rx_mb * 2); // convert to MB/s
        snprintf(tx_text, sizeof(tx_text), "TX: %.2f MB/s", delta_tx_mb * 2);
        snprintf(total_text, sizeof(total_text), "Total: RX %.2f MB | TX %.2f MB", total_rx_mb, total_tx_mb);
        
        // Draw text labels in a neon color (bright cyan).
        draw_text(110, 190, rx_text, 0.0f, 1.0f, 1.0f);
        draw_text(110, 290, tx_text, 0.0f, 1.0f, 1.0f);
        draw_text(100, 400, total_text, 1.0f, 0.5f, 1.0f);
        
        // Additional cyberpunk style: render a footer instruction.
        draw_text(100, 450, "[ESC to quit]", 1.0f, 0.2f, 1.0f);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}

