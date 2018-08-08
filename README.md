# head 2 mouse

**Step 1.** print one marker from this ![marker template](https://github.com/josejuan/head2mouse/blob/master/markers.pdf).

**Step 2.** put in your head that marker (I used some plastic glasses, a wire and glue :sunglasses:), something like

![aruco](https://github.com/josejuan/head2mouse/blob/master/img/aruco.png)

**Step 3.** build, you will try to solve some dependencies and download aruco somewhere, ...
```
rm -rf ./dist
mkdir ./dist && \
cd ./dist && \
ARUCO=/home/josejuan/Projects/track cmake .. && \
make && \
cd ..
```

**Step 4.** put your webcam in front and run
```
./dist/head2mouse 0.4 1920 1080 $((1920 / 2)) $((1080 / 2)) 0
```

to calibrate:
1. look to the upper left screen corner and press enter.
1. look to the upper right screen corner and press enter.
1. look to the bottom left screen corner and press enter.
1. look to the bottom right screen corner and press enter.
1. press `m` to activate/deactivate cursor pointer control.

## Results

Now, you can control your cursor pointer using your ~~mind~~ head.

For testing purposes I compared drawing using the mouse and the head:

![aruco](https://github.com/josejuan/head2mouse/blob/master/img/test_mouse.png)
![aruco](https://github.com/josejuan/head2mouse/blob/master/img/test_head.png)

Not so bad, I'm using the mouse for twenty years and my head for twenty minutes.

## About implementation

Using the aruco library is easy track the marker point and take four points for the four screen corners to get from camera the `frustum`:

![aruco](https://github.com/josejuan/head2mouse/blob/master/img/frustumbox.png)

Using a simple `frustum` projection we can estimate the cursor position.

The head could produce tremor (especially if you use a wire to hang the marker) to reduce it I use ![exponential smoothing](https://en.wikipedia.org/wiki/Exponential_smoothing).

