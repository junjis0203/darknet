#include "detector_lib.h"

#include "parser.h"
#include "utils.h"
#include "region_layer.h"

network create_detector(char *datacfg, char *cfgfile, char *weightfile)
{
    // from detector.c/test_detector()
    //list *options = read_data_cfg(datacfg);
    //char *name_list = option_find_str(options, "names", "data/names.list");
    //char **names = get_labels(name_list);

    //image **alphabet = load_alphabet();
    network net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    set_batch_network(&net, 1);
    return net;
}

static list *extract_detections(image im, int num, float thresh, box *boxes, float **probs, int classes)
{
    // from image.c/draw_detections()
    int i;
    list *box_list = make_list();

    for(i = 0; i < num; ++i){
        int class = max_index(probs[i], classes);
        float prob = probs[i][class];
        if(prob > thresh){

	    //int width = im.h * .012;

	    /*
            if(0){
                width = pow(prob, 1./2.)*10+1;
                alphabet = 0;
            }
	    */

            /*
	    printf("%s: %.0f%%\n", names[class], prob*100);
            int offset = class*123457 % classes;
            float red = get_color(2,offset,classes);
            float green = get_color(1,offset,classes);
            float blue = get_color(0,offset,classes);
            float rgb[3];
	    */

            //width = prob*20+2;

	    /*
            rgb[0] = red;
            rgb[1] = green;
            rgb[2] = blue;
	    */
            box b = boxes[i];

            int left  = (b.x-b.w/2.)*im.w;
            int right = (b.x+b.w/2.)*im.w;
            int top   = (b.y-b.h/2.)*im.h;
            int bot   = (b.y+b.h/2.)*im.h;

            if(left < 0) left = 0;
            if(right > im.w-1) right = im.w-1;
            if(top < 0) top = 0;
            if(bot > im.h-1) bot = im.h-1;

	    box *pb = calloc(1, sizeof(box));
	    pb->x = left;
	    pb->y = top;
	    pb->w = right - left;
	    pb->h = bot - top;
	    list_insert(box_list, pb);

	    /*
            draw_box_width(im, left, top, right, bot, width, red, green, blue);
            if (alphabet) {
                image label = get_label(alphabet, names[class], (im.h*.03)/10);
                draw_label(im, top + width, left, label, rgb);
            }
	    */
        }
    }
    return box_list;
}

int detect_object(image im, network net, float thresh, float hier_thresh, box **_boxes)
{
    // from detector.c/test_detector()
    //srand(2222222);
    //clock_t time;
    //char buff[256];
    //char *input = buff;
    int j;
    float nms=.4;
    //while(1){
        //if(filename){
        //    strncpy(input, filename, 256);
        //} else {
        //    printf("Enter Image Path: ");
        //    fflush(stdout);
        //    input = fgets(input, 256, stdin);
        //    if(!input) return;
        //    strtok(input, "\n");
        //}
        //image im = load_image_color(input,0,0);
        image sized = resize_image(im, net.w, net.h);
        layer l = net.layers[net.n-1];

        box *boxes = calloc(l.w*l.h*l.n, sizeof(box));
        float **probs = calloc(l.w*l.h*l.n, sizeof(float *));
        for(j = 0; j < l.w*l.h*l.n; ++j) probs[j] = calloc(l.classes + 1, sizeof(float *));

        float *X = sized.data;
        //time=clock();
        network_predict(net, X);
        //printf("%s: Predicted in %f seconds.\n", input, sec(clock()-time));
        get_region_boxes(l, 1, 1, thresh, probs, boxes, 0, 0, hier_thresh);
        if (l.softmax_tree && nms) do_nms_obj(boxes, probs, l.w*l.h*l.n, l.classes, nms);
        else if (nms) do_nms_sort(boxes, probs, l.w*l.h*l.n, l.classes, nms);
        //draw_detections(im, l.w*l.h*l.n, thresh, boxes, probs, names, alphabet, l.classes);

	// 検出結果を引数で指定されたboxに格納
	list *box_list = extract_detections(im, l.w*l.h*l.n, thresh, boxes, probs, l.classes);
	int n = box_list->size;
	box **array = (box **)list_to_array(box_list);
	*_boxes = calloc(n, sizeof(box));
	for (j = 0; j < n; j++) {
	    // 左辺はboxの配列へのポインタ、右辺はboxポインタの配列
	    (*_boxes)[j] = *array[j];
	}
	free_list_contents(box_list);
	free_list(box_list);

        //save_image(im, "predictions");
        //show_image(im, "predictions");

        //free_image(im);
        free_image(sized);
        free(boxes);
        free_ptrs((void **)probs, l.w*l.h*l.n);
#if 0
#ifdef OPENCV
        cvWaitKey(0);
        cvDestroyAllWindows();
#endif
#endif
        //if (filename) break;
    //}

    return n;
}

void destroy_detector(network net)
{
    free_network(net);
    // free_networkで消してない。他にもmake_network以外で確保しているものがないだろうか
    free(net.seen);
}
