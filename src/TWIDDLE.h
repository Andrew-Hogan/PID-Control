#ifndef TWIDDLE_H
#define TWIDDLE_H

class TWIDDLE {
public:
  /*
  * Parameters
  */
  double p[3];
  double dp[3];

  /*
  * Decision metrics
  */
  double error;
  double best_error;

  /*
  * Tracking variables
  */
  int index;
  bool back_step;
  int run_num;


  /*
  * Constructor
  */
  TWIDDLE();

  /*
  * Destructor.
  */
  virtual ~TWIDDLE();

  /*
  * Initialize twiddle.
  */
  void Init(double p1, double p2, double p3);

  /*
  * Update the twiddle values at the end of a testing session and reset.
  */
  void UpdateError(int n);

  /*
  * Check if dp is within tolerance set in main.
  */
  bool SumDP(double tolerance);
};

#endif /* TWIDDLE_H */