#include <time.h>
#include <aruco.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define W_CAPTURE "wcapture"
#define W_DRAW    "wdraw"

using namespace std;
using namespace cv;
using namespace aruco;

// calibration points
int iCal = 0;
Point2d cal[4];

// calibration parameters
double h;
double M;
double w;
double d;

// time statistics (milliseconds)
#define STATISTICS_ESTF 0.4
double captureTime = 0;
double arucoTime = 0;
double fps = 0;

// current clock in milliseconds
long tms() {
  return (1000 * clock()) / CLOCKS_PER_SEC;
}

void compute_parameters() {
  h = (cal[2].y + cal[3].y - cal[0].y - cal[1].y) / 2;
  M = (cal[1].x - cal[0].x) / 2;
  w = (cal[3].x - cal[2].x) / 2;
  d = (M * h) / (w - M);
}

int main(int argc, char **argv) {

  if(argc != 7) {
    fprintf(stderr, "usage: %s <estf> <s.w> <s.h> <c.w> <c.h> <stats> [ calibrate | run <p1> ]\n", argv[0]);
    fprintf(stderr, "    <estf>   exponential smoothing tremor factor (0.6 is a good factor)\n");
    fprintf(stderr, "    <s.w>    screen width to map in pixels\n");
    fprintf(stderr, "    <s.h>    screen height to map in pixels\n");
    fprintf(stderr, "    <c.w>    capture width resolution in pixels\n");
    fprintf(stderr, "    <c.h>    capture height resolution in pixels\n");
    fprintf(stderr, "    <stats>  show statistics (0|1)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "    calibrate  run in calibration mode\n");
    fprintf(stderr, "         1. show to the up left screen corner (probably 5cm above) and press Enter\n");
    fprintf(stderr, "         2. show to the up right screen corner (probably 5cm above) and press Enter\n");
    fprintf(stderr, "         3. show to the bottom left screen corner (probably 10cm above) and press Enter\n");
    fprintf(stderr, "         4. show to the bottom right screen corner (probably 10cm above) and press Enter\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "         p     enable/disable painting mode, useful to fill the used camera area\n");
    fprintf(stderr, "         m     enable/disable mouse control, useful to test parameters\n");
    fprintf(stderr, "         ESC   exit\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "    run        enter in mouse control (no way to exit)\n");
    exit(1);
  }

  double estf         = atof(argv[1]);
  int screen_width    = atoi(argv[2]);
  int screen_height   = atoi(argv[3]);
  int capture_width   = atoi(argv[4]);
  int capture_height  = atoi(argv[5]);
  bool showstats      = atoi(argv[6]) == 1;

  // last keyboard press
  long lastKeyTime = 0;
  // time to next press (milliseconds)
#define KEY_TIME 100

  // mouse control enabled
  bool mousify = false;

  // drawing enabled
  bool drawing = false;

  Display *dpy;
  Window root_window;

  dpy = XOpenDisplay(0);
  root_window = XRootWindow(dpy, 0);

  VideoCapture capture;
  capture.open(0);
  if(!capture.isOpened()) {
    return -1;
  }

  capture.set(CV_CAP_PROP_FRAME_WIDTH,  capture_width);
  capture.set(CV_CAP_PROP_FRAME_HEIGHT, capture_height);
  namedWindow(W_CAPTURE, WINDOW_AUTOSIZE);
  namedWindow(W_DRAW, WINDOW_AUTOSIZE);

  MarkerDetector TheMarkerDetector;
  TheMarkerDetector.setDictionary("ARUCO_MIP_36h12");
  TheMarkerDetector.setDetectionMode(aruco::DM_FAST);

  // previous estimated position
  double tx = 0.0;
  double ty = 0.0;

  Mat *draw = NULL;

  for(;;) {
    Mat captured;

    if(showstats) {
      static long fps_ = 0;
      static long t = 0;
      fps_ += 1;
      long f = tms();
      if(f > t) {
        t = f + 1000;
        fps = (1.0 - STATISTICS_ESTF) * fps + STATISTICS_ESTF * fps_;
        fps_ = 0;

        double mleft    = (cal[0].x + cal[2].x) / 2;
        double mright   = (cal[1].x + cal[3].x) / 2;
        double mtop     = (cal[0].y + cal[1].y) / 2;
        double mbottom  = (cal[2].y + cal[3].y) / 2;
        fprintf(stderr, "STATS: %f fps, capture %f mS, aruco %f mS\n", fps, captureTime, arucoTime);
      }
    }

    {
      long t = tms();
      capture >> captured;
      t = tms() - t;
      captureTime = (1.0 - STATISTICS_ESTF) * captureTime + STATISTICS_ESTF * t;
    }
    if(captured.empty()) {
      fprintf(stderr, "%s: cannot capture frame!\n", argv[0]);
      exit(1);
    }

    static Point2f q = Point2f(0, 0);
    {
      long t = tms();
      vector<aruco::Marker> detected_markers = TheMarkerDetector.detect(captured);
      for (auto m: detected_markers) {
        m.draw(captured, Scalar(0, 0, 255), 1);
        q = m.getCenter();
      }
      t = tms() - t;
      arucoTime = (1.0 - STATISTICS_ESTF) * arucoTime + STATISTICS_ESTF * t;
    }

    if(drawing) {
      if(draw == NULL)
        draw = new Mat(captured.rows, captured.cols, CV_8UC3, Scalar(0, 0, 0));
      circle(*draw, q, 1, Scalar(255, 255, 255), -2);
    }

    if(iCal == 4) {
      double wy = ((q.y - (cal[0].y + cal[1].y) / 2) * (d + h)) / (d + q.y);
      double wx = ((q.x - (cal[0].x + cal[1].x + cal[2].x + cal[3].x) / 4) * (d + h - wy)) / d;
      int my = (int) ((1.414 * wy * screen_height) / h);
      int mx = (int) ((wx * screen_width) / (2 * w) + screen_width / 2);

      tx = (1.0 - estf) * tx + estf * mx;
      ty = (1.0 - estf) * ty + estf * my;

      if(mousify) {
        XSelectInput(dpy, root_window, KeyReleaseMask);
        XWarpPointer(dpy, None, root_window, 0, 0, 0, 0, (int) tx, (int) ty);
        XFlush(dpy);
      }
    }
    circle(captured, q, 5, Scalar(255, 100, 100), -2);

    imshow(W_CAPTURE, captured);
    if(draw != NULL) imshow(W_DRAW, *draw);

    long t = (1000 * clock()) / CLOCKS_PER_SEC;
    int key = waitKey(1);
    if(key > 0 && lastKeyTime < t) {
      lastKeyTime = t + KEY_TIME;
      if(key == 27) {
        fprintf(stderr, "%s: exiting\n", argv[0]);
        break;
      }
      if(key == 109) {
        mousify = !mousify;
        fprintf(stderr, "%s: mousify is %s\n", argv[0], mousify ? "on" : "off");
      }
      if(key == 112) {
        drawing= !drawing;
        fprintf(stderr, "%s: drawing is %s\n", argv[0], drawing ? "on" : "off");
      }
      if(key == 13 && iCal < 4) {
        cal[iCal++] = q;

        if(iCal == 4) {
          fprintf(stderr, "%s: all points calibrated\n", argv[0]);
          compute_parameters();
        } else {
          fprintf(stderr, "%s: point %i calibrated\n", argv[0], iCal);
        }
      }
      fprintf(stderr, "%s: key %i\n", argv[0], key);
    }
  }
  fprintf(stderr, "VALUES: %f %f %f %f %f %f %f %f\n", cal[0].x, cal[0].y, cal[1].x, cal[1].y, cal[2].x, cal[2].y, cal[3].x, cal[3].y);
  return 0;
}
