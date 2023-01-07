# ms-matrix.lv2

A mid-sid encoder with DbFS peak and rms meters.<bg/>

Based on the (great) work of Robin Gareus: https://github.com/x42/x42-plugins<br/>
![alt text](https://github.com/lherg/ms-matrix.lv2/blob/main/png/ms-matrix.png)<br/>

The high pass filter is not yet implemented, but will be soon...<bg/>

It can also decode:<bg/>

![alt text](https://github.com/lherg/ms-matrix.lv2/blob/main/png/enc-dec-raysession.png)<br/>
![alt text](https://github.com/lherg/ms-matrix.lv2/blob/main/png/enc-dec-2msmatrix.png)<br/>
![alt text](https://github.com/lherg/ms-matrix.lv2/blob/main/png/enc-dec-scope.png)<br/>

# Installation
First clone robtk<bg/>
```
git clone https://github.com/x42/robtk.git
```

Compile<bg/>
```
make
```

Install to ~/.lv2<bg/>
```
make install
```
Run<bg/>
```
jalv.gtk http://github.com/lherg/lv2/eg-amp-fgain-ano
```

```
or the jack client<bg/>
bin/x42-ms_matrix
```

Remove<bg/>
```
make clean
```
