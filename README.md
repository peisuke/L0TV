# Implementation of "L0TV: a new method for image restoration in the presence of impulse noise"

This repository is reproduced code of "L0TV: a new method for image restoration in the 
presence of impulse noise" presented in CVPR2015. This method is for image restoration 
in the presence of impulse noise. 

The arXiv link is [here](https://www.cv-foundation.org/openaccess/content_cvpr_2015/papers/Yuan_L0TV_A_New_2015_CVPR_paper.pdf)

## Build and run

```
$ mkdir build
$ cmake .. && make
$ cd ..
$ ./L0TV Lena.png
```

## Example

After run the script the image is shown in your screen.

<img src="https://user-images.githubusercontent.com/14243883/31289510-ec2ef160-ab03-11e7-8246-83ecf02d2311.jpg" width="320px">

Then, push '+' key and add noise, or push '-' key an reduce noise.　The sample image added noise, is shown as below.

<img src="https://user-images.githubusercontent.com/14243883/31289507-ec0a6afc-ab03-11e7-8c32-3c831e9890d6.jpg" width="320px">

To run the algorithm, push 'q' key and ENTER. The noise image is here.

<img src="https://user-images.githubusercontent.com/14243883/31289508-ec10ca50-ab03-11e7-9f7b-76ee655e781a.jpg" width="320px">

The result image is shown as below. Almost noise was erased.

<img src="https://user-images.githubusercontent.com/14243883/31289509-ec1638c8-ab03-11e7-981e-cfd04fbd6373.jpg" width="320px">
