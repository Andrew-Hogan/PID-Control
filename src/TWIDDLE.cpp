#include "TWIDDLE.h"

using namespace std;

/*
* TODO: Complete the PID class.
*/

TWIDDLE::TWIDDLE() {}

TWIDDLE::~TWIDDLE() {}

void TWIDDLE::Init(double p1, double p2, double p3) {
  p[0] = p1;
  p[1] = p2;
  p[2] = p3;

  dp[0] = 0.5;
  dp[1] = 0.5;
  dp[2] = 0.5;

  index = 0;
  error = 0;
  best_error = 0;
  back_step = false; // Tracker for twiddle direction
  run_num = 1; // Counter to track location in test run
}

void TWIDDLE::UpdateError(int n) {
  error = error / n;
  if (best_error == 0) {
    best_error = error;
    p[index] += dp[index];
  }
  else {
    if (error < best_error) {
      best_error = error;
      dp[index] *= 1.1;
      back_step = false;
      index = (index + 1) % 3;
      p[index] += dp[index];
    }
    else if (back_step == true) {
      p[index] += dp[index];
      dp[index] *= 0.9;
      back_step = false;
      index = (index + 1) % 3;
      p[index] += dp[index];
    }
    else {
      p[index] -= 2 * dp[index];
      back_step = true;
    }
  }
  error = 0;
  run_num = 1;
}

bool TWIDDLE::SumDP(double tolerance) {
  return ((dp[0] + dp[1] + dp[2]) > tolerance);
}