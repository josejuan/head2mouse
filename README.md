# head 2 mouse

**Step 1.** print one marker from this ![marker template](https://github.com/josejuan/head2mouse/blob/master/markers.pdf).

**Step 2.** put in your head that marker (I used some plastic glasses, a wire and glue :sunglasses:), something like

![aruco](https://github.com/josejuan/head2mouse/blob/master/img/aruco.png)

**Step 3.** build, you will try to solve some dependencies and download aruco somewhere, ...
```
( ( [ -d ./dist ] && rm -rf ./dist ) || true ) && \
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

--------


