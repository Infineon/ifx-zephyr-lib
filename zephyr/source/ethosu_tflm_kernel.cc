/* Copyright 2020-2023 The TensorFlow Authors. All Rights Reserved.
 * Portions Copyright (c) 2026 Infineon Technologies AG.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file ethosu_tflm_kernel.cc
 * @brief TFLite Micro CUSTOM op kernel for the ARM Ethos-U NPU.
 *
 * Provides Register_ETHOSU() and GetString_ETHOSU() so that tflite::AllOpsResolver
 * can register the Ethos-U CUSTOM op via AddEthosU().
 *
 * WHY THIS FILE EXISTS:
 *   The upstream tflite-micro project ships this as ethos_u.cc. The Zephyr
 *   tflite-micro module (modules/tflite-micro) only compiles that file when
 *   CONFIG_ETHOS_U=y. For the Infineon ML Middleware path we deliberately do
 *   NOT set CONFIG_ETHOS_U (which would pull in the native Zephyr Ethos-U
 *   hardware driver and conflict with the ML Middleware's own NPU init).
 *   Instead, we compile this file directly when CONFIG_ML_MIDDLEWARE_ETHOS_U55=y.
 *
 *   The Infineon ml-tflite-micro module ships ethosu.h (the declaration) but
 *   not ethosu.cc (the implementation). This file provides that implementation.
 *
 * UPSTREAM REFERENCE:
 *   Based on tensorflow/lite/micro/kernels/ethos_u/ethos_u.cc from the
 *   TensorFlow Lite Micro project (Apache 2.0).
 */

#include <cstdint>
#include <cstring>

#include "tensorflow/lite/micro/micro_common.h"
#include "tensorflow/lite/micro/kernels/ethosu.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_context.h"
#include "tensorflow/lite/c/common.h"

extern "C" {
#include "ethosu_driver.h"
}

extern struct ethosu_driver *mtb_ml_ethosu_driver_handle;

namespace tflite {

namespace {

constexpr char kEthosuCustomCode[] = "ethos-u";

struct OpData {
    /* No persistent state needed — all data comes from the flatbuffer */
};

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
    return nullptr;
}

void Free(TfLiteContext* context, void* buffer) {}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
    return kTfLiteOk;
}

TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
    const uint8_t* custom_data =
        reinterpret_cast<const uint8_t*>(node->custom_initial_data);
    const size_t custom_data_size =
        static_cast<size_t>(node->custom_initial_data_size);

    /* Build base address array from input tensors */
    const int num_inputs  = node->inputs->size;
    const int num_outputs = node->outputs->size;
    const int num_bases   = num_inputs + num_outputs;

    uint64_t base_addrs[num_bases];
    size_t   base_sizes[num_bases];

    for (int i = 0; i < num_inputs; ++i) {
        const TfLiteTensor* t = context->GetTensor(context, node->inputs->data[i]);
        base_addrs[i] = reinterpret_cast<uint64_t>(t->data.raw_const);
        base_sizes[i] = static_cast<size_t>(t->bytes);
    }
    for (int i = 0; i < num_outputs; ++i) {
        TfLiteTensor* t = context->GetTensor(context, node->outputs->data[i]);
        base_addrs[num_inputs + i] = reinterpret_cast<uint64_t>(t->data.raw);
        base_sizes[num_inputs + i] = static_cast<size_t>(t->bytes);
    }

    if (mtb_ml_ethosu_driver_handle == nullptr) {
        MicroPrintf("Ethos-U driver handle not initialised.");
        return kTfLiteError;
    }

    int result = ethosu_invoke_v3(
        mtb_ml_ethosu_driver_handle,
        custom_data,
        static_cast<int>(custom_data_size),
        base_addrs,
        base_sizes,
        num_bases,
        /*user_arg=*/nullptr);

    if (result != 0) {
        MicroPrintf("Ethos-U invocation failed: %d", result);
        return kTfLiteError;
    }

    return kTfLiteOk;
}

}  // namespace

TFLMRegistration* Register_ETHOSU() {
    static TFLMRegistration r = {Init, Free, Prepare, Eval};
    return &r;
}

const char* GetString_ETHOSU() {
    return kEthosuCustomCode;
}

}  // namespace tflite
