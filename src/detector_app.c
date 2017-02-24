#include "detector_lib.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
  if (argc < 4) {
    printf("Usage: %s datacfg cfgfile weightfile imgfile\n", argv[0]);
    return 1;
  }

  network net = create_detector(argv[1], argv[2], argv[3]);

  image im = load_image_color(argv[4], 0, 0);
  float thresh = .24;
  float hier_thresh = .5;
  box *boxes = NULL;
  int n = detect_object(im, net, thresh, hier_thresh, &boxes);

  int i;
  for (i = 0; i < n; i++) {
    printf("%d: x:%g, y:%g, w:%g, h:%g\n",
	   i, boxes[i].x, boxes[i].y, boxes[i].w, boxes[i].h);
  }

  free(boxes);
  free_image(im);

  destroy_detector(net);

  return 0;
}
