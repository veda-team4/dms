#ifndef FUNCTIONS_H
#define FUNCTIONS_H

int startpage();
int camsetpage();
int calibratepage(double* thresholdEAR, double* openedEAR, double* closedEAR);
int monitorpage(double thresholdEAR);
int reportpage();

#endif // FUNCTIONS_H