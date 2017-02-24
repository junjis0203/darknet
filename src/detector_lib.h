#ifndef DETECTOR_LIB_H
#define DETECTOR_LIB_H

#include "network.h"
#include "image.h"
#include "box.h"

network create_detector(char *datacfg, char *cfgfile, char *weightfile);
int detect_object(image im, network net, float thresh, float hier_thresh, box **boxes);
void destroy_detector(network net);

#endif
