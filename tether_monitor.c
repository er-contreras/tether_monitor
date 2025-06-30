#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define IFACE "enx9a10e8587884"
#define RX_PATH "/sys/class/net/" IFACE "/statistics/rx_bytes"
#define TX_PATH "/sys/class/net/" IFACE "/statistics/tx_bytes"

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

int main() {
  unsigned long long prev_rx = read_bytes(RX_PATH);
  unsigned long long prev_tx = read_bytes(TX_PATH);

  while (1) {
    sleep(1);

    unsigned long long curr_rx = read_bytes(RX_PATH);
    unsigned long long curr_tx = read_bytes(TX_PATH);

    double total_rx_mb = curr_rx / (1024.0 * 1024.0);
    double total_tx_mb = curr_tx / (1024.0 * 1024.0);
    double delta_rx_mb = (curr_rx - prev_rx) / (1024.0 * 1024.0);
    double delta_tx_mb = (curr_tx - prev_tx) / (1024.0 * 1024.0);

    printf("RX: %.2f MB (+%.2f) | TX: %.2f MB (+%.2f)\n", total_rx_mb, delta_rx_mb, total_tx_mb, delta_tx_mb);

    prev_rx = curr_rx;
    prev_tx = curr_tx;
  }

  return 0;
}
