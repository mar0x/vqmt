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

/**************************************************************************

 Calculation of the Structural Similarity (SSIM) image quality measure.

**************************************************************************/

#ifndef SSIM_hpp
#define SSIM_hpp

#include "Metric.hpp"

class SSIM : protected Metric {
public:
	SSIM(int height, int width, int t);
	// Compute the SSIM index of the processed image
	float compute(const cv::Mat& original, const cv::Mat& processed);
#if defined(HAVE_SSIM_BLUR_8)
	cv::Scalar compute_x8(const cv::Mat& original, const cv::Mat& processed);
#endif
protected:
	// Compute the SSIM index and mean of the contrast comparison function
	cv::Scalar computeSSIM(const cv::Mat& img1, const cv::Mat& img2);
private:
	cv::Mat mu1, mu2;
	cv::Mat mu1_sq, mu2_sq, mu1_mu2;
	cv::Mat img1_sq, img2_sq, img1_img2;
	cv::Mat sigma1_sq, sigma2_sq, sigma12;

#if defined(HAVE_SSIM_BLUR_8)
	cv::Mat bmu1, bmu2;
	cv::Mat bmu1_sq, bmu2_sq, bmu1_mu2;
	cv::Mat bsigma1_sq, bsigma2_sq, bsigma12;
#endif
};

#endif
