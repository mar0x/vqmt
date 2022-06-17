//
// Copyright(c) Multimedia Signal Processing Group (MMSPG),
//              Ecole Polytechnique Fédérale de Lausanne (EPFL)
//              http://mmspg.epfl.ch
//              Zhou Wang
//              https://ece.uwaterloo.ca/~z70wang/
// All rights reserved.
// Author: Philippe Hanhart (philippe.hanhart@epfl.ch)
//
// Permission is hereby granted, without written agreement and without
// license or royalty fees, to use, copy, modify, and distribute the
// software provided and its documentation for research purpose only,
// provided that this copyright notice and the original authors' names
// appear on all copies and supporting documentation.
// The software provided may not be commercially distributed.
// In no event shall the Ecole Polytechnique Fédérale de Lausanne (EPFL)
// be liable to any party for direct, indirect, special, incidental, or
// consequential damages arising out of the use of the software and its
// documentation.
// The Ecole Polytechnique Fédérale de Lausanne (EPFL) specifically
// disclaims any warranties.
// The software provided hereunder is on an "as is" basis and the Ecole
// Polytechnique Fédérale de Lausanne (EPFL) has no obligation to provide
// maintenance, support, updates, enhancements, or modifications.
//

//
// This is an OpenCV implementation of the original Matlab implementation
// from Nikolay Ponomarenko available from http://live.ece.utexas.edu/research/quality/.
// Please refer to the following papers:
// - Z. Wang, A.C. Bovik, H.R. Sheikh, and E.P. Simoncelli, "Image quality
//   assessment: from error visibility to structural similarity," IEEE
//   Transactions on Image Processing, vol. 13, no. 4, pp. 600–612, April 2004.
//

#include "SSIM.hpp"

const float SSIM::C1 = 6.5025f;
const float SSIM::C2 = 58.5225f;

SSIM::SSIM(int h, int w) : Metric(h, w),
  mu1(h, w, CV_32FC3),
  mu2(h, w, CV_32FC3),
  mu1_sq(h, w, CV_32FC3),
  mu2_sq(h, w, CV_32FC3),
  mu1_mu2(h, w, CV_32FC3),
  img1_sq(h, w, CV_32FC3),
  img2_sq(h, w, CV_32FC3),
  img1_img2(h, w, CV_32FC3),
  sigma1_sq(h, w, CV_32FC3),
  sigma2_sq(h, w, CV_32FC3),
  sigma12(h, w, CV_32FC3),
  tmp1(h, w, CV_32FC3),
  tmp2(h, w, CV_32FC3),
  tmp3(h, w, CV_32FC3),
  ssim_map(h, w, CV_32FC3),
  cs_map(h, w, CV_32FC3)
{
}

float SSIM::compute(const cv::Mat& original, const cv::Mat& processed)
{
	cv::Scalar res = computeSSIM(original, processed);
	return float(res.val[0]);
}

cv::Scalar SSIM::computeSSIM(const cv::Mat& img1, const cv::Mat& img2)
{
	// mu1 = filter2(window, img1, 'valid');
	applyGaussianBlur(img1, mu1, 11, 1.5);

	// mu2 = filter2(window, img2, 'valid');
	applyGaussianBlur(img2, mu2, 11, 1.5);

	// mu1_sq = mu1.*mu1;
	cv::multiply(mu1, mu1, mu1_sq);
	// mu2_sq = mu2.*mu2;
	cv::multiply(mu2, mu2, mu2_sq);
	// mu1_mu2 = mu1.*mu2;
	cv::multiply(mu1, mu2, mu1_mu2);

	cv::multiply(img1, img1, img1_sq);
	cv::multiply(img2, img2, img2_sq);
	cv::multiply(img1, img2, img1_img2);

	// sigma1_sq = filter2(window, img1.*img1, 'valid') - mu1_sq;
	applyGaussianBlur(img1_sq, sigma1_sq, 11, 1.5);
	sigma1_sq -= mu1_sq;

	// sigma2_sq = filter2(window, img2.*img2, 'valid') - mu2_sq;
	applyGaussianBlur(img2_sq, sigma2_sq, 11, 1.5);
	sigma2_sq -= mu2_sq;

	// sigma12 = filter2(window, img1.*img2, 'valid') - mu1_mu2;
	applyGaussianBlur(img1_img2, sigma12, 11, 1.5);
	sigma12 -= mu1_mu2;

	// cs_map = (2*sigma12 + C2)./(sigma1_sq + sigma2_sq + C2);
	tmp1 = 2*sigma12 + C2;
	tmp2 = sigma1_sq + sigma2_sq + C2;
	cv::divide(tmp1, tmp2, cs_map);
	// ssim_map = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))./((mu1_sq + mu2_sq + C1).*(sigma1_sq + sigma2_sq + C2));
	tmp3 = 2*mu1_mu2 + C1;
	cv::multiply(tmp1, tmp3, tmp1);
	tmp3 = mu1_sq + mu2_sq + C1;
	cv::multiply(tmp2, tmp3, tmp2);
	cv::divide(tmp1, tmp2, ssim_map);

	// mssim = mean2(ssim_map);
	cv::Scalar ssim_mean = cv::mean(ssim_map);
	double mssim = ssim_mean.val[0];
	// mcs = mean2(cs_map);
	cv::Scalar cs_mean = cv::mean(cs_map);
	double mcs = cs_mean.val[0];

	for (int i = 1; i < img1.channels(); ++i) {
		mssim += ssim_mean.val[1];
		mcs += cs_mean.val[1];
	}
	mssim /= img1.channels();
	mcs /= img1.channels();

	cv::Scalar res(mssim, mcs);

	return res;
}
