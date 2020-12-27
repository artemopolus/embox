#include <string.h>

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "tensorflow/lite/core/api/error_reporter.h"

#include "CNN_model_tfmodel.hpp"

// TFLite globals
namespace {
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  // tflite::MicroInterpreter* interpreter = nullptr;
  // TfLiteTensor* model_input = nullptr;
  // TfLiteTensor* model_output = nullptr;

  // Create an area of memory to use for input, output, and other TensorFlow
  // arrays. You'll need to adjust this by compiling, running, and looking
  // for errors.
  // constexpr int kTensorArenaSize = 2 * 1024;
  // __attribute__((aligned(16)))uint8_t tensor_arena[kTensorArenaSize];
} // namespace
  char buf[50];
  int buf_len = 0;
  TfLiteStatus tflite_status;
  uint32_t num_elements;
  uint32_t timestamp;
  float y_val;

// void updateTime(void)
// {
//   //for while true

//       // Fill input buffer (use test value)
//     for (uint32_t i = 0; i < num_elements; i++)
//     {
//       model_input->data.f[i] = 2.0f;
//     }

//     // Get current timestamp
//     // timestamp = htim16.Instance->CNT;

//     // Run inference
//     tflite_status = interpreter->Invoke();
//     if (tflite_status != kTfLiteOk)
//     {
//       error_reporter->Report("Invoke failed");
//     }

//     // Read output (predicted y) of neural network
//     y_val = model_output->data.f[0];

//     // Print output of neural network along with inference time (microseconds)
//     // buf_len = sprintf(buf,
//     //                   "Output: %f | Duration: %lu\r\n",
//     //                   y_val,
//     //                   htim16.Instance->CNT - timestamp);
//     // HAL_UART_Transmit(&huart2, (uint8_t *)buf, buf_len, 100);

// }

int main(int argc, char *argv[])
{
  printf("Start tensor flow example\n");
  

  // static tflite::MicroErrorReporter micro_error_reporter;
  // error_reporter = &micro_error_reporter;
  // Say something to test error reporter
  // error_reporter->Report("STM32 TensorFlow Lite test");
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;
  model = tflite::GetModel(CNN_model_tfmodel);

  if (model->version() != TFLITE_SCHEMA_VERSION)
  {
      error_reporter->Report(
            "Model provided is schema version %d not equal "
            "to supported version %d.",
            model->version(), TFLITE_SCHEMA_VERSION);
  //   error_reporter->Report("Model version does not match Schema");
  //   while(1);
  }
  
  
  // // Pull in only needed operations (should match NN layers). Template parameter
  // // <n> is number of ops to be added. Available ops:
  // // tensorflow/lite/micro/kernels/micro_ops.h
  // static tflite::AllOpsResolver resolver;
  
  static tflite::MicroMutableOpResolver<4> micro_op_resolver;
  // micro_op_resolver.AddDepthwiseConv2D();
  // micro_op_resolver.AddFullyConnected();
  // micro_op_resolver.AddReshape();
  // micro_op_resolver.AddSoftmax();
  // // Add dense neural network layer operation

  // if (tflite_status != kTfLiteOk)
  // {
  //   error_reporter->Report("Could not add FULLY CONNECTED op");
  //   while(1);
  // }

  // // Build an interpreter to run the model with.
  // static tflite::MicroInterpreter static_interpreter(
  //     model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  // interpreter = &static_interpreter;

  // // Allocate memory from the tensor_arena for the model's tensors.
  // tflite_status = interpreter->AllocateTensors();
  // if (tflite_status != kTfLiteOk)
  // {
  //   error_reporter->Report("AllocateTensors() failed");
  //   while(1);
  // }

  // // Assign model input and output buffers (tensors) to pointers
  // model_input = interpreter->input(0);
  // model_output = interpreter->output(0);

  // // Get number of elements in input tensor
  // num_elements = model_input->bytes / sizeof(float);
  // buf_len = sprintf(buf, "Number of input elements: %lu\r\n", num_elements);



  return 0;
}
