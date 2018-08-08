# head 2 mouse

Print one marker from this ![marker template](https://github.com/josejuan/head2mouse/blob/master/aruco_)
![aruco](https://github.com/josejuan/head2mouse/blob/master/img/aruco.png)

```
( ( [ -d ./dist ] && rm -rf ./dist ) || true ) && \
mkdir ./dist && \
cd ./dist && \
ARUCO=/home/josejuan/Projects/track cmake .. && \
make && \
cd ..
```



```
./dist/head2mouse 0.4 1920 1080 $((1920 / 2)) $((1080 / 2)) 0
```

