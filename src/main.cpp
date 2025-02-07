//
// Copyright(c) Multimedia Signal Processing Group (MMSPG),
//              Ecole Polytechnique Fédérale de Lausanne (EPFL)
//              http://mmspg.epfl.ch
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

/**************************************************************************

 Usage:
  VQMT.exe OriginalVideo ProcessedVideo Height Width NumberOfFrames ChromaFormat Output Metrics

  OriginalVideo: the original video as raw YUV video file, progressively scanned, and 8 bits per sample
  ProcessedVideo: the processed video as raw YUV video file, progressively scanned, and 8 bits per sample
  Height: the height of the video
  Width: the width of the video
  NumberOfFrames: the number of frames to process
  ChromaFormat: the chroma subsampling format. 0: YUV400, 1: YUV420, 2: YUV422, 3: YUV444
  Output: the name of the output file(s)
  Metrics: the list of metrics to use
   available metrics:
   - PSNR: Peak Signal-to-Noise Ratio (PNSR)
   - SSIM: Structural Similarity (SSIM)
   - MSSSIM: Multi-Scale Structural Similarity (MS-SSIM)
   - VIFP: Visual Information Fidelity, pixel domain version (VIFp)
   - PSNRHVS: Peak Signal-to-Noise Ratio taking into account Contrast Sensitivity Function (CSF) (PSNR-HVS)
   - PSNRHVSM: Peak Signal-to-Noise Ratio taking into account Contrast Sensitivity Function (CSF) and between-coefficient contrast masking of DCT basis functions (PSNR-HVS-M)

 Example:
  VQMT.exe original.yuv processed.yuv 1088 1920 250 1 results PSNR SSIM MSSSIM VIFP
  will create the following output files in CSV (comma-separated values) format:
  - results_pnsr.csv
  - results_ssim.csv
  - results_msssim.csv
  - results_vifp.csv

 Notes:
 - SSIM comes for free when MSSSIM is computed (but you still need to specify it to get the output)
 - PSNRHVS and PSNRHVSM are always computed at the same time (but you still need to specify both to get the two outputs)
 - When using MSSSIM, the height and width of the video have to be multiple of 16
 - When using VIFP, the height and width of the video have to be multiple of 8

 Changes in version 1.1 (since 1.0) on 30/3/13
 - Added support for large files (>2GB)
 - Added support for different chroma sampling formats (YUV400, YUV420, YUV422, and YUV444)

**************************************************************************/

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <opencv2/core/core.hpp>
#include "VideoYUV.hpp"
#include "PSNR.hpp"
#include "SSIM.hpp"
#include "MSSSIM.hpp"
#include "VIFP.hpp"
#include "PSNRHVS.hpp"

enum Params {
	PARAM_ORIGINAL = 1,	// Original video stream (YUV)
	PARAM_PROCESSED,	// Processed video stream (YUV)
	PARAM_HEIGHT,		// Height
	PARAM_WIDTH,		// Width
	PARAM_NBFRAMES,		// Number of frames
	PARAM_CHROMA,		// Chroma format
	PARAM_RESULTS,		// Output file for results
	PARAM_METRICS,		// Metric(s) to compute
	PARAM_SIZE
};

enum Metrics {
	METRIC_PSNR = 0,
	METRIC_YUVPSNR,
	METRIC_SSIM,
	METRIC_YUVSSIM,
	METRIC_MSSSIM,
	METRIC_VIFP,
	METRIC_PSNRHVS,
	METRIC_PSNRHVSM,
	METRIC_SIZE
};

int main (int argc, const char *argv[])
{
	// Check number of input parameters
	if (argc < PARAM_SIZE) {
		fprintf(stderr, "Check software usage: at least %d parameters are required.\n", PARAM_SIZE);
		return EXIT_FAILURE;
	}

	double duration = static_cast<double>(cv::getTickCount());

	// Input parameters
	char *endptr = NULL;
	int height = static_cast<int>(strtol(argv[PARAM_HEIGHT], &endptr, 10));
	if (*endptr) {
		fprintf(stderr, "Incorrect value for video height: %s\n", argv[PARAM_HEIGHT]);
		return EXIT_FAILURE;
	}
	int width = static_cast<int>(strtol(argv[PARAM_WIDTH], &endptr, 10));
	if (*endptr) {
		fprintf(stderr, "Incorrect value for video width: %s\n", argv[PARAM_WIDTH]);
		return EXIT_FAILURE;
	}
	int nbframes = static_cast<int>(strtol(argv[PARAM_NBFRAMES], &endptr, 10));
	if (*endptr) {
		fprintf(stderr, "Incorrect value for number of frames: %s\n", argv[PARAM_NBFRAMES]);
		return EXIT_FAILURE;
	}
	int chroma = static_cast<int>(strtol(argv[PARAM_CHROMA], &endptr, 10));
	if (*endptr) {
		fprintf(stderr, "Incorrect value for chroma: %s\n", argv[PARAM_CHROMA]);
		return EXIT_FAILURE;
	}

	// Input video streams
	VideoYUV *original  = new VideoYUV(argv[PARAM_ORIGINAL], height, width, nbframes, chroma);
	VideoYUV *processed = new VideoYUV(argv[PARAM_PROCESSED], height, width, nbframes, chroma);

	// Output files for results
	FILE *result_file[METRIC_SIZE] = {NULL};
	char *str = new char[256];
	for (int i=7; i<argc; i++) {
		if (strcmp(argv[i], "PSNR") == 0 || strcmp(argv[i], "YPSNR") == 0) {
			sprintf(str, "%s_psnr.csv", argv[PARAM_RESULTS]);
			result_file[METRIC_PSNR] = fopen(str, "w");
		} else if (strcmp(argv[i], "YUVPSNR") == 0) {
			sprintf(str, "%s_yuvpsnr.csv", argv[PARAM_RESULTS]);
			result_file[METRIC_YUVPSNR] = fopen(str, "w");
		}
		else if (strcmp(argv[i], "SSIM") == 0) {
			sprintf(str, "%s_ssim.csv", argv[PARAM_RESULTS]);
			result_file[METRIC_SSIM] = fopen(str, "w");
		}
		else if (strcmp(argv[i], "YUVSSIM") == 0) {
			sprintf(str, "%s_yuvssim.csv", argv[PARAM_RESULTS]);
			result_file[METRIC_YUVSSIM] = fopen(str, "w");
		}
		else if (strcmp(argv[i], "MSSSIM") == 0) {
			sprintf(str, "%s_msssim.csv", argv[PARAM_RESULTS]);
			result_file[METRIC_MSSSIM] = fopen(str, "w");
		}
		else if (strcmp(argv[i], "VIFP") == 0) {
			sprintf(str, "%s_vifp.csv", argv[PARAM_RESULTS]);
			result_file[METRIC_VIFP] = fopen(str, "w");
		}
		else if (strcmp(argv[i], "PSNRHVS") == 0) {
			sprintf(str, "%s_psnrhvs.csv", argv[PARAM_RESULTS]);
			result_file[METRIC_PSNRHVS] = fopen(str, "w");
		}
		else if (strcmp(argv[i], "PSNRHVSM") == 0) {
			sprintf(str, "%s_psnrhvsm.csv", argv[PARAM_RESULTS]);
			result_file[METRIC_PSNRHVSM] = fopen(str, "w");
		}
	}
	delete[] str;

	// Check size for VIFp downsampling
	if (result_file[METRIC_VIFP] != NULL && (height % 8 != 0 || width % 8 != 0)) {
		fprintf(stderr, "VIFp: 'height' and 'width' have to be multiple of 8.\n");
		exit(EXIT_FAILURE);
	}
	// Check size for MS-SSIM downsampling
	if (result_file[METRIC_MSSSIM] != NULL && (height % 16 != 0 || width % 16 != 0)) {
		fprintf(stderr, "MS-SSIM: 'height' and 'width' have to be multiple of 16.\n");
		exit(EXIT_FAILURE);
	}

	// Print header to file
	for (int m=0; m<METRIC_SIZE; m++) {
		if (result_file[m] != NULL) {
			fprintf(result_file[m], "frame,value\n");
		}
	}

	PSNR *psnr     = new PSNR(height, width, CV_32F);
	PSNR *yuvpsnr  = new PSNR(height, width, CV_32FC3);
	SSIM *ssim     = new SSIM(height, width, CV_32F);
	SSIM *yuvssim  = new SSIM(height, width, CV_32FC3);
	MSSSIM *msssim = new MSSSIM(height, width);
	VIFP *vifp     = new VIFP(height, width);
	PSNRHVS *phvs  = new PSNRHVS(height, width);

	cv::Mat original_frame(height,width,CV_32F), processed_frame(height,width,CV_32F);
	cv::Mat original_frame3(height,width,CV_32FC3), processed_frame3(height,width,CV_32FC3);

	float result[METRIC_SIZE] = {0};
	float result_avg[METRIC_SIZE] = {0};

	for (int frame=0; frame<nbframes; frame++) {
		// Grab frame
		if (!original->readOneFrame()) exit(EXIT_FAILURE);
		original->getLuma(original_frame, CV_32F);
		if (!processed->readOneFrame()) exit(EXIT_FAILURE);
		processed->getLuma(processed_frame, CV_32F);

		if (result_file[METRIC_YUVPSNR] != NULL || result_file[METRIC_YUVSSIM] != NULL) {
			original->getYUV(original_frame3);
			processed->getYUV(processed_frame3);
		}

		// Compute PSNR
		if (result_file[METRIC_PSNR] != NULL) {
			result[METRIC_PSNR] = psnr->compute(original_frame, processed_frame);
		}

		// Compute YUVPSNR
		if (result_file[METRIC_YUVPSNR] != NULL) {
			result[METRIC_YUVPSNR] = yuvpsnr->compute(original_frame3, processed_frame3);
		}

		// Compute SSIM and MS-SSIM
		if (result_file[METRIC_SSIM] != NULL && result_file[METRIC_MSSSIM] == NULL) {
			result[METRIC_SSIM] = ssim->compute(original_frame, processed_frame);
		}

		// Compute YUVSSIM and MS-SSIM
		if (result_file[METRIC_YUVSSIM] != NULL) {
			result[METRIC_YUVSSIM] = yuvssim->compute(original_frame3, processed_frame3);
		}

		if (result_file[METRIC_MSSSIM] != NULL) {
			msssim->compute(original_frame, processed_frame);
			if (result_file[METRIC_SSIM] != NULL) {
				result[METRIC_SSIM] = msssim->getSSIM();
			}
			result[METRIC_MSSSIM] = msssim->getMSSSIM();
		}

		// Compute VIFp
		if (result_file[METRIC_VIFP] != NULL) {
			result[METRIC_VIFP] = vifp->compute(original_frame, processed_frame);
		}

		// Compute PSNR-HVS and PSNR-HVS-M
		if (result_file[METRIC_PSNRHVS] != NULL || result_file[METRIC_PSNRHVSM] != NULL) {
			phvs->compute(original_frame, processed_frame);
			if (result_file[METRIC_PSNRHVS] != NULL) {
				result[METRIC_PSNRHVS] = phvs->getPSNRHVS();
			}
			if (result_file[METRIC_PSNRHVSM] != NULL) {
				result[METRIC_PSNRHVSM] = phvs->getPSNRHVSM();
			}
		}

		// Print quality index to file
		for (int m=0; m<METRIC_SIZE; m++) {
			if (result_file[m] != NULL) {
				result_avg[m] += result[m];
				fprintf(result_file[m], "%d,%.6f\n", frame, static_cast<double>(result[m]));
			}
		}
	}

	// Print average quality index to file
	for (int m=0; m<METRIC_SIZE; m++) {
		if (result_file[m] != NULL) {
			result_avg[m] /= static_cast<float>(nbframes);
			fprintf(result_file[m], "average,%.6f", static_cast<double>(result_avg[m]));
			fclose(result_file[m]);
		}
	}

	delete psnr;
	delete ssim;
	delete msssim;
	delete vifp;
	delete phvs;
	delete original;
	delete processed;

	duration = static_cast<double>(cv::getTickCount())-duration;
	duration /= cv::getTickFrequency();
	//printf("Time: %0.3fs\n", duration);

	return EXIT_SUCCESS;
}
