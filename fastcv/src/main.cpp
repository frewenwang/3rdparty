//
// Created by Liwd on 22-7-5.
//

#include "fastcv.h"
#include <fstream>
#include <iostream>
#include <thread>
#include <AEEstd.h>

using namespace std;

int DMS_W = 1600;
int DMS_H = 1300;

int OMS_W = 1920;
int OMS_H = 1280;

int srcW = OMS_W;
int srcH = OMS_H;

int dstW = OMS_W;
int dstH = OMS_H;

fcvOperationMode modeVals[] = {
	FASTCV_OP_LOW_POWER,
	FASTCV_OP_PERFORMANCE,
	FASTCV_OP_CPU_OFFLOAD,
	FASTCV_OP_CPU_PERFORMANCE
};

uint32_t opMode = (uint32_t)FASTCV_OP_CPU_PERFORMANCE;
uint32_t dstWidth = 640;
uint32_t dstHeight = 480;
uint32_t loopCount = 1;
bool saveFile = false;
bool fcvMem = false;
bool doUYVY2Gray = false;
bool doUYVY2RGB = false;
bool doRGB2BGR = false;
bool doScale = false;
bool doWarpAffine = false;


void testUYVY2RGB(uint8_t *pSrc, uint8_t *pDst) {
	auto startTime = std::chrono::system_clock::now();

	int stauts = fcvColorUYVYtoRGB888u8(pSrc, srcW * 2, srcW, srcH, pDst, srcW * 3);

	auto now = std::chrono::system_clock::now();
	auto used = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count();
	std::cout << "fcvColorUYVYtoRGB888u8 = " << stauts << " | time = " << used << " us" << std::endl;

	if (saveFile) {
		FILE *yuvFile = fopen("cvt.rgb", "wb+");
		fwrite(pDst, 1, srcW * srcH * 3, yuvFile);
		fclose(yuvFile);
	}
}

void testRGB2BGR(uint8_t *pSrc, uint8_t *pDst) {
	auto startTime = std::chrono::system_clock::now();

	fcvColorRGB888ToBGR888u8(pSrc, srcW, srcH, 0, pDst, 0);

	auto now = std::chrono::system_clock::now();
	auto used = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count();
	std::cout << "fcvColorRGB888ToBGR888u8 = " << 0 << " | time = " << used << " us" << std::endl;

	if (saveFile) {
		FILE *yuvFile = fopen("oms.bgr", "wb+");
		fwrite(pDst, 1, srcW * srcH * 3, yuvFile);
		fclose(yuvFile);
	}
}

void testUYVY2Gray(uint8_t *pSrc, uint8_t *pDstY, uint8_t *pDstUV) {
	auto startTime2 = std::chrono::system_clock::now();
	int stauts2 = fcvColorUYVYtoYUV420SemiPlanaru8(pSrc, srcW, srcH, srcW * 2,
												   pDstY, pDstUV,
												   srcW, srcW);
	auto now2 = std::chrono::system_clock::now();
	auto used2 = std::chrono::duration_cast<std::chrono::microseconds>(now2 - startTime2).count();
	std::cout << "fcvColorUYVYtoYUV420SemiPlanaru8 = " << stauts2 << " | time = " << used2 << " us" << std::endl;

	if (saveFile) {
		FILE *yFile = fopen("oms.gray", "wb+");
		fwrite(pDstY, 1, srcW *  srcH, yFile);
		fclose(yFile);
	}
}

void testScale(uint8_t *pSrc, uint8_t *pDst) {
	auto startTime = std::chrono::system_clock::now();
	int stauts = fcvScaleu8(pSrc, srcW, srcH, srcW,
							pDst, 800, 650, 800, FASTCV_INTERPOLATION_TYPE_NEAREST_NEIGHBOR);
	auto now = std::chrono::system_clock::now();
	auto used = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count();
	std::cout << "fcvScaleu8 = " << stauts << " | time = " << used << " us" << std::endl;

	if (saveFile) {
		FILE *yFile = fopen("scale.gray", "wb+");
		fwrite(pDst, 1, 800 *  650, yFile);
		fclose(yFile);
	}
}

void testWarpAffine(uint8_t *pSrc, uint8_t *pDst) {
//    const float32_t mat[6] = {0.273641348, -0.204319835, 49.4342995, 0.204319835, 0.273641348, -281.471436};
    const float32_t mat[6] = {0.248995, -0.210790, -25.551514, 0.210790, 0.248995, -274.382324};

//    fcvBorderType b = FASTCV_BORDER_CONSTANT;
    auto startTime = std::chrono::system_clock::now();
    int stauts = fcvTransformAffineClippedu8_v3(pSrc, 1920, 1280, 1920 * 1,
                                                mat,
                                                pDst, 187, 187, 187,
                                                nullptr, (fcvInterpolationType)1, (fcvBorderType)1, 0);
    auto now = std::chrono::system_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count();
    std::cout << "fcvTransformAffineClippedu8_v3 = " << stauts << " | time = " << used << " us" << std::endl;
    std::cout << to_string(pDst[100]) << std::endl;
    if (saveFile) {
        FILE *yFile = fopen("affine.gray", "wb+");
        fwrite(pDst, 1, 800 *  650, yFile);
        fclose(yFile);
    }
}

void printUsage() {
	fprintf(stderr, "USAGE: [--help] [-l loops] [-o opMode] [-s saveFile] [-m mallocType]\n"
					"       [-t testCase] [-w dstWidth] [-h dstHeight]\n\n"
					"       -l integer [Default 1]\n"
					"       -o integer | Integer value indicating Operation Mode.  Valid values are:\n"
					"           FASTCV_OP_LOW_POWER       = 0\n"
					"           PERFORMANCE               = 1\n"
					"           CPU_OFFLOAD               = 2 [default]\n"
					"           FASTCV_OP_CPU_PERFORMANCE = 3\n"
					"       -s integer\n"
					"           Don't save = 0 [default]\n"
					"           Save = 1\n"
					"       -m integer\n"
					"           malloc = 0 [default]\n"
					"           fcvMalloc = 1\n"
					"       -t string | Test Cases.  Valid values are:\n"
					"           uyvy2gray\n"
					"           uyvy2rgb\n"
					"           rgb2bgr\n"
					"           scale\n"
					);
}

int parseArgs(int argc, char *argv[]) {
	char modeString[20];

	for (int i = 1; i < argc; ++i) {
		if (std_strncmp(argv[i], "-s", 2) == 0) {
			++i;
			if (i < argc) {
				saveFile = atoi(argv[i]) == 1;
				printf("SaveFile : %d\n", saveFile);
			} else {
				fprintf(stderr, "ERROR: missing param with option -s\n");
				return 1;
			}
		} else if (std_strncmp(argv[i], "-l", 2) == 0) {
			++i;
			if (i < argc) {
				loopCount = atoi(argv[i]);
				loopCount = (loopCount == 0) ? 1 : loopCount;
				printf("LoopCount : %d\n", loopCount);
			} else {
				fprintf(stderr, "ERROR: missing param with option -l\n");
				return 1;
			}
		} else if (std_strncmp(argv[i], "-o", 2) == 0) {
			++i;
			if (i < argc) {
				opMode = atoi(argv[i]);

				switch (opMode) {
				case 0: std_strlcpy(modeString, "LOW_POWER", sizeof(modeString));
					break;
				case 3: std_strlcpy(modeString, "CPU_PERFORMANCE", sizeof(modeString));
					break;
				case 2: std_strlcpy(modeString, "CPU_OFFLOAD", sizeof(modeString));
					break;
				case 1: std_strlcpy(modeString, "PERFORMANCE", sizeof(modeString));
					break;
				default:std_strlcpy(modeString, "Unknown", sizeof(modeString));
					break;
				}
				printf("OperationMode: %s\n", modeString);
			} else {
				fprintf(stderr, "ERROR: missing param with option -o\n");
				return 1;
			}
		} else if (std_strncmp(argv[i], "-w", 2) == 0) {
			++i;
			if (i < argc) {
				dstWidth = atoi(argv[i]);
				printf("dstWidth : %d\n", dstWidth);
			} else {
				fprintf(stderr, "ERROR: missing param with option -w\n");
				return 1;
			}
		} else if (std_strncmp(argv[i], "-h", 2) == 0) {
			++i;
			if (i < argc) {
				dstHeight = atoi(argv[i]);
				printf("dstHeight : %d\n", dstHeight);
			} else {
				fprintf(stderr, "ERROR: missing param with option -h\n");
				return 1;
			}
		} else if (std_strncmp(argv[i], "-m", 2) == 0) {
			++i;
			if (i < argc) {
				fcvMem = atoi(argv[i]) == 1;
				printf("UseFcvMem : %d\n", fcvMem);
			} else {
				fprintf(stderr, "ERROR: missing param with option -m\n");
				return 1;
			}
		} else if (std_strncmp(argv[i], "-t", 2) == 0) {
			++i;
			if (i < argc) {
				printf("Test : %s\n", argv[i]);
				if (std_strncmp(argv[i], "uyvy2gray", 9) == 0) {
					doUYVY2Gray = true;
				} else if (std_strncmp(argv[i], "uyvy2rgb", 8) == 0) {
					doUYVY2RGB = true;
				} else if (std_strncmp(argv[i], "rgb2bgr", 7) == 0) {
					doRGB2BGR = true;
				} else if (std_strncmp(argv[i], "scale", 5) == 0) {
					doScale = true;
				} else if (std_strncmp(argv[i], "warp", 4) == 0) {
                    doWarpAffine = true;
                }
			} else {
				fprintf(stderr, "ERROR: missing param with option -t\n");
				return 1;
			}
		} else if (std_strncmp(argv[i], "--help", 6) == 0) {
			++i;
			if (i < argc) {
				printUsage();
			}
		}
	}
	return 0;
}


int main(int argc, char *argv[]) {
	if (argc == 1) {
		printUsage();
		return 0;
	}
	parseArgs(argc, argv);

	// Init
	int ret = fcvSetOperationMode(modeVals[opMode]);
	printf("fcvSetOperationMode mode = %d ret = %d\n", opMode, ret);

//	std::this_thread::sleep_for(std::chrono::seconds(1));

	fcvMemInit();

	uint8_t *pSrc = nullptr;
	uint8_t *pDst = nullptr;
	uint8_t *pDstY = nullptr;
	uint8_t *pDstUV = nullptr;

	uint8_t *pRGB = nullptr;
	uint8_t *pBGR = nullptr;

	uint8_t *pScale = nullptr;
    uint8_t *pWarp = nullptr;

	if (fcvMem == 1) {
		pSrc = (uint8_t *)fcvMemAlloc(srcW * srcH * 2, 16);
		pDst = (uint8_t *)fcvMemAlloc(dstW * dstH * 3, 128);
		pDstY = (uint8_t *)fcvMemAlloc(srcW * srcH, 128);
		pDstUV = (uint8_t *)fcvMemAlloc(srcW * srcH, 16);

		pRGB = (uint8_t *)fcvMemAlloc(srcW * srcH * 3, 128);
		pBGR = (uint8_t *)fcvMemAlloc(srcW * srcH * 3, 128);

		pScale = (uint8_t *)fcvMemAlloc(800 * 650 * 3, 16);
        pWarp = (uint8_t *) fcvMemAlloc(187 * 187 * 1, 128);
	} else {
		pSrc = (uint8_t *)malloc(srcW * srcH * 2);
		pDst = (uint8_t *)malloc(srcW * srcH * 3);
		pDstY = (uint8_t *)malloc(srcW * srcH);
		pDstUV = (uint8_t *)malloc(srcW * srcH);

		pRGB = (uint8_t *)malloc(srcW * srcH * 3);
		pBGR = (uint8_t *)malloc(srcW * srcH * 3);

		pScale = (uint8_t *)malloc(800 * 650 * 3);
        pWarp = (uint8_t *) malloc(187 * 187 * 1);
	}

	// Read Image File
	int size = 0;
	std::ifstream file("oms.yuv", std::ios::binary);
	if (file.good()) {
		file.seekg(0, file.end);
		size = file.tellg();
		file.seekg(0, file.beg);
		file.read((char *)pSrc, size);
	}
	file.close();

	for (int i = 0; i < loopCount ; i++) { // || true
		auto startTime = std::chrono::system_clock::now();

		if (doUYVY2Gray) {
			testUYVY2Gray(pSrc, pDstY, pDstUV);
		}
		if (doUYVY2RGB) {
			testUYVY2RGB(pSrc, pRGB);
		}
		if (doRGB2BGR) {
			testRGB2BGR(pRGB, pRGB);
		}
		if (doScale) {
			testScale(pDstY, pScale);
		}
        if (doWarpAffine) {
            testWarpAffine(pDstY, pWarp);
        }

		auto now = std::chrono::system_clock::now();
		long used = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count();
		used = 33000 - used;
		if (used > 0) {
			std::this_thread::sleep_for(std::chrono::microseconds(used));
		}
	}

	// DeInit FastCV
	fcvMemFree(pSrc);
	pSrc = NULL;
	if (pDst) {
		fcvMemFree(pDst);
		pDst = NULL;
	}
	fcvMemDeInit();
	fcvCleanUp();

	return 0;
}