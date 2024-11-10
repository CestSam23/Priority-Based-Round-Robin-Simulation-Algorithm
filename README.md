Bienvenidas Gatas Rompe Kernels

Este será el repositorio donde iremos trabajando.

Algoritmo: Priority Based, round robin scheduling algotithm

General Structures:

struct Process{
  pid_t id;
  int cpuBurst;
  int tCompletition;
  int tWaiting;
  int priority;
  char name[5]
}
