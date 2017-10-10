#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

cv::Mat difX(const cv::Mat &X)
{
  cv::Mat R(X.size(), X.type());
  X.colRange(1, X.cols).copyTo(R.colRange(0, X.cols-1));
  X.col(X.cols-1).copyTo(R.col(X.cols-1));
  return R - X;
}

cv::Mat difY(const cv::Mat &X)
{
  cv::Mat R(X.size(), X.type());
  X.rowRange(1, X.rows).copyTo(R.rowRange(0, X.rows - 1));
  X.row(X.rows - 1).copyTo(R.row(X.rows - 1));
  return R - X;
}

cv::Mat divX(const cv::Mat &X)
{
  cv::Mat Px(X.size(), X.type());
  X.col(0).copyTo(Px.col(0));
  X.colRange(0, X.cols-1).copyTo(Px.colRange(1, X.cols));
  cv::Mat R = X - Px;
  X.col(0).copyTo(R.col(0));
  R.col(R.cols-1) = -X.col(X.cols-2);

  return R;
}

cv::Mat divY(const cv::Mat &X)
{
  cv::Mat Px(X.size(), X.type());
  X.row(0).copyTo(Px.row(0));
  X.rowRange(0, X.rows - 1).copyTo(Px.rowRange(1, X.rows));
  cv::Mat R = X - Px;
  X.row(0).copyTo(R.row(0));
  R.row(R.rows - 1) = -X.row(X.rows - 2);

  return R;
}

cv::Mat boxproj(const cv::Mat &X)
{
  cv::Mat dst;
  cv::max(X, 0, dst);
  cv::min(dst, 1, dst);
  return dst;
}

cv::Mat threasholding_l1_w(const cv::Mat &X, const cv::Mat &Y)
{
  cv::Mat signX = X / cv::abs(X);
  cv::Mat normX;
  cv::max(0, cv::abs(X) - Y, normX);
  return -signX.mul(normX);
}

cv::Mat theasholding_RS(const cv::Mat &X, const cv::Mat &Y, double lambda, double beta)
{
  cv::Mat R, S;
  cv::Mat normRS;
  cv::sqrt(X.mul(X) + Y.mul(Y), normRS);
  cv::Mat Weighted = cv::max(-(lambda/beta)/normRS+1, 0);
  R = -X.mul(Weighted);
  S = -Y.mul(Weighted);
  cv::Mat RS(R.rows + S.rows, R.cols, R.type());
  R.copyTo(RS.rowRange(0, R.rows));
  S.copyTo(RS.rowRange(R.rows, RS.rows));

  return RS;
}

int main(int argc, char* argv[])
{ 
  if(argc != 2) {
    std::cout << "Usage: L0TV [input filename]" << std::endl;
    return 0;
  }

  cv::Mat src(cv::imread(argv[1]));
  cv::cvtColor(src, src, cv::COLOR_BGR2GRAY);

  //  ƒmƒCƒY‚ÌÝ’è
  double noise = 0.0;
  cv::Mat img;
  while(1) {
    src.copyTo(img);

    for (auto it = img.begin<uchar>(), end = img.end<uchar>(); it != end; it++) {
      double h = (double)rand() / RAND_MAX;
      if (h < noise) (*it) = 0;
      if (h > 1 - noise) (*it) = 255;
    }
    cv::imshow("img", img);
    int key = cv::waitKey(0);
    if(key == 'q') break;
    if (key == '+' || key == ';') noise += 0.01;
    if (key == '-') noise -= 0.01;
    if (noise < 0) noise = 0;
  }
  cv::destroyWindow("img");

  cv::Mat B;
  img.convertTo(B, CV_32FC1, 1.0/255);

  cv::Mat U = B.clone();
  cv::Mat dx = difX(U);
  cv::Mat dy = difY(U);
  cv::Mat Kub = U - B;
  cv::Mat V = cv::Mat::ones(B.size(), B.type());
  cv::Mat Z = Kub.clone();
  cv::Mat R = difX(U);
  cv::Mat S = difY(U);

  cv::Mat piz = cv::Mat::zeros(B.size(), B.type());
  cv::Mat pir = cv::Mat::zeros(B.size(), B.type());
  cv::Mat pis = cv::Mat::zeros(B.size(), B.type());
  cv::Mat piv = cv::Mat::zeros(B.size(), B.type());

  double gamma = 0.5 * (1 + sqrt(5.0));
  double alpha = 10;
  double beta = 10;
  double rho = 10;
  double ratio = 3;

  const double lambda = 8;
  const double LargestEig = 1.0;
  const int max_itr = 100;

  cv::imshow("U", U);
  cv::imshow("V", V);
  cv::waitKey(0);

  for (int itr = 0; itr<max_itr; itr++) {
    //Update Z
    cv::Mat cof_A = rho * V.mul(V) + alpha ;
    cv::Mat cof_B = -alpha * Kub - piz ;
    cv::Mat cof_C = piv.mul(V) ;
    Z = threasholding_l1_w(cof_B / cof_A, cof_C / cof_A);

    // UpdateRS
    cv::Mat RS = theasholding_RS(-pir / beta - dx, -pis / beta - dy, lambda, beta);
    RS.rowRange(0, U.rows).copyTo(R);
    RS.rowRange(U.rows, RS.rows).copyTo(S);

    //  Update U
    cv::Mat g1 = alpha * (Kub-Z) + piz;
    cv::Mat g3 = -beta*divX(dx) + divX(-pir + beta*R);
    cv::Mat g4 = -beta*divY(dy) + divY(-pis + beta*S);
    cv::Mat gradU = g1 + g3 + g4;
    double Lip = beta*4 + beta*4 + alpha*LargestEig;
    U = boxproj(U - gradU/Lip);
    dx = difX(U);
    dy = difY(U);
    Kub = U - B;

    //  Update V
    cof_A = rho * Z.mul(Z) + 1.0e-10;
    cof_B = piv.mul(abs(Z)) - 1;
    cof_C = -cof_B / cof_A;
    V = boxproj(cof_C);

    cv::Mat Kubz = Kub - Z;
    cv::Mat dxR = dx - R;
    cv::Mat dyS = dy - S;
    cv::Mat VabsZ = V.mul(abs(Z));

    piz = piz + gamma * alpha * Kubz;
    pir = pir + gamma * beta * dxR;
    pis = pis + gamma * beta * dyS;
    piv = piv + gamma * rho * VabsZ;

    double r1 = cv::norm(Kubz);
    double r2 = cv::norm(dxR) + cv::norm(dyS);
    double r3 = cv::norm(VabsZ);

    if ((itr+1) % 30 == 0) {
      if (r1 > r2 && r1 > r3) alpha = alpha * ratio;
      if (r2 > r1 && r2 > r3) beta = beta * ratio;
      if (r3 > r1 && r3 > r2) rho = rho * ratio;
    }

    printf("%d/%d\n", itr, max_itr);
    cv::imshow("U", U);
    cv::imshow("V", V);
    cv::waitKey(1);
  }

  int key = cv::waitKey(0);

  if(key != 'q') {
    cv::destroyWindow("U");
    cv::destroyWindow("V");
  }

  return 0;
}
