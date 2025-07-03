#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t running = 1;

#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"

#define IFACE "enx9a10e8587884"
#define RX_PATH "/sys/class/net/" IFACE "/statistics/rx_bytes"
#define TX_PATH "/sys/class/net/" IFACE "/statistics/tx_bytes"

#define WIDTH 800
#define HEIGHT 600

void handle_sigint(int sig) {
  (void)sig;
  running = 0;
}

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

void draw_bar(float x, float y, float width, float height, float value,
              float max_value, float r, float g, float b) {
  float bar_width = (value / max_value) * width;
  if (bar_width > width)
    bar_width = width;

  glColor3f(r * 0.4f, g * 0.4f, b * 0.4f);
  glBegin(GL_QUADS);
  glVertex2f(x - 2, y - 2);
  glVertex2f(x + bar_width + 2, y - 2);
  glVertex2f(x + bar_width + 2, y + height + 2);
  glVertex2f(x - 2, y + height + 2);
  glEnd();

  glColor3f(r, g, b);
  glBegin(GL_QUADS);
  glVertex2f(x, y);
  glVertex2f(x + bar_width, y);
  glVertex2f(x + bar_width, y + height);
  glVertex2f(x, y + height);
  glEnd();
}

void draw_text(float x, float y, const char *text, float r, float g, float b) {
  char buffer[99999];
  int num_quads =
      stb_easy_font_print(x, y, (char *)text, NULL, buffer, sizeof(buffer));

  glColor3f(r, g, b);

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(2, GL_FLOAT, 16, buffer);
  glDrawArrays(GL_QUADS, 0, num_quads * 4);
  glDisableClientState(GL_VERTEX_ARRAY);
}

int main() {
  signal(SIGINT, handle_sigint);

  Display *dpy = XOpenDisplay(NULL);
  if (!dpy) {
    fprintf(stderr, "Failed to open display\n");
    return 1;
  }

  Window root = DefaultRootWindow(dpy);
  GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
  XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
  Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

  XSetWindowAttributes swa;
  swa.colormap = cmap;
  swa.event_mask = ExposureMask | KeyPressMask;
  Window win =
      XCreateWindow(dpy, root, 0, 0, WIDTH, HEIGHT, 0, vi->depth, InputOutput,
                    vi->visual, CWColormap | CWEventMask, &swa);
  XMapWindow(dpy, win);
  GLXContext glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
  glXMakeCurrent(dpy, win, glc);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);

  unsigned long long prev_rx = read_bytes(RX_PATH);
  unsigned long long prev_tx = read_bytes(TX_PATH);
  double total_rx_mb = prev_rx / (1024.0 * 1024.0);
  double total_tx_mb = prev_tx / (1024.0 * 1024.0);

  while (running) {
    while (XPending(dpy)) {
      XEvent e;
      XNextEvent(dpy, &e);
      if (e.type == KeyPress) {
        KeySym key = XLookupKeysym(&e.xkey, 0);
        if (key == XK_Escape)
          running = 0;
      }
    }

    usleep(100000);

    unsigned long long curr_rx = read_bytes(RX_PATH);
    unsigned long long curr_tx = read_bytes(TX_PATH);
    double new_total_rx_mb = curr_rx / (1024.0 * 1024.0);
    double new_total_tx_mb = curr_tx / (1024.0 * 1024.0);
    double delta_rx = new_total_rx_mb - total_rx_mb;
    double delta_tx = new_total_tx_mb - total_tx_mb;
    total_rx_mb = new_total_rx_mb;
    total_tx_mb = new_total_tx_mb;

    glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    float bar_max = 2.0f;
    draw_bar(100, 200, 600, 40, delta_rx, bar_max, 0.2f, 1.0f, 0.4f);
    draw_bar(100, 300, 600, 40, delta_tx, bar_max, 0.4f, 0.7f, 1.0f);

    char rx_text[128], tx_text[128], total_text[128];
    snprintf(rx_text, sizeof(rx_text), "RX: %.2f MB/s", delta_rx * 10);
    snprintf(tx_text, sizeof(tx_text), "TX: %.2f MB/s", delta_tx * 10);
    snprintf(total_text, sizeof(total_text), "Total: RX %.2f MB | TX %.2f MB",
             total_rx_mb, total_tx_mb);

    draw_text(110, 190, rx_text, 0.0f, 1.0f, 1.0f);
    draw_text(110, 290, tx_text, 0.0f, 1.0f, 1.0f);
    draw_text(100, 400, total_text, 1.0f, 0.5f, 1.0f);
    draw_text(100, 450, "[ESC to quit]", 1.0f, 0.3f, 1.0f);

    glXSwapBuffers(dpy, win);
  }

shutdown:
  glXMakeCurrent(dpy, None, NULL);
  glXDestroyContext(dpy, glc);
  XDestroyWindow(dpy, win);
  XCloseDisplay(dpy);
  return 0;
}
