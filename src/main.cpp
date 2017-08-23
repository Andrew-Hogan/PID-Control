#include <uWS/uWS.h>
#include <iostream>
#include "json.hpp"
#include "PID.h"
#include "TWIDDLE.h"
#include <math.h>

// for convenience
using json = nlohmann::json;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }

// Checks if the SocketIO event has JSON data.
// If there is data the JSON object in string format will be returned,
// else the empty string "" will be returned.
std::string hasData(std::string s) {
  auto found_null = s.find("null");
  auto b1 = s.find_first_of("[");
  auto b2 = s.find_last_of("]");
  if (found_null != std::string::npos) {
    return "";
  }
  else if (b1 != std::string::npos && b2 != std::string::npos) {
    return s.substr(b1, b2 - b1 + 1);
  }
  return "";
}

// Used for twiddle iterations.
void reset_simulator(uWS::WebSocket<uWS::SERVER>& ws) {
  std::string msg("42[\"reset\",{}]");
  ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
}

int main()
{
  uWS::Hub h;

  PID pid;
  TWIDDLE twid;

  //modified original: 0.1, 0.0001, 4.0
  pid.Init(.1, .0001, 4.3645);
  twid.Init(pid.Kp, pid.Ki, pid.Kd);

  h.onMessage([&pid, &twid](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length, uWS::OpCode opCode) {
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    if (length && length > 2 && data[0] == '4' && data[1] == '2')
    {
      auto s = hasData(std::string(data).substr(0, length));
      if (s != "") {
        auto j = json::parse(s);
        std::string event = j[0].get<std::string>();
        if (event == "telemetry") {
          // j[1] is the data JSON object
          double cte = std::stod(j[1]["cte"].get<std::string>());
          double speed = std::stod(j[1]["speed"].get<std::string>());
          double angle = std::stod(j[1]["steering_angle"].get<std::string>());
          double steer_value;
          //twiddle accuracy
          double tolerance = .0001;
          //twiddle testing time per run
          int time_steps = 2500;
          bool use_twiddle = false;

          if (use_twiddle == false) {
            pid.UpdateError(cte);
            steer_value = pid.TotalError();
          }
          else {
            if (twid.SumDP(tolerance) == true) { // Check tolerance
              if (twid.run_num > time_steps){
                twid.UpdateError(.5 * time_steps);
                pid.Init(twid.p[0], twid.p[1], twid.p[2]);
                std::cout << "Gen Error: " << twid.best_error << " Next P Values: " << twid.p[0] << " , " << twid.p[1] << " , " << twid.p[2] << std::endl;
                std::cout << "Current PID Values: " << pid.Kp << " , " << pid.Ki << " , " << pid.Kd << std::endl;
                reset_simulator(ws);
              }
              else {
                pid.UpdateError(cte);
                steer_value = pid.TotalError();
                if (twid.run_num > (.5 * time_steps)){
                  twid.error += pow(cte, 2);
                }
                twid.run_num += 1;
              }
            }
            else {
              std::cout << "Best Error: " << twid.best_error << " P Values: " << twid.p[0] << " , " << twid.p[1] << " , " << twid.p[2] << std::endl;
            }
          }

		      // steering normalization
          if (steer_value < -1.0) { 
		        steer_value = -1.0; 
		      }
	        if (steer_value > 1.0) { 
		        steer_value = 1.0; 
		      }

          // DEBUG
          //std::cout << "CTE: " << cte << " Steering Value: " << steer_value << std::endl;

          json msgJson;
          msgJson["steering_angle"] = steer_value;
          msgJson["throttle"] = 0.4;
          auto msg = "42[\"steer\"," + msgJson.dump() + "]";
          //std::cout << msg << std::endl;
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
        }
      } else {
        // Manual driving
        std::string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
    }
  });

  // We don't need this since we're not using HTTP but if it's removed the program
  // doesn't compile :-(
  h.onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
    const std::string s = "<h1>Hello world!</h1>";
    if (req.getUrl().valueLength == 1)
    {
      res->end(s.data(), s.length());
    }
    else
    {
      // i guess this should be done more gracefully?
      res->end(nullptr, 0);
    }
  });

  h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {
    std::cout << "Connected!!!" << std::endl;
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code, char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port))
  {
    std::cout << "Listening to port " << port << std::endl;
  }
  else
  {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }
  h.run();
}
