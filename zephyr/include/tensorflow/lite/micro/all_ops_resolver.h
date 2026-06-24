/*
 * Copyright (c) 2026 Infineon Technologies AG
 * SPDX-License-Identifier: Apache-2.0
 *
 * Zephyr bridge: provides tflite::AllOpsResolver backed by MicroMutableOpResolver.
 *
 * The upstream TFLite Micro repository ships all_ops_resolver.h / all_ops_resolver.cc
 * as a convenience class that registers every built-in op. The Zephyr tflite-micro
 * module intentionally omits it to encourage selective op registration (smaller flash).
 *
 * This header re-creates AllOpsResolver using the MicroMutableOpResolver<N> API that
 * IS available in the Zephyr module, so that mtb_ml_model.cpp can be compiled without
 * any source modifications.
 *
 * Op count: ~113 (standard + signal). Template parameter is set with headroom.
 * Signal ops are guarded by TFLM_SIGNAL_OPS — define it to include them.
 */

#ifndef ZEPHYR_IFX_ML_MIDDLEWARE_ALL_OPS_RESOLVER_H_
#define ZEPHYR_IFX_ML_MIDDLEWARE_ALL_OPS_RESOLVER_H_

#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"

namespace tflite {

/* 130 slots: 98 standard + 12 signal + 3 misc with headroom */
class AllOpsResolver : public MicroMutableOpResolver<130> {
 public:
  AllOpsResolver() {
    /* ------------------------------------------------------------------ */
    /* Standard built-in ops                                               */
    /* ------------------------------------------------------------------ */
    AddAbs();
    AddAdd();
    AddAddN();
    AddArgMax();
    AddArgMin();
    AddAssignVariable();
    AddAveragePool2D();
    AddBatchMatMul();
    AddBatchToSpaceNd();
    AddBroadcastArgs();
    AddBroadcastTo();
    AddCallOnce();
    AddCast();
    AddCeil();
    AddCircularBuffer();
    AddConcatenation();
    AddConv2D();
    AddCos();
    AddCumSum();
    AddDepthToSpace();
    AddDepthwiseConv2D();
    AddDequantize();
    AddDiv();
    AddElu();
    AddEmbeddingLookup();
    AddEqual();
    AddExp();
    AddExpandDims();
    AddFill();
    AddFloor();
    AddFloorDiv();
    AddFloorMod();
    AddFullyConnected();
    AddGather();
    AddGatherNd();
    AddGreater();
    AddGreaterEqual();
    AddHardSwish();
    AddIf();
    AddL2Normalization();
    AddL2Pool2D();
    AddLeakyRelu();
    AddLess();
    AddLessEqual();
    AddLog();
    AddLogSoftmax();
    AddLogicalAnd();
    AddLogicalNot();
    AddLogicalOr();
    AddLogistic();
    AddMaximum();
    AddMaxPool2D();
    AddMean();
    AddMinimum();
    AddMirrorPad();
    AddMul();
    AddNeg();
    AddNotEqual();
    AddPack();
    AddPad();
    AddPadV2();
    AddPrelu();
    AddQuantize();
    AddReadVariable();
    AddReduceMax();
    AddRelu();
    AddRelu6();
    AddReshape();
    AddResizeBilinear();
    AddResizeNearestNeighbor();
    AddRound();
    AddRsqrt();
    AddSelectV2();
    AddShape();
    AddSin();
    AddSlice();
    AddSoftmax();
    AddSpaceToBatchNd();
    AddSpaceToDepth();
    AddSplit();
    AddSplitV();
    AddSqueeze();
    AddSqrt();
    AddSquare();
    AddSquaredDifference();
    AddStridedSlice();
    AddSub();
    AddSum();
    AddSvdf();
    AddTanh();
    AddTranspose();
    AddTransposeConv();
    AddUnidirectionalSequenceLSTM();
    AddUnpack();
    AddVarHandle();
    AddWhile();
    AddZerosLike();

    /* ------------------------------------------------------------------ */
    /* Ethos-U NPU custom op (only when driver is enabled)                 */
    /* ------------------------------------------------------------------ */
#if defined(CONFIG_ETHOS_U) || defined(COMPONENT_U55)
    AddEthosU();
#endif

    /* ------------------------------------------------------------------ */
    /* TFLM Signal ops (audio DSP: FFT, filter banks, etc.)               */
    /* Define TFLM_SIGNAL_OPS to include these.                           */
    /* ------------------------------------------------------------------ */
#if defined(TFLM_SIGNAL_OPS)
    AddDelay();
    AddEnergy();
    AddFftAutoScale();
    AddFilterBank();
    AddFilterBankLog();
    AddFilterBankSpectralSubtraction();
    AddFilterBankSquareRoot();
    AddFramer();
    AddIrfft();
    AddOverlapAdd();
    AddPCAN();
    AddRfft();
    AddStacker();
    AddWindow();
#endif
  }
};

}  // namespace tflite

#endif  /* ZEPHYR_IFX_ML_MIDDLEWARE_ALL_OPS_RESOLVER_H_ */
